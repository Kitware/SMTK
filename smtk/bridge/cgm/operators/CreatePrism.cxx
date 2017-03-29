//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/CreatePrism.h"

#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "Body.hpp"
#include "CGMApp.hpp"
#include "CubitAttribManager.hpp"
#include "CubitCompat.hpp"
#include "CubitDefines.h"
#include "DLIList.hpp"
#include "DagType.hpp"
#include "GeometryModifyTool.hpp"
#include "GeometryQueryEngine.hpp"
#include "GeometryQueryTool.hpp"
#include "InitCGMA.hpp"
#include "RefEntity.hpp"
#include "RefEntityFactory.hpp"
#include "RefGroup.hpp"

#include "smtk/bridge/cgm/CreatePrism_xml.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

smtk::model::OperatorResult CreatePrism::operateInternal()
{
  smtk::attribute::DoubleItem::Ptr heightItem =
    this->specification()->findDouble("height");
  smtk::attribute::DoubleItem::Ptr majorRadiusItem =
    this->specification()->findDouble("major radius");
  smtk::attribute::DoubleItem::Ptr minorRadiusItem =
    this->specification()->findDouble("minor radius");
  smtk::attribute::IntItem::Ptr numberOfSidesItem =
    this->specification()->findInt("number of sides");

  int numberOfSides = numberOfSidesItem->value();
  double majorRadius = majorRadiusItem->value();
  double minorRadius = minorRadiusItem->value();
  double height = heightItem->value();

  //smtk::bridge::cgm::CAUUID::registerWithAttributeManager();
  //std::cout << "Default modeler \"" << GeometryQueryTool::instance()->get_gqe()->modeler_type() << "\"\n";
  //CubitStatus s;
  DLIList<RefEntity*> imported;
  //int prevAutoFlag = CGMApp::instance()->attrib_manager()->auto_flag();
  //CGMApp::instance()->attrib_manager()->auto_flag(CUBIT_TRUE);
  Body* cgmBody = GeometryModifyTool::instance()->prism(height, numberOfSides, majorRadius, minorRadius);
  //CGMApp::instance()->attrib_manager()->auto_flag(prevAutoFlag);
  if (!cgmBody)
    {
    smtkInfoMacro(log(), "Failed to create body.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);

  DLIList<Body*> cgmEntitiesOut;
  cgmEntitiesOut.push(cgmBody);
  this->addEntitiesToResult(cgmEntitiesOut, result, CREATED);
  // Nothing to expunge.

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  SMTKCGMSESSION_EXPORT,
  smtk::bridge::cgm::CreatePrism,
  cgm_create_prism,
  "create prism",
  CreatePrism_xml,
  smtk::bridge::cgm::Session);
