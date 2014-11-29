//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/CreateBrick.h"

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

#include "smtk/bridge/cgm/CreateBrick_xml.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

// local helper
bool CreateBrick::ableToOperate()
{
  return this->specification()->isValid();
}

smtk::model::OperatorResult CreateBrick::operateInternal()
{
  using smtk::attribute::SearchStyle;

  // Figure out which variant of the method to use.
  smtk::attribute::IntItem::Ptr methodItem = this->findInt("construction method");
  smtk::attribute::DoubleItem::Ptr widthItem = this->findDouble("width");
  smtk::attribute::DoubleItem::Ptr depthItem = this->findDouble("depth");
  smtk::attribute::DoubleItem::Ptr heightItem = this->findDouble("height");

  smtk::attribute::DoubleItem::Ptr centerItem = this->findDouble("center");
  smtk::attribute::DoubleItem::Ptr axis0Item = this->findDouble("axis 0");
  smtk::attribute::DoubleItem::Ptr axis1Item = this->findDouble("axis 1");
  smtk::attribute::DoubleItem::Ptr axis2Item = this->findDouble("axis 2");
  smtk::attribute::DoubleItem::Ptr extensionItem = this->findDouble("extension");

  Body* cgmBody;
  int method = methodItem->discreteIndex(0);
  switch (method)
    {
  case 0: // axis-aligned cuboid
      {
      double width = widthItem->value();
      double depth = depthItem->value();
      double height = heightItem->value();

      cgmBody = GeometryModifyTool::instance()->brick(width, depth, height);
      }
    break;
  case 1: // parallelepiped
      {
      CubitVector center;
      CubitVector axes[3];
      CubitVector extension;

      center.x(centerItem->value(0));
      center.y(centerItem->value(1));
      center.z(centerItem->value(2));

      axes[0].x(axis0Item->value(0));
      axes[0].y(axis0Item->value(1));
      axes[0].z(axis0Item->value(2));

      axes[1].x(axis1Item->value(0));
      axes[1].y(axis1Item->value(1));
      axes[1].z(axis1Item->value(2));

      axes[2].x(axis2Item->value(0));
      axes[2].y(axis2Item->value(1));
      axes[2].z(axis2Item->value(2));

      extension.x(extensionItem->value(0));
      extension.y(extensionItem->value(1));
      extension.z(extensionItem->value(2));

      cgmBody = GeometryModifyTool::instance()->brick(center, axes, extension);
      }
    break;
  default:
    cgmBody = NULL;
    }
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
  smtk::bridge::cgm::CreateBrick,
  cgm_create_brick,
  "create brick",
  CreateBrick_xml,
  smtk::bridge::cgm::Bridge);
