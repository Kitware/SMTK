//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/rgg/operators/EditPin.h"

#include "smtk/session/rgg/Resource.h"
#include "smtk/session/rgg/Session.h"

#include "smtk/session/rgg/operators/CreatePin.h"

#include "smtk/io/Logger.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Model.h"

#include "smtk/session/rgg/EditPin_xml.h"

#include <string> // std::to_string
using namespace smtk::model;

namespace smtk
{
namespace session
{
namespace rgg
{

EditPin::Result EditPin::operateInternal()
{
  Result result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  EntityRefArray entities = this->parameters()->associatedModelEntities<EntityRefArray>();
  if (entities.empty() || !entities[0].isAuxiliaryGeometry())
  {
    smtkErrorMacro(this->log(), "Cannot edit a non auxiliary geometry entity");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::model::EntityRefArray expunged, modified, tobeDeleted;

  smtk::model::AuxiliaryGeometry auxGeom = entities[0].as<AuxiliaryGeometry>();
  auto resource = std::dynamic_pointer_cast<smtk::session::rgg::Resource>(auxGeom.resource());
  // Remove all current child auxiliary geometries first
  EntityRefArray children = auxGeom.embeddedEntities<EntityRefArray>();
  auxGeom.setIntegerProperty("previous children size", static_cast<int>(children.size()));
  tobeDeleted.insert(tobeDeleted.end(), children.begin(), children.end());

  resource->deleteEntities(tobeDeleted, modified, expunged, this->m_debugLevel > 0);

  // A list contains all subparts and layers of the pin
  std::vector<EntityRef> subAuxGeoms;

  CreatePin::populatePin(this, auxGeom, subAuxGeoms);

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItem::Ptr createdItem = result->findComponent("created");
  for (auto& c : subAuxGeoms)
  {
    createdItem->appendValue(c.component());
  }

  smtk::attribute::ComponentItem::Ptr modifiedItem = result->findComponent("modified");
  modifiedItem->appendValue(auxGeom.component());
  for (auto& m : modified)
  {
    modifiedItem->appendValue(m.component());
  }

  smtk::attribute::ComponentItem::Ptr expungedItem = result->findComponent("expunged");
  for (auto& e : expunged)
  {
    expungedItem->appendValue(e.component());
  }

  return result;
}

const char* EditPin::xmlDescription() const
{
  return EditPin_xml;
}
} // namespace rgg
} //namespace session
} // namespace smtk
