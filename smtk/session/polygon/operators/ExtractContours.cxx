//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/polygon/operators/ExtractContours.h"

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/Session.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/operators/CreateEdge.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Model.h"
#include "smtk/model/SessionRef.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/session/polygon/operators/ExtractContours_xml.h"

#include <limits>
#include <sstream>

using namespace smtk::model;

namespace smtk
{
namespace session
{

namespace polygon
{

bool ExtractContours::ableToOperate()
{
  if (!smtk::operation::Operation::ableToOperate())
  {
    return false;
  }

  smtk::model::AuxiliaryGeometry aux(
    this->parameters()->associations()->valueAs<smtk::model::Entity>());
  if (!aux.isValid())
  {
    return false;
  }
  std::string url = aux.url();
  return !url.empty();
}

int internal_createEdge(
  smtk::session::polygon::CreateEdge::Ptr edgeOp,
  smtk::attribute::AttributePtr opParams,
  smtk::model::EntityRefArray& createdEds,
  const smtk::model::Model& model,
  smtk::io::Logger& logger)
{
  smtk::attribute::AttributePtr params = edgeOp->parameters();
  params->associateEntity(model);
  smtk::attribute::IntItem::Ptr constructMethod = params->findInt("construction method");
  constructMethod->setDiscreteIndex(0); // "points coornidates"
  smtk::attribute::IntItem::Ptr numCoords = params->findInt("coordinates");
  smtk::attribute::DoubleItem::Ptr pointsItem = params->findDouble("points");
  smtk::attribute::ConstItemPtr sourceItem =
    opParams->find("points", smtk::attribute::ALL_CHILDREN);
  pointsItem->assign(sourceItem);
  smtk::attribute::IntItem::Ptr offsetsItem = params->findInt("offsets");
  sourceItem = opParams->find("offsets", smtk::attribute::ALL_CHILDREN);
  offsetsItem->assign(sourceItem);
  sourceItem = opParams->find("coordinates", smtk::attribute::ALL_CHILDREN);
  numCoords->assign(sourceItem); // number of elements in coordinates

  smtk::operation::Operation::Result edgeResult = edgeOp->operate();
  if (
    edgeResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    smtkDebugMacro(logger, "\"create edge\" op failed to creat edge with given line cells.");
    return 0;
  }
  smtk::attribute::ComponentItem::Ptr newEdges = edgeResult->findComponent("created");
  for (auto it = newEdges->begin(); it != newEdges->end(); ++it)
  {
    createdEds.push_back(
      std::dynamic_pointer_cast<smtk::model::Entity>(*it)->referenceAs<smtk::model::Edge>());
  }
  return static_cast<int>(createdEds.size());
}

ExtractContours::Result ExtractContours::operateInternal()
{
  // ableToOperate should have verified that aux is valid
  smtk::model::AuxiliaryGeometry aux =
    this->parameters()->associations()->valueAs<smtk::model::Entity>();
  smtk::model::Model model = aux.owningModel();

  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(model.component()->resource());

  SessionPtr opsession = resource->polygonSession();

  bool noExistingTess = model.entitiesWithTessellation().empty();
  internal::pmodel::Ptr storage = resource->findStorage<internal::pmodel>(model.entity());
  smtk::attribute::DoubleItem::Ptr boundsItem =
    this->parameters()->findAs<smtk::attribute::DoubleItem>(
      "image bounds", smtk::attribute::ALL_CHILDREN);
  // if there is no entities with tessellation in the model before this op,
  // we will try to set the origin of the model to be center of the image bounds
  // if it is valid. The reason is that by default a new model's origin is (0, 0, 0),
  // and its feature size is 1e-6,  so if the image that these contours are extracted
  // from has large coordinates numbers, the actual projected points in storage will
  // have very big numbers (see pmodel::projectPoint) which could lead the finding of
  // point coordinates from vtk to points in storage difficult, for example,
  // pmodel::splitModelEdgeAtModelVertex, which is used by SplitEdge op to find an edge
  // point given coordinates converted from VTK to do a split.
  if (storage && boundsItem && noExistingTess)
  {
    double bounds[6];
    for (int i = 0; i < 6; ++i)
      bounds[i] = boundsItem->value(i);
    if (bounds[0] < bounds[1] && bounds[2] < bounds[3] && bounds[4] <= bounds[5])
    {
      for (int j = 0; j < 3; ++j)
      {
        storage->origin()[j] = (bounds[2 * j + 1] + bounds[2 * j]) / 2.0;
      }
      model.setFloatProperty(
        "origin", smtk::model::FloatList(storage->origin(), storage->origin() + 3));
    }
  }

  smtk::model::EntityRefArray newEdges;

  smtk::attribute::DoubleItem::Ptr pointsItem =
    this->parameters()->findAs<smtk::attribute::DoubleItem>(
      "points", smtk::attribute::ALL_CHILDREN);

  smtk::session::polygon::CreateEdge::Ptr edgeOp = smtk::session::polygon::CreateEdge::create();
  if (!edgeOp)
  {
    smtkInfoMacro(log(), "Failed to create CreateEdge op.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  int numEdges =
    internal_createEdge(edgeOp, this->parameters(), newEdges, aux.owningModel(), log());

  Result result = this->createResult(
    numEdges > 0 ? smtk::operation::Operation::Outcome::SUCCEEDED
                 : smtk::operation::Operation::Outcome::FAILED);

  if (numEdges > 0)
  {
    smtk::attribute::ComponentItem::Ptr createdItem = result->findComponent("created");
    for (auto it = newEdges.begin(); it != newEdges.end(); ++it)
    {
      createdItem->appendValue(it->component());
    }
    smtk::attribute::ComponentItem::Ptr modifiedItem = result->findComponent("modified");
    modifiedItem->appendValue(aux.owningModel().component());

    operation::MarkGeometry(resource).markResult(result);
  }

  return result;
}

const char* ExtractContours::xmlDescription() const
{
  return ExtractContours_xml;
}

} // namespace polygon
} // namespace session
} // namespace smtk
