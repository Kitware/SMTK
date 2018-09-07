//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/cgm/operators/CreatePrism.h"

#include "smtk/session/cgm/CAUUID.h"
#include "smtk/session/cgm/Engines.h"
#include "smtk/session/cgm/Session.h"
#include "smtk/session/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
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

#include "smtk/session/cgm/CreatePrism_xml.h"

namespace smtk
{
namespace session
{
namespace cgm
{

smtk::operation::OperationResult CreatePrism::operateInternal()
{
  smtk::attribute::DoubleItem::Ptr heightItem = this->specification()->findDouble("height");
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

  //smtk::session::cgm::CAUUID::registerWithAttributeManager();
  //std::cout << "Default modeler \"" << GeometryQueryTool::instance()->get_gqe()->modeler_type() << "\"\n";
  //CubitStatus s;
  DLIList<RefEntity*> imported;
  //int prevAutoFlag = CGMApp::instance()->attrib_manager()->auto_flag();
  //CGMApp::instance()->attrib_manager()->auto_flag(CUBIT_TRUE);
  Body* cgmBody =
    GeometryModifyTool::instance()->prism(height, numberOfSides, majorRadius, minorRadius);
  //CGMApp::instance()->attrib_manager()->auto_flag(prevAutoFlag);
  if (!cgmBody)
  {
    smtkInfoMacro(log(), "Failed to create body.");
    return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
  }

  smtk::operation::OperationResult result =
    this->createResult(smtk::operation::Operation::OPERATION_SUCCEEDED);

  DLIList<Body*> cgmEntitiesOut;
  cgmEntitiesOut.push(cgmBody);
  this->addEntitiesToResult(cgmEntitiesOut, result, CREATED);
  // Nothing to expunge.

  return result;
}

} // namespace cgm
} //namespace session
} // namespace smtk

smtkImplementsModelOperation(SMTKCGMSESSION_EXPORT, smtk::session::cgm::CreatePrism,
  cgm_create_prism, "create prism", CreatePrism_xml, smtk::session::cgm::Session);
