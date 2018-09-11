//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/cgm/operators/Copy.h"

#include "smtk/session/cgm/CAUUID.h"
#include "smtk/session/cgm/Engines.h"
#include "smtk/session/cgm/Session.h"
#include "smtk/session/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Model.h"

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

#include "smtk/session/cgm/Copy_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace session
{
namespace cgm
{

smtk::operation::OperationResult Copy::operateInternal()
{
  EntityRefArray entitiesIn = this->associatedEntitiesAs<EntityRefArray>();
  if (entitiesIn.size() != 1)
  {
    smtkInfoMacro(
      log(), "Expected a single entity to copy but was given " << entitiesIn.size() << ".");
    return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
  }

  EntityRef entity = entitiesIn[0];
  RefEntity* cgmOut;
  if (entity.isModel())
  {
    Body* refBody = this->cgmEntityAs<Body*>(entity);
    if (!refBody)
    {
      smtkInfoMacro(log(), "Unable obtain CGM body from " << entity.name() << ".");
      return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
    }
    Body* newBody = GeometryModifyTool::instance()->copy_body(refBody);
    if (!newBody)
    {
      smtkInfoMacro(log(), "Unable to copy body " << refBody << " (" << entity.name() << ").");
      return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
    }
    cgmOut = newBody;
  }
  else if (entity.isCellEntity())
  {
    RefEntity* refEntity = this->cgmEntity(entity);
    if (!refEntity)
    {
      smtkInfoMacro(log(), "Unable obtain CGM entity from " << entity.name() << ".");
      return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
    }
    RefEntity* newEntity = GeometryModifyTool::instance()->copy_refentity(refEntity);
    if (!newEntity)
    {
      smtkInfoMacro(log(), "Unable to copy entity " << refEntity << " (" << entity.name() << ").");
      return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
    }
    cgmOut = newEntity;
  }
  else
  {
    smtkInfoMacro(log(), "Expected a cell (vertex, edge, face, volume) or model but was given "
        << entity.flagSummary(0) << " (named " << entity.name() << ").");
    return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
  }

  smtk::operation::OperationResult result =
    this->createResult(smtk::operation::Operation::OPERATION_SUCCEEDED);

  DLIList<RefEntity*> cgmEntitiesOut;
  cgmEntitiesOut.push(cgmOut);
  this->addEntitiesToResult(cgmEntitiesOut, result, MODIFIED);
  // Nothing to expunge.

  return result;
}

} // namespace cgm
} //namespace session
} // namespace smtk

smtkImplementsModelOperation(SMTKCGMSESSION_EXPORT, smtk::session::cgm::Copy, cgm_copy, "copy",
  Copy_xml, smtk::session::cgm::Session);
