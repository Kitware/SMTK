//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/operators/ForceCreateFace.h"

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/internal/Config.h"
#include "smtk/session/polygon/internal/Edge.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Model.txx"
#include "smtk/session/polygon/internal/Util.h"

#include "smtk/session/polygon/Operation.txx"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/UnionFind.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Resource.txx"
#include "smtk/model/Tessellation.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/session/polygon/operators/ForceCreateFace_xml.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/polygon/polygon.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <deque>
#include <map>
#include <set>
#include <vector>

namespace poly = boost::polygon;
using namespace boost::polygon::operators;

namespace smtk
{
namespace session
{
namespace polygon
{

typedef std::vector<std::pair<smtk::model::Edge, bool>> EdgesWithOrientation;

/// Ensure that we are provided the proper edge orientations in addition to the usual checks.
bool ForceCreateFace::ableToOperate()
{
  // TODO: This should probably also verify that the "counts" item
  //       matches the number of points or edges provided:
  int method = this->parameters()->findInt("construction method")->discreteIndex();
  smtk::attribute::IntItem::Ptr edgeDirItem = this->parameters()->findInt("orientations");
  int numOrient = edgeDirItem != nullptr ? static_cast<int>(edgeDirItem->numberOfValues()) : -1;
  return this->Superclass::ableToOperate() &&
    (method == ForceCreateFace::POINTS ||
     (method == ForceCreateFace::EDGES &&
      (numOrient == -1 ||
       numOrient == static_cast<int>(this->parameters()->associations()->numberOfValues()))));
}

/// Create one or more polygonal faces without sanity checks.
smtk::operation::Operation::Result ForceCreateFace::operateInternal()
{
  int method = this->parameters()->findInt("construction method")->discreteIndex();

  smtk::attribute::DoubleItem::Ptr pointsItem = this->parameters()->findDouble("points");
  smtk::attribute::IntItem::Ptr coordinatesItem = this->parameters()->findInt("coordinates");
  smtk::attribute::IntItem::Ptr countsItem = this->parameters()->findInt("counts");
  smtk::attribute::IntItem::Ptr edgeDirItem = this->parameters()->findInt("orientations");

  int numCoordsPerPt = coordinatesItem->value(0);

  auto modelItem = this->parameters()->associations();
  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(
      modelItem->valueAs<smtk::model::Entity>()->resource());
  if (!resource)
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);

  smtk::model::Model smodel;

