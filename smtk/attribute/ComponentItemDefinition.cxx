//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/ComponentItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Container.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/common/UUID.h"

#include "smtk/attribute/ReferenceItem.txx"
#include "smtk/attribute/ReferenceItemDefinition.txx"

#include <cassert>

using namespace smtk::attribute;

/// Construct an item definition given a name. Names should be unique and non-empty.
ComponentItemDefinition::ComponentItemDefinition(const std::string& sname)
  : Superclass(sname)
{
}

/// Destructor.
ComponentItemDefinition::~ComponentItemDefinition()
{
}

/// Return the type of storage used by items defined by this class.
Item::Type ComponentItemDefinition::type() const
{
  return Item::ComponentType;
}

bool ComponentItemDefinition::isValueValid(smtk::resource::ComponentPtr comp) const
{
  // If the component is invalid, then no filtering is needed.
  if (!comp)
  {
    return false;
  }

  // All components are required to have resources in order to be valid.
  auto rsrc = comp->resource();
  if (!rsrc)
  {
    return false;
  }

  // If there are no filter values, then we accept all components.
  if (m_acceptable.empty())
  {
    return true;
  }

  // Search for the resource index in the resource metdata.
  const smtk::resource::Metadata* metadata = nullptr;

  auto manager = rsrc->manager();
  if (manager)
  {
    auto& container = manager->metadata().get<smtk::resource::IndexTag>();
    auto metadataIt = container.find(rsrc->index());
    if (metadataIt != container.end())
    {
      metadata = &(*metadataIt);
    }
  }

  if (metadata == nullptr)
  {
    // If we can't find the resource's metadata, that's ok. It just means we do
    // not have the ability to accept derived resources from base resource
    // indices. We can still check if the resource is explicitly accepted.
    auto range = m_acceptable.equal_range(rsrc->typeName());
    for (auto& it = range.first; it != range.second; ++it)
    {
      if (rsrc->queryOperation(it->second)(comp))
      {
        return true;
      }
    }
    return false;
  }

  // With the resource's metadata, we can resolve queries for derived resources.
  auto& container = manager->metadata().get<smtk::resource::NameTag>();

  // For every element in the filter map...
  for (auto& acceptable : m_acceptable)
  {
    // ...we access the metadata for that resource type...
    auto md = container.find(acceptable.first);
    // ...and ask (a) if our resource is of that type, and (b) if its associated
    // filter accepts the component.
    if (md != container.end() && metadata->isOfType(md->index()) &&
      rsrc->queryOperation(acceptable.second)(comp))
    {
      return true;
    }
  }

  return false;
}

smtk::attribute::ItemPtr ComponentItemDefinition::buildItem(
  Attribute* owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(new ComponentItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr ComponentItemDefinition::buildItem(
  Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new ComponentItem(owningItem, itemPosition, subGroupPosition));
}

smtk::attribute::ItemDefinitionPtr ComponentItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;
  auto copy = ComponentItemDefinition::New(this->name());
  this->copyTo(copy);
  return copy;
}
