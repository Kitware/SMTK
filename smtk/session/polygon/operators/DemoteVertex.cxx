//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/operators/DemoteVertex.h"

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/Session.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Model.txx"

#include "smtk/io/Logger.h"

#include "smtk/model/Vertex.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/polygon/operators/DemoteVertex_xml.h"

namespace smtk
{
namespace session
{
namespace polygon
{

DemoteVertex::Result DemoteVertex::operateInternal()
{
  auto vertItem = this->parameters()->associations();
  smtk::model::Vertex vertexToDemote(vertItem->valueAs<smtk::model::Entity>());

  if (!vertexToDemote.isValid())
  {
    smtkErrorMacro(this->log(), "The input vertex (" << vertexToDemote.entity() << ") is invalid.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(
      vertexToDemote.component()->resource());

  internal::vertex::Ptr storage = resource->findStorage<internal::vertex>(vertexToDemote.entity());
  internal::pmodel* mod = storage->parentAs<internal::pmodel>();
  if (!storage || !mod)
  {
    smtkErrorMacro(this->log(), "The input vertex has no storage or no parent model set.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::model::EntityRefs created;
  smtk::model::EntityRefs modified;
  smtk::model::EntityRefs expunged;
  bool ok = mod->demoteModelVertex(resource, storage, created, modified, expunged);
  Result opResult;
  if (ok)
  {
    opResult = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

    smtk::attribute::ComponentItem::Ptr createdItem = opResult->findComponent("created");
    for (auto it = created.begin(); it != created.end(); ++it)
    {
      createdItem->appendValue(it->component());
    }

    smtk::attribute::ComponentItem::Ptr modifiedItem = opResult->findComponent("modified");
    for (auto it = modified.begin(); it != modified.end(); ++it)
    {
      modifiedItem->appendValue(it->component());
    }

    smtk::attribute::ComponentItem::Ptr expungedItem = opResult->findComponent("expunged");
    for (auto it = expunged.begin(); it != expunged.end(); ++it)
    {
      expungedItem->appendValue(it->component());
    }

    operation::MarkGeometry(resource).markResult(opResult);
  }
  else
  {
    opResult = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  return opResult;
}

const char* DemoteVertex::xmlDescription() const
{
  return DemoteVertex_xml;
}

} // namespace polygon
} //namespace session
} // namespace smtk
