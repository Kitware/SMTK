// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/Delete.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Manager.txx"

#include "smtk/bridge/rgg/Session.h"
#include "smtk/bridge/rgg/operators/CreatePin.h"

#include "smtk/bridge/rgg/Delete_xml.h"

namespace smtk
{
namespace bridge
{
namespace rgg
{
using namespace smtk::model;

smtk::model::OperatorResult Delete::operateInternal()
{
  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_FAILED);

  if (entities.empty())
  {
    smtkErrorMacro(this->log(), "No entity to delete");
    return result;
  }

  EntityRefArray tobeDeleted;
  for (auto ent = entities.begin(); ent != entities.end(); ent++)
  {
    if (!ent->isAuxiliaryGeometry() || !ent->hasStringProperty("rggType"))
    {
      smtkErrorMacro(this->log(), "Non rgg entities cannot be deleted by"
                                  "this operator");
      return result;
    }
    if (ent->stringProperty("rggType")[0] != SMTK_BRIDGE_RGG_PIN)
    {
      smtkWarningMacro(this->log(), "Currently delete operator is only"
                                    "applicable to nuclear pin");
      continue;
    }
    // Nuclear pin
    if (ent->stringProperty("rggType")[0] == SMTK_BRIDGE_RGG_PIN)
    {
      // Delete the pin and its childrens
      EntityRefArray children = ent->as<AuxiliaryGeometry>().embeddedEntities<EntityRefArray>();
      tobeDeleted.push_back(*ent);
      tobeDeleted.insert(tobeDeleted.end(), children.begin(), children.end());
    }
  }
  // Delete entities from the manager
  smtk::model::EntityRefArray expunged;
  smtk::model::EntityRefArray modified;
  if (this->manager())
  {
    this->session()->manager()->deleteEntities(
      tobeDeleted, modified, expunged, this->m_debugLevel > 0);
  }

  result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
  this->addEntitiesToResult(result, modified, MODIFIED);
  this->addEntitiesToResult(result, expunged, EXPUNGED);
  smtkInfoMacro(this->log(), "Deleted " << tobeDeleted.size() << " of " << entities.size()
                                        << " requested entities");
  return result;
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::Delete, rgg_delete, "delete",
  Delete_xml, smtk::bridge::rgg::Session);
