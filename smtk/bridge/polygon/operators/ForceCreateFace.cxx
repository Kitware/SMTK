//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/operators/ForceCreateFace.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Config.h"
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/internal/Model.txx"
#include "smtk/bridge/polygon/internal/Edge.h"
#include "smtk/bridge/polygon/internal/Util.h"

#include "smtk/bridge/polygon/Operator.txx"

#include "smtk/common/UnionFind.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Tessellation.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Manager.txx"

#include "smtk/bridge/polygon/ForceCreateFace_xml.h"

#include "boost/polygon/polygon.hpp"

#include <deque>
#include <map>
#include <set>
#include <vector>

namespace poly = boost::polygon;
using namespace boost::polygon::operators;

namespace smtk {
  namespace bridge {
    namespace polygon {

typedef std::vector<std::pair<smtk::model::Edge,bool> > EdgesWithOrientation;

// Debug utility:
#if 0
template<typename T>
void printPointSeq(const char* msg, T start, T end)
{
  std::cout << msg << "\n";
  for (T it = start; it != end; ++it)
    {
    std::cout << "  " << it->x() << " " << it->y() << "\n";
    }
}
#endif // 0

/// Ensure that we are provided the proper edge orientations in addition to the usual checks.
bool ForceCreateFace::ableToOperate()
{
  // TODO: This should probably also verify that the "counts" item
  //       matches the number of points or edges provided:
  int method = this->findInt("construction method")->discreteIndex();
  smtk::attribute::IntItem::Ptr edgeDirItem = this->findInt("orientations");
  int numOrient = edgeDirItem->numberOfValues();
  return this->Superclass::ableToOperate() &&
    (method == ForceCreateFace::POINTS ||
     (method == ForceCreateFace::EDGES &&
      (numOrient == -1 ||
       numOrient == this->specification()->associations()->numberOfValues()
      )
     )
    );
}

/// Create one or more polygonal faces without sanity checks.
smtk::model::OperatorResult ForceCreateFace::operateInternal()
{
  int method = this->findInt("construction method")->discreteIndex();

  smtk::attribute::DoubleItem::Ptr pointsItem = this->findDouble("points");
  smtk::attribute::IntItem::Ptr coordinatesItem = this->findInt("coordinates");
  smtk::attribute::IntItem::Ptr countsItem = this->findInt("counts");
  smtk::attribute::IntItem::Ptr edgeDirItem = this->findInt("orientations");

  int numCoordsPerPt = coordinatesItem->value(0);

  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->associations();
  smtk::model::Model smodel;

  Session* sess = this->polygonSession();
  smtk::model::ManagerPtr mgr;
  if (!sess || !(mgr = sess->manager()))
    {
    // error logging requires mgr...
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // Obtain the SMTK and polygon models we will work with:
  switch (method)
    {
  case ForceCreateFace::POINTS:
    smodel = modelItem->value(0).as<smtk::model::Model>();
    break;
  case ForceCreateFace::EDGES:
    smodel = modelItem->value(0).as<smtk::model::Edge>().owningModel();
    break;
  default:
      {
      smtkErrorMacro(this->log(), "Unknown construction method " << method << ".");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    break;
    }
  internal::pmodel::Ptr pmodel = this->findStorage<internal::pmodel>(smodel.entity());
  if (!pmodel)
    {
    smtkErrorMacro(this->log(), "The associated model is not a polygon-session model.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // Sanity-check point coordinates array size:
  if (
    method == ForceCreateFace::POINTS &&
    pointsItem->numberOfValues() % numCoordsPerPt > 0)
    {
    smtkErrorMacro(this->log(),
      "Number of point-coordinates (" << pointsItem->numberOfValues() << ") " <<
      "not a multiple of the number of coordinates per pt (" << numCoordsPerPt << ")");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // I. While the counts array indicates we have faces to process:
  smtk::attribute::IntItem::value_type::const_iterator countIt;
  smtk::attribute::IntItem::value_type::const_iterator endCount;

  // Initialize iterators over things we consume as we create faces:
  smtk::attribute::DoubleItem::value_type::const_iterator coordIt = pointsItem->begin();
  smtk::attribute::ModelEntityItem::const_iterator edgeIt = modelItem->begin();
  smtk::attribute::IntItem::value_type::const_iterator edgeDirIt = edgeDirItem->begin();

  // Handle default value for "counts" item:
  std::vector<int> tmpCount;
  if (countsItem->numberOfValues() == 1 && countsItem->value(0) < 0)
    {
    // If given default value, create a simple and valid "counts" array to iterator over:
    tmpCount.push_back(
      method == ForceCreateFace::POINTS ?
      pointsItem->numberOfValues() / numCoordsPerPt :
      modelItem->numberOfValues());
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
  for (; countIt != endCount; )
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
        smtk::model::Edge modelEdge = pmodel->createModelEdgeFromPoints(mgr, polypts.begin(), polypts.end(), false);
        outerLoopEdges.push_back(std::make_pair(modelEdge, true));
        }
      else
        {
        this->pointsForLoop(polypts, *countIt, edgeIt, modelItem->end(), edgeDirIt, edgeDirItem->end(), outerLoopEdges);
        }
      //printPointSeq("outer loop", polypts.begin(), polypts.end());
      pface.set(polypts.begin(), polypts.end());
      }

    // III. Create a polygon set and add a polygon For each inner loop
    ++countIt;
    std::vector<poly::polygon_data<internal::Coord> > holes;
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
        smtk::model::Edge modelEdge = pmodel->createModelEdgeFromPoints(mgr, polypts.begin(), polypts.end(), false);
        innerLoopsEdges.back().push_back(std::make_pair(modelEdge, true));
        }
      else
        {
        this->pointsForLoop(polypts, *countIt, edgeIt, modelItem->end(), edgeDirIt, edgeDirItem->end(), innerLoopsEdges.back());
        }
      /*
       char holemsg[512];
       sprintf(holemsg, "  inner loop %d ", h);
       printPointSeq(holemsg, polypts.begin(), polypts.end());
       */
      // Add to polygon_set_data
      poly::polygon_data<internal::Coord> loop;
      loop.set(polypts.begin(), polypts.end());
      holes.push_back(loop);
      }
    pface.set_holes(holes.begin(), holes.end());

    // IV. Transcribe face (uses, loops, face) to smtk
    //
    // Make up some UUIDs for the new entities.
    smtk::common::UUID modelFaceId = mgr->unusedUUID();
    smtk::common::UUID modelFaceUseId = mgr->unusedUUID();
    smtk::common::UUID outerLoopId = mgr->unusedUUID();
    // Transcribe the outer loop, face use, and face:
    if (!mgr->insertModelFaceWithOrientedOuterLoop(modelFaceId, modelFaceUseId, outerLoopId, outerLoopEdges))
      {
      smtkErrorMacro(this->log(), "Could not create SMTK outer loop of face.");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    smtk::model::Face modelFace(mgr, modelFaceId);
    smodel.addCell(modelFace);
    modelFace.assignDefaultName();
    created.push_back(modelFace);

    // Now transcribe inner-loop edges:
    std::size_t numInner = innerLoopsEdges.size();
    for (std::size_t inner = 0; inner < numInner; ++inner)
      {
      smtk::common::UUID innerLoopId = mgr->unusedUUID();
      if (!mgr->insertModelFaceOrientedInnerLoop(innerLoopId, outerLoopId, innerLoopsEdges[inner]))
        {
        smtkErrorMacro(this->log(), "Could not create SMTK inner loop of face.");
        return this->createResult(smtk::model::OPERATION_FAILED);
        }
      }

    // V. Add tessellation
    poly::polygon_set_data<internal::Coord> polys;
    std::vector<poly::polygon_data<internal::Coord> > tess;
    polys.insert(pface);
    polys.get_trapezoids(tess);
    std::vector<poly::polygon_data<internal::Coord> >::const_iterator pit;
    smtk::model::Tessellation blank;
    smtk::model::UUIDsToTessellations::iterator smtkTess =
      mgr->setTessellation(modelFaceId, blank);
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
      triConn[1] = smtkTess->second.addCoords(&smtkPt[0]);
      //std::cout << "  " << triConn[1] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      ++pcit;
      pmodel->liftPoint(*pcit, &smtkPt[0]);
      triConn[3] = smtkTess->second.addCoords(&smtkPt[0]);
      ++pcit;
      //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
      for (; pcit != poly::end_points(*pit); ++pcit)
        {
        triConn[2] = triConn[3];
        pmodel->liftPoint(*pcit, &smtkPt[0]);
        triConn[3] = smtkTess->second.addCoords(&smtkPt[0]);
        //std::cout << "  " << triConn[3] << "  " << smtkPt[0] << " " << smtkPt[1] << " " << smtkPt[2] << "\n";
        smtkTess->second.insertNextCell(triConn);
        }
      //std::cout << "\n";
      }
    }

  smtk::model::OperatorResult result =
    this->createResult(smtk::model::OPERATION_SUCCEEDED);
  this->addEntitiesToResult(result, created, CREATED);
  return result;
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  SMTKPOLYGONSESSION_EXPORT,
  smtk::bridge::polygon::ForceCreateFace,
  polygon_force_create_face,
  "force create face",
  ForceCreateFace_xml,
  smtk::bridge::polygon::Session);
