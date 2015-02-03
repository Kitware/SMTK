//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/Rotate.h"

#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/DoubleItem.h"
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

#include "smtk/bridge/cgm/Rotate_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {
    namespace cgm {

smtk::model::OperatorResult Rotate::operateInternal()
{
  smtk::attribute::DoubleItemPtr centerItem = this->findDouble("center");
  smtk::attribute::DoubleItemPtr axisItem = this->findDouble("axis");
  smtk::attribute::DoubleItemPtr angleItem = this->findDouble("angle");

  Models bodiesIn = this->associatedEntitiesAs<Models>();

  Models::iterator it;
  DLIList<RefEntity*> cgmEntitiesIn;
  DLIList<RefEntity*> cgmEntitiesOut;
  RefEntity* refEntity;
  for (it = bodiesIn.begin(); it != bodiesIn.end(); ++it)
    {
    refEntity = this->cgmEntity(*it);
    if (refEntity)
      {
      cgmEntitiesIn.append(refEntity);
      this->manager()->erase(*it); // We will re-transcribe momentarily. TODO: This could be more efficient.
      }
    }

  int nb = cgmEntitiesIn.size();

  CubitVector center(centerItem->value(0), centerItem->value(1), centerItem->value(2));
  CubitVector axis(axisItem->value(0), axisItem->value(1), axisItem->value(2));
  if (axis.normalize() == 0)
    {
    smtkInfoMacro(log(),
      "Ill-defined rotation: given axis of rotation is a zero-length vector.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }
  double angle = angleItem->value(0);
  GeometryQueryTool::instance()->rotate(
    cgmEntitiesIn,
    center, axis, angle,
    true, // (check before transforming)
    cgmEntitiesOut);
  if (cgmEntitiesOut.size() != nb)
    {
    smtkInfoMacro(log(), "Failed to rotate bodies or wrong number"
      << " (" << cgmEntitiesOut.size() << " != " << nb << ")"
      << " of resulting bodies.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultEntities =
    result->findModelEntity("entities");

  Session* session = this->cgmSession();
  int numEntitiesOut = cgmEntitiesOut.size();
  resultEntities->setNumberOfValues(numEntitiesOut);

  for (int i = 0; i < numEntitiesOut; ++i)
    {
    refEntity = cgmEntitiesOut.get_and_step();
    if (!refEntity)
      continue;

    smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(refEntity, true);
    smtk::common::UUID entId = refId->entityId();
    smtk::model::EntityRef smtkEntry(this->manager(), entId);
    if (session->transcribe(smtkEntry, smtk::model::SESSION_EVERYTHING, false))
      resultEntities->setValue(i, smtkEntry);
    }

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::Rotate,
  cgm_rotate,
  "rotate",
  Rotate_xml,
  smtk::bridge::cgm::Session);
