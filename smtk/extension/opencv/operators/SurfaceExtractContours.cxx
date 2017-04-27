//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "SurfaceExtractContours.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SessionRef.h"

#include "smtk/bridge/polygon/SurfaceExtractContours_xml.h"

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
  if (!this->ensureSpecification())
  {
    return false;
  }

  smtk::model::AuxiliaryGeometry aux =
    this->specification()->associations()->value().as<smtk::model::AuxiliaryGeometry>();
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

int internal_createEdge(smtk::model::Operator::Ptr edgeOp, smtk::attribute::AttributePtr opSpec,
  smtk::model::EntityRefArray& createdEds, const smtk::model::Model& model,
  smtk::io::Logger& logger)
{

  smtk::attribute::AttributePtr spec = edgeOp->specification();
  spec->associateEntity(model);
  smtk::attribute::IntItem::Ptr constructMethod = spec->findInt("construction method");
  constructMethod->setDiscreteIndex(0); // "points coornidates"
  smtk::attribute::IntItem::Ptr numCoords = spec->findInt("coordinates");
  smtk::attribute::DoubleItem::Ptr pointsItem = spec->findDouble("points");
  smtk::attribute::ConstItemPtr sourceItem = opSpec->find("points", smtk::attribute::ALL_CHILDREN);
  pointsItem->assign(sourceItem);
  smtk::attribute::IntItem::Ptr offsetsItem = spec->findInt("offsets");
  sourceItem = opSpec->find("offsets", smtk::attribute::ALL_CHILDREN);
  offsetsItem->assign(sourceItem);
  sourceItem = opSpec->find("coordinates", smtk::attribute::ALL_CHILDREN);
  numCoords->assign(sourceItem); // number of elements in coordinates

  OperatorResult edgeResult = edgeOp->operate();
  if (edgeResult->findInt("outcome")->value() != OPERATION_SUCCEEDED)
  {
    smtkDebugMacro(logger, "\"create edge\" op failed to creat edge with given line cells.");
    return 0;
  }
  smtk::attribute::ModelEntityItem::Ptr newEdges = edgeResult->findModelEntity("created");
  createdEds.insert(createdEds.end(), newEdges->begin(), newEdges->end());
  return static_cast<int>(createdEds.size());
}
}

OperatorResult SurfaceExtractContours::operateInternal()
{

  Session* opsession = this->polygonSession();
  // ableToOperate should have verified that aux is valid
  smtk::model::AuxiliaryGeometry aux =
    this->specification()->associations()->value().as<smtk::model::AuxiliaryGeometry>();
  smtk::model::Model model = aux.owningModel();

  smtk::model::EntityRefArray newEdges;

  smtk::attribute::DoubleItem::Ptr pointsItem =
    this->specification()->findAs<smtk::attribute::DoubleItem>(
      "points", smtk::attribute::ALL_CHILDREN);

  smtk::model::Operator::Ptr edgeOp = opsession->op("create edge");
  if (!edgeOp)
  {
    smtkInfoMacro(log(), "Failed to create CreateEdge op.");
    return this->createResult(OPERATION_FAILED);
  }
  int numEdges = internal_createEdge(edgeOp, this->specification(), newEdges, model, log());
  OperatorResult result = this->createResult(numEdges > 0 ? OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (numEdges > 0)
  {
    this->addEntitiesToResult(result, newEdges, CREATED);
    this->addEntityToResult(result, model, MODIFIED);
  }
  return result;
}

} // namespace polygon
} // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(SMTKOPENCVEXT_EXPORT, smtk::bridge::polygon::SurfaceExtractContours,
  opencv_surface_extract_contours, "extract surface contours", SurfaceExtractContours_xml,
  smtk::bridge::polygon::Session);
