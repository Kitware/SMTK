//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/opencv/operators/SurfaceExtractContours.h"

#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/operators/CreateEdge.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Model.h"
#include "smtk/model/SessionRef.h"

#include "smtk/extension/opencv/SurfaceExtractContours_xml.h"

#include "cJSON.h"
#include "smtk/io/SaveJSON.h"
#include <limits>
#include <sstream>

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace polygon
{

bool SurfaceExtractContours::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  smtk::model::AuxiliaryGeometry aux =
    this->parameters()->associations()->valueAs<smtk::model::Entity>();
  if (!aux.isValid())
  {
    return false;
  }
  std::string url = aux.url();
  if (url.empty())
  {
    return false;
  }

  return true;
}

namespace
{

int internal_createEdge(smtk::bridge::polygon::CreateEdge::Ptr edgeOp,
  smtk::attribute::AttributePtr opParams, smtk::model::EntityRefArray& createdEds,
  const smtk::model::Model& model, smtk::io::Logger& logger)
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
  if (edgeResult->findInt("outcome")->value() !=
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
}

SurfaceExtractContours::Result SurfaceExtractContours::operateInternal()
{
  // ableToOperate should have verified that aux is valid
  smtk::model::AuxiliaryGeometry aux =
    this->parameters()->associations()->valueAs<smtk::model::Entity>();
  smtk::model::Model model = aux.owningModel();

  smtk::bridge::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::polygon::Resource>(model.component()->resource());

  SessionPtr opsession = resource->polygonSession();

  smtk::model::EntityRefArray newEdges;

  smtk::attribute::DoubleItem::Ptr pointsItem =
    this->parameters()->findAs<smtk::attribute::DoubleItem>(
      "points", smtk::attribute::ALL_CHILDREN);

  CreateEdge::Ptr edgeOp = CreateEdge::create();
  if (!edgeOp)
  {
    smtkInfoMacro(log(), "Failed to create CreateEdge op.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  int numEdges = internal_createEdge(edgeOp, this->parameters(), newEdges, model, log());
  Result result = this->createResult(numEdges > 0 ? smtk::operation::Operation::Outcome::SUCCEEDED
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
  }
  return result;
}

const char* SurfaceExtractContours::xmlDescription() const
{
  return SurfaceExtractContours_xml;
}

} // namespace polygon
} // namespace bridge

} // namespace smtk