  // Obtain the SMTK and polygon models we will work with:
  switch (method)
  {
    case ForceCreateFace::POINTS:
      smodel = smtk::model::Model(modelItem->valueAs<smtk::model::Entity>());
      break;
    case ForceCreateFace::EDGES:
      smodel = smtk::model::Edge(modelItem->valueAs<smtk::model::Entity>()).owningModel();
      break;
    default:
    {
      smtkErrorMacro(this->log(), "Unknown construction method " << method << ".");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    break;
  }
  internal::pmodel::Ptr pmodel = resource->findStorage<internal::pmodel>(smodel.entity());
  if (!pmodel)
  {
    smtkErrorMacro(this->log(), "The associated model is not a polygon-session model.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Sanity-check point coordinates array size:
  if (method == ForceCreateFace::POINTS && pointsItem->numberOfValues() % numCoordsPerPt > 0)
  {
    smtkErrorMacro(
      this->log(),
      "Number of point-coordinates (" << pointsItem->numberOfValues() << ") "
                                      << "not a multiple of the number of coordinates per pt ("
                                      << numCoordsPerPt << ")");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // I. While the counts array indicates we have faces to process:
  smtk::attribute::IntItem::value_type::const_iterator countIt;
  smtk::attribute::IntItem::value_type::const_iterator endCount;

  // Initialize iterators over things we consume as we create faces:
  smtk::attribute::DoubleItem::value_type::const_iterator coordIt = pointsItem->begin();
  auto edgeIt = modelItem->begin();
  smtk::attribute::IntItem::value_type::const_iterator edgeDirIt = edgeDirItem != nullptr
    ? edgeDirItem->begin()
    : smtk::attribute::IntItem::value_type::const_iterator();

  // Handle default value for "counts" item:
  std::vector<int> tmpCount;
  if (countsItem->numberOfValues() == 1 && countsItem->value(0) < 0)
  {
    // If given default value, create a simple and valid "counts" array to iterator over:
    tmpCount.push_back(static_cast<int>(
      method == ForceCreateFace::POINTS ? pointsItem->numberOfValues() / numCoordsPerPt
                                        : modelItem->numberOfValues()));
    tmpCount.push_back(0);
    countIt = tmpCount.begin();
    endCount = tmpCount.end();
  }
  else
  {
    countIt = countsItem->begin();
    endCount = countsItem->end();
  }

  // Begin loop over faces requested by "counts":
  smtk::model::Faces created;
  for (; countIt != endCount;)
  {
    EdgesWithOrientation outerLoopEdges;
    // II. Queue ordered points for the outer loop
    poly::polygon_with_holes_data<internal::Coord> pface;
    { // This block exists so that polypts can go out of scope early.
      std::vector<internal::Point> polypts;
      if (method == ForceCreateFace::POINTS)
      {
        this->pointsForLoop(polypts, *countIt, coordIt, pointsItem->end(), numCoordsPerPt, pmodel);
        // Now create the matching SMTK edge
        smtk::model::Edge modelEdge =
          pmodel->createModelEdgeFromPoints(resource, polypts.begin(), polypts.end(), false);
        outerLoopEdges.emplace_back(modelEdge, true);
      }
      else
      {
        this->pointsForLoop(
          polypts,
          *countIt,
          edgeIt,
          modelItem->end(),
          edgeDirIt,
          edgeDirItem->end(),
          outerLoopEdges);
      }
      pface.set(polypts.begin(), polypts.end());
    }

    // III. Create a polygon set and add a polygon For each inner loop
    ++countIt;
    std::vector<poly::polygon_data<internal::Coord>> holes;
    int numHoles = (countIt == endCount ? 0 : *countIt);
    ++countIt;
    std::vector<EdgesWithOrientation> innerLoopsEdges;
    for (int h = 0; h < numHoles; ++h, ++countIt)
    {
      // Create placeholder for SMTK model edge(s) bounding the current hole:
      EdgesWithOrientation blank;
      innerLoopsEdges.push_back(blank);

      // Create or record edges of loop so we can add SMTK use-records below:
      std::vector<internal::Point> polypts;
      if (method == ForceCreateFace::POINTS)
      {
        this->pointsForLoop(polypts, *countIt, coordIt, pointsItem->end(), numCoordsPerPt, pmodel);
        // Now create the matching SMTK edge
        smtk::model::Edge modelEdge =
          pmodel->createModelEdgeFromPoints(resource, polypts.begin(), polypts.end(), false);
        innerLoopsEdges.back().emplace_back(modelEdge, true);
      }
      else
      {
        this->pointsForLoop(
          polypts,
          *countIt,
          edgeIt,
          modelItem->end(),
          edgeDirIt,
          edgeDirItem->end(),
          innerLoopsEdges.back());
      }
      // Add to polygon_set_data
      poly::polygon_data<internal::Coord> loop;
      loop.set(polypts.begin(), polypts.end());
      holes.push_back(loop);
    }
    pface.set_holes(holes.begin(), holes.end());

    // IV. Transcribe face (uses, loops, face) to smtk
    //
    // Make up some UUIDs for the new entities.
    smtk::common::UUID modelFaceId = resource->unusedUUID();
    smtk::common::UUID modelFaceUseId = resource->unusedUUID();
    smtk::common::UUID outerLoopId = resource->unusedUUID();
    // Transcribe the outer loop, face use, and face:
    if (!resource->insertModelFaceWithOrientedOuterLoop(
          modelFaceId, modelFaceUseId, outerLoopId, outerLoopEdges))
    {
      smtkErrorMacro(this->log(), "Could not create SMTK outer loop of face.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    smtk::model::Face modelFace(resource, modelFaceId);
    smodel.addCell(modelFace);
    modelFace.assignDefaultName();
    created.push_back(modelFace);

    // Now transcribe inner-loop edges:
    std::size_t numInner = innerLoopsEdges.size();
    for (std::size_t inner = 0; inner < numInner; ++inner)
    {
      smtk::common::UUID innerLoopId = resource->unusedUUID();
      if (!resource->insertModelFaceOrientedInnerLoop(
            innerLoopId, outerLoopId, innerLoopsEdges[inner]))
      {
        smtkErrorMacro(this->log(), "Could not create SMTK inner loop of face.");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
    }

    // V. Add tessellation
    poly::polygon_set_data<internal::Coord> polys;
    std::vector<poly::polygon_data<internal::Coord>> tess;
    polys.insert(pface);
    polys.get_trapezoids(tess);
    std::vector<poly::polygon_data<internal::Coord>>::const_iterator pit;
    smtk::model::Tessellation* smtkTess = modelFace.resetTessellation();
    double smtkPt[3];
    for (pit = tess.begin(); pit != tess.end(); ++pit)
    {
      //std::cout << "Fan\n";
      poly::polygon_data<internal::Coord>::iterator_type pcit;
      pcit = poly::begin_points(*pit);
      std::vector<int> triConn;
      triConn.resize(4);
      triConn[0] = smtk::model::TESS_TRIANGLE;
      pmodel->liftPoint(*pcit, &smtkPt[0]);
      triConn[1] = smtkTess->addCoords(&smtkPt[0]);
      ++pcit;
      pmodel->liftPoint(*pcit, &smtkPt[0]);
      triConn[3] = smtkTess->addCoords(&smtkPt[0]);
      ++pcit;
      for (; pcit != poly::end_points(*pit); ++pcit)
      {
        triConn[2] = triConn[3];
        pmodel->liftPoint(*pcit, &smtkPt[0]);
        triConn[3] = smtkTess->addCoords(&smtkPt[0]);
        smtkTess->insertNextCell(triConn);
      }
      //std::cout << "\n";
    }
    std::vector<double> bbox(6);
    smtk::model::Tessellation::invalidBoundingBox(bbox.data());
    smtkTess->getBoundingBox(bbox.data());
    modelFace.setBoundingBox(bbox.data());
  }

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItem::Ptr createdItem = result->findComponent("created");
  for (auto& c : created)
  {
    createdItem->appendValue(c.component());
  }
  operation::MarkGeometry(resource).markResult(result);

  return result;
}

const char* ForceCreateFace::xmlDescription() const
{
  return ForceCreateFace_xml;
}

} // namespace polygon
} //namespace session
} // namespace smtk
