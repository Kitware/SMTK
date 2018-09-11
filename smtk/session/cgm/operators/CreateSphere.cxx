//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/cgm/operators/CreateSphere.h"

#include "smtk/session/cgm/CAUUID.h"
#include "smtk/session/cgm/Engines.h"
#include "smtk/session/cgm/Session.h"
#include "smtk/session/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
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

#include "smtk/session/cgm/CreateSphere_xml.h"

namespace smtk
{
namespace session
{
namespace cgm
{

smtk::operation::OperationResult CreateSphere::operateInternal()
{
  smtk::attribute::DoubleItem::Ptr centerItem = this->specification()->findDouble("center");
  smtk::attribute::DoubleItem::Ptr radiusItem = this->specification()->findDouble("radius");
  smtk::attribute::DoubleItem::Ptr innerRadiusItem =
    this->specification()->findDouble("inner radius");

  double center[3];
  double radius = radiusItem->value();
  double innerRadius = innerRadiusItem->value();
  for (int i = 0; i < 3; ++i)
    center[i] = centerItem->value(i);

  //smtk::session::cgm::CAUUID::registerWithAttributeManager();
  //std::cout << "Default modeler \"" << GeometryQueryTool::instance()->get_gqe()->modeler_type() << "\"\n";
  //CubitStatus s;
  DLIList<RefEntity*> imported;
  //int prevAutoFlag = CGMApp::instance()->attrib_manager()->auto_flag();
  //CGMApp::instance()->attrib_manager()->auto_flag(CUBIT_TRUE);
  Body* cgmBody = GeometryModifyTool::instance()->sphere(radius, 0., 0., 0., innerRadius);
  //CGMApp::instance()->attrib_manager()->auto_flag(prevAutoFlag);
  if (!cgmBody)
  {
    smtkInfoMacro(log(), "Failed to create body.");
    return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
  }

  // Do this separately because CGM's sphere() method is broken (for OCC at a minimum).
  CubitVector delta(center[0], center[1], center[2]);
#if CGM_MAJOR_VERSION >= 15
  DLIList<Body*> cgmBodies;
  cgmBodies.push(cgmBody);
  CubitStatus status = GeometryQueryTool::instance()->translate(cgmBodies, delta);
#else
  CubitStatus status = GeometryQueryTool::instance()->translate(cgmBody, delta, center[2]);
#endif
  if (status != CUBIT_SUCCESS)
  {
    smtkInfoMacro(log(), "Failed to translate body.");
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

smtkImplementsModelOperation(SMTKCGMSESSION_EXPORT, smtk::session::cgm::CreateSphere,
  cgm_create_sphere, "create sphere", CreateSphere_xml, smtk::session::cgm::Session);
