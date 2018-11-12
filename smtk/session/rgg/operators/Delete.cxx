// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/rgg/operators/Delete.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Resource.txx"
#include "smtk/model/ShellEntity.txx"

#include "smtk/session/rgg/Session.h"
#include "smtk/session/rgg/operators/CreateDuct.h"
#include "smtk/session/rgg/operators/CreatePin.h"

#include "smtk/session/rgg/Delete_xml.h"

namespace smtk
{
namespace session
{
namespace rgg
{
using namespace smtk::model;

Delete::Result Delete::operateInternal()
{
  EntityRefArray entities = this->parameters()->associatedModelEntities<EntityRefArray>();
  Result result = this->createResult(smtk::operation::Operation::Outcome::FAILED);

  if (entities.empty())
  {
    smtkErrorMacro(this->log(), "No entity to delete");
    return result;
  }

  smtk::session::rgg::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::rgg::Resource>(entities[0].component()->resource());

  EntityRefArray tobeDeleted;
  for (auto ent = entities.begin(); ent != entities.end(); ent++)
  {
    if ((!ent->isAuxiliaryGeometry() || !ent->hasStringProperty("rggType")) && !ent->isInstance())
    {
      smtkErrorMacro(this->log(), "Non rgg entities cannot be deleted by "
                                  "this operator");
      return result;
    }
    // FIXME: Remove me when we have full support for rgg entities
    if (ent->hasStringProperty("rggType") &&
      ent->stringProperty("rggType")[0] != SMTK_SESSION_RGG_PIN &&
      ent->stringProperty("rggType")[0] != SMTK_SESSION_RGG_DUCT)
    {
      smtkWarningMacro(this->log(), "Currently delete operator is only"
                                    "applicable to nuclear pin, duct");
      continue;
    }
    // Nuclear pin and duct
    if (ent->hasStringProperty("rggType") &&
      (ent->stringProperty("rggType")[0] == SMTK_SESSION_RGG_PIN ||
          ent->stringProperty("rggType")[0] == SMTK_SESSION_RGG_DUCT))
    {
      // Delete the pin and its childrens
      // Corresponding instances would also get deleted
      EntityRefArray children = ent->as<AuxiliaryGeometry>().embeddedEntities<EntityRefArray>();
      tobeDeleted.push_back(*ent);
      tobeDeleted.insert(tobeDeleted.end(), children.begin(), children.end());
    }
    // Nuclear instance
    if (ent->isInstance())
    {
      tobeDeleted.push_back(*ent);
    }
  }
  // Delete entities from the resource
  smtk::model::EntityRefArray expunged;
  smtk::model::EntityRefArray modified;
  resource->deleteEntities(tobeDeleted, modified, expunged, this->m_debugLevel > 0);

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItem::Ptr modifiedItem = result->findComponent("modified");
  for (auto& m : modified)
  {
    modifiedItem->appendValue(m.component());
  }
  smtk::attribute::ComponentItem::Ptr expungedItem = result->findComponent("expunged");
  for (auto& e : expunged)
  {
    expungedItem->appendValue(e.component());
  }

  smtkInfoMacro(this->log(), "Deleted " << tobeDeleted.size() << " of " << entities.size()
                                        << " requested entities");
  return result;
}

const char* Delete::xmlDescription() const
{
  return Delete_xml;
}
} // namespace rgg
} //namespace session
} // namespace smtk
