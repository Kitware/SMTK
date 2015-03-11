//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/CreateEdge.h"

#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/model/Vertex.h"

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
#include "RefEdge.hpp"

#include "smtk/bridge/cgm/CreateEdge_xml.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

smtk::model::OperatorResult CreateEdge::operateInternal()
{
  smtk::model::Vertices vertices =
    this->associatedEntitiesAs<smtk::model::Vertices>();
  if (vertices.size() != 2)
    {
    smtkInfoMacro(log(), "Expected 2 vertices, got " << vertices.size() << ".");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

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
    smtkInfoMacro(log(), "Bad curve type " << curveType << ".");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }
  RefVertex* v0 = this->cgmEntityAs<RefVertex*>(vertices[0]);
  RefVertex* v1 = this->cgmEntityAs<RefVertex*>(vertices[1]);
  if (!v0 || !v1)
    {
    smtkInfoMacro(log(), "One or more vertices were invalid " << v0 << ", " << v1 << ".");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  RefEdge* cgmEdge = GeometryModifyTool::instance()->make_RefEdge(curveType, v0, v1, &point);
  if (!cgmEdge)
    {
    smtkInfoMacro(log(), "Failed to create edge.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // Assign color to match vertex API that requires a color.
  cgmEdge->color(color);

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);

  DLIList<RefEdge*> cgmEdgesOut;
  cgmEdgesOut.push(cgmEdge);
  this->addEntitiesToResult(cgmEdgesOut, result, CREATED);
  // Nothing to expunge.

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::CreateEdge,
  cgm_create_edge,
  "create edge",
  CreateEdge_xml,
  smtk::bridge::cgm::Session);
