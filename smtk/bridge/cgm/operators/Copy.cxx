//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/Copy.h"

#include "smtk/bridge/cgm/Bridge.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "Body.hpp"
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

#include "smtk/bridge/cgm/Copy_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {
    namespace cgm {

// local helper
bool Copy::ableToOperate()
{
  return this->specification()->isValid();
}

smtk::model::OperatorResult Copy::operateInternal()
{
  CursorArray entitiesIn = this->associatedEntitiesAs<CursorArray>();
  if (entitiesIn.size() != 1)
    {
    smtkInfoMacro(log(), "Expected a single entity to copy but was given " << entitiesIn.size() << ".");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  Cursor entity = entitiesIn[0];
  RefEntity* cgmOut;
  if (entity.isModelEntity())
    {
    Body* refBody = this->cgmEntityAs<Body*>(entity);
    if (!refBody)
      {
      smtkInfoMacro(log(), "Unable obtain CGM body from " << entity.name() << ".");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    Body* newBody = GeometryModifyTool::instance()->copy_body(refBody);
    if (!newBody)
      {
      smtkInfoMacro(log(), "Unable to copy body " << refBody << " (" << entity.name() << ").");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    cgmOut = newBody;
    }
  else if (entity.isCellEntity())
    {
    RefEntity* refEntity = this->cgmEntity(entity);
    if (!refEntity)
      {
      smtkInfoMacro(log(), "Unable obtain CGM entity from " << entity.name() << ".");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    RefEntity* newEntity = GeometryModifyTool::instance()->copy_refentity(refEntity);
    if (!newEntity)
      {
      smtkInfoMacro(log(), "Unable to copy entity " << refEntity << " (" << entity.name() << ").");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    cgmOut = newEntity;
    }
  else
    {
    smtkInfoMacro(log(),
      "Expected a cell (vertex, edge, face, volume) or model but was given "
      << entity.flagSummary(0) << " (named " << entity.name() << ").");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultEntities =
    result->findModelEntity("entities");

  Bridge* bridge = this->cgmBridge();
  resultEntities->setNumberOfValues(1);

  smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(cgmOut, true);
  smtk::common::UUID entId = refId->entityId();
  smtk::model::Cursor smtkEntry(this->manager(), entId);
  if (bridge->transcribe(smtkEntry, smtk::model::BRIDGE_EVERYTHING, false))
    resultEntities->setValue(0, smtkEntry);

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::Copy,
  cgm_copy,
  "copy",
  Copy_xml,
  smtk::bridge::cgm::Bridge);
