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
#include "Body.hpp"

#include "smtk/bridge/cgm/CreatePrism_xml.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

// local helper
bool CreatePrism::ableToOperate()
{
  return this->specification()->isValid();
}

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
    std::cerr << "Failed to create body\n";
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultBodies =
    result->findModelEntity("bodies");

  Bridge* bridge = this->cgmBridge();
  resultBodies->setNumberOfValues(1);

  smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(cgmBody, true);
  smtk::common::UUID entId = refId->entityId();
  smtk::model::Cursor smtkEntry(this->manager(), entId);
  if (bridge->transcribe(smtkEntry, smtk::model::BRIDGE_EVERYTHING, false))
    resultBodies->setValue(0, smtkEntry);

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::CreatePrism,
  cgm_create_prism,
  "create prism",
  CreatePrism_xml,
  smtk::bridge::cgm::Bridge);
