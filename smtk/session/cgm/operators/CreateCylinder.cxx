//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/cgm/operators/CreateCylinder.h"

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

#include "smtk/session/cgm/CreateCylinder_xml.h"

namespace smtk
{
namespace session
{
namespace cgm
{

smtk::operation::OperationResult CreateCylinder::operateInternal()
{
  smtk::attribute::DoubleItem::Ptr heightItem = this->specification()->findDouble("height");
  smtk::attribute::DoubleItem::Ptr majorBaseRadiusItem =
    this->specification()->findDouble("major base radius");
  smtk::attribute::DoubleItem::Ptr minorBaseRadiusItem =
    this->specification()->findDouble("minor base radius");
  smtk::attribute::DoubleItem::Ptr majorTopRadiusItem =
    this->specification()->findDouble("major top radius");

  double height = heightItem->value();
  double majorTopRadius = majorTopRadiusItem->value();
  double majorBaseRadius = majorBaseRadiusItem->value();
  double minorBaseRadius = minorBaseRadiusItem->value();

  //smtk::session::cgm::CAUUID::registerWithAttributeManager();
  //std::cout << "Default modeler \"" << GeometryQueryTool::instance()->get_gqe()->modeler_type() << "\"\n";
  //CubitStatus s;
  DLIList<RefEntity*> imported;
  //int prevAutoFlag = CGMApp::instance()->attrib_manager()->auto_flag();
  //CGMApp::instance()->attrib_manager()->auto_flag(CUBIT_TRUE);
  Body* cgmBody = GeometryModifyTool::instance()->cylinder(
    height, majorBaseRadius, minorBaseRadius, majorTopRadius);
  //CGMApp::instance()->attrib_manager()->auto_flag(prevAutoFlag);
  if (!cgmBody)
  {
    smtkInfoMacro(log(), "Failed to create body.");
    return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
  }

  smtk::operation::OperationResult result =
    this->createResult(smtk::operation::Operation::OPERATION_SUCCEEDED);

  DLIList<Body*> cgmBodiesOut;
  cgmBodiesOut.push(cgmBody);
  this->addEntitiesToResult(cgmBodiesOut, result, CREATED);
  // Nothing to expunge.

  return result;
}

} // namespace cgm
} //namespace session
} // namespace smtk

smtkImplementsModelOperation(SMTKCGMSESSION_EXPORT, smtk::session::cgm::CreateCylinder,
  cgm_create_cylinder, "create cylinder", CreateCylinder_xml, smtk::session::cgm::Session);
