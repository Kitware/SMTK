//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/CreateVertex.h"

#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

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

#include "smtk/bridge/cgm/CreateVertex_xml.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

smtk::model::OperatorResult CreateVertex::operateInternal()
{
  smtk::attribute::DoubleItem::Ptr pointItem =
    this->specification()->findDouble("point");
  smtk::attribute::IntItem::Ptr colorItem =
    this->specification()->findInt("color");

  int color = colorItem->value();
  CubitVector point(
    pointItem->value(0),
    pointItem->value(1),
    pointItem->value(2));

  RefVertex* cgmVert = GeometryModifyTool::instance()->make_RefVertex(point, color);
  if (!cgmVert)
    {
    smtkInfoMacro(log(), "Failed to create vertex.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);

  DLIList<RefVertex*> cgmEntitiesOut;
  cgmEntitiesOut.push(cgmVert);
  this->addEntitiesToResult(cgmEntitiesOut, result, CREATED);
  // Nothing to expunge.

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  CGMSMTK_EXPORT,
  smtk::bridge::cgm::CreateVertex,
  cgm_create_vertex,
  "create vertex",
  CreateVertex_xml,
  smtk::bridge::cgm::Session);
