//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/CreateFaceOperator.h"

#include "smtk/bridge/cgm/Bridge.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "CGMApp.hpp"
#include "DagType.hpp"
#include "CubitAttribManager.hpp"
#include "CubitCompat.hpp"
#include "CubitDefines.h"
#include "DLIList.hpp"
#include "InitCGMA.hpp"
#include "GeometryModifyTool.hpp"
#include "GeometryQueryEngine.hpp"
#include "GeometryQueryTool.hpp"
#include "RefEntity.hpp"
#include "RefEntityFactory.hpp"
#include "RefGroup.hpp"
#include "RefVertex.hpp"
#include "RefFace.hpp"

#include "smtk/bridge/cgm/CreateFaceOperator_xml.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

// local helper
bool CreateFaceOperator::ableToOperate()
{
  return this->specification()->isValid();
}

smtk::model::OperatorResult CreateFaceOperator::operateInternal()
{
  smtk::attribute::ModelEntityItem::Ptr verticesItem =
    this->findModelEntity("vertices");
  smtk::attribute::DoubleItem::Ptr pointItem =
    this->findDouble("point");
  smtk::attribute::IntItem::Ptr curveTypeItem =
    this->findInt("curve type");
  smtk::attribute::IntItem::Ptr colorItem =
    this->findInt("color");

  int color = colorItem->value();
  CubitVector point(
    pointItem->value(0),
    pointItem->value(1),
    pointItem->value(2));
  GeometryType curveType = static_cast<GeometryType>(
    curveTypeItem->concreteDefinition()->discreteValue(
      curveTypeItem->discreteIndex()));
  switch (curveType)
    {
  case STRAIGHT_CURVE_TYPE: //    intermediate_point_ptr  is not used
  case PARABOLA_CURVE_TYPE: //    intermediate_point_ptr is the tip of the parabola
  case HYPERBOLA_CURVE_TYPE: //    intermediate_point_ptr is the center of its two foci
  case ELLIPSE_CURVE_TYPE:
    //    intermediate_point_ptr is the center of the ellipse
    //    the two points are vertices, one gives the major radius,
    //    the other point gives the minor radius.
  case ARC_CURVE_TYPE: //    arc passes three points
    break;
  default:
    std::cerr << "Bad curve type " << curveType << "\n";
    return this->createResult(smtk::model::OPERATION_FAILED);
    }
  RefVertex* v0 = this->cgmEntityAs<RefVertex*>(verticesItem->value(0));
  RefVertex* v1 = this->cgmEntityAs<RefVertex*>(verticesItem->value(1));
  if (!v0 || !v1)
    {
    std::cerr << "One or more vertices were invalid " << v0 << ", " << v1 << "\n";
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  RefFace* cgmFace = GeometryModifyTool::instance()->make_RefFace(curveType, v0, v1, &point);
  if (!cgmFace)
    {
    std::cerr << "Failed to create face\n";
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // Assign color to match vertex API that requires a color.
  cgmFace->color(color);

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultVert =
    result->findModelEntity("face");

  Bridge* bridge = this->cgmBridge();
  smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(cgmFace, true);
  smtk::common::UUID entId = refId->entityId();
  smtk::model::Cursor smtkEntry(this->manager(), entId);
  if (bridge->transcribe(smtkEntry, smtk::model::BRIDGE_EVERYTHING, false))
    resultVert->setValue(0, smtkEntry);

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::CreateFaceOperator,
  cgm_create_face,
  "create face",
  CreateFaceOperator_xml,
  smtk::bridge::cgm::Bridge);
