//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/Translate.h"

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

#include "smtk/bridge/cgm/Translate_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {
    namespace cgm {

smtk::model::OperatorResult Translate::operateInternal()
{
  smtk::attribute::DoubleItemPtr offset = this->findDouble("offset");

  ModelEntities bodiesIn = this->associatedEntitiesAs<ModelEntities>();

  ModelEntities::iterator it;
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

  GeometryQueryTool::instance()->translate(
    cgmEntitiesIn,
    offset->value(0), offset->value(1), offset->value(2),
    true, // (check before transforming)
    cgmEntitiesOut);
  if (cgmEntitiesOut.size() != nb)
    {
    smtkInfoMacro(log(), "Failed to translate bodies or wrong number"
      << " (" << cgmEntitiesOut.size() << " != " << nb << ")"
      << " of resulting bodies.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultEntities =
    result->findModelEntity("entities");

  Bridge* bridge = this->cgmBridge();
  int numEntitiesOut = cgmEntitiesOut.size();
  resultEntities->setNumberOfValues(numEntitiesOut);

  for (int i = 0; i < numEntitiesOut; ++i)
    {
    refEntity = cgmEntitiesOut.get_and_step();
    if (!refEntity)
      continue;

    smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(refEntity, true);
    smtk::common::UUID entId = refId->entityId();
    smtk::model::Cursor smtkEntry(this->manager(), entId);
    if (bridge->transcribe(smtkEntry, smtk::model::BRIDGE_EVERYTHING, false))
      resultEntities->setValue(i, smtkEntry);
    }

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::Translate,
  cgm_translate,
  "translate",
  Translate_xml,
  smtk::bridge::cgm::Bridge);
