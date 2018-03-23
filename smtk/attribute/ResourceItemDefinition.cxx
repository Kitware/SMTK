//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/ResourceItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Container.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/attribute/ReferenceItem.txx"
#include "smtk/attribute/ReferenceItemDefinition.txx"

using namespace smtk::attribute;

/// Construct an item definition given a name. Names should be unique and non-empty.
ResourceItemDefinition::ResourceItemDefinition(const std::string& sname)
  : Superclass(sname)
{
}

/// Destructor.
ResourceItemDefinition::~ResourceItemDefinition()
{
}

/// Return the type of storage used by items defined by this class.
Item::Type ResourceItemDefinition::type() const
{
  return Item::ResourceType;
}

bool ResourceItemDefinition::isValueValid(smtk::resource::ResourcePtr rsrc) const
{
  // If the resource is invalid, then no filtering is needed.
  if (!rsrc)
  {
    return false;
  }

  // If there are no filter values, then we accept all resources.
  if (m_acceptable.empty())
  {
    return true;
  }

  // TODO:
  // Queries to filter resources as acceptable have not been implemented.
  // See smtk::attribute::ComponentItemDefinition::isValueValid() for
  // a pattern to follow when implementing this.
  //
  // For now all we do is test the resource names in m_acceptable
  // to see if any are exact matches for rsrc.

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

  if (!metadata)
  {
    // If we don't have a resource manager, see if the resource type is
    // listed explicitly. If so, presume that's OK.
    return m_acceptable.find(rsrc->typeName()) != m_acceptable.end();
  }

  auto& container = manager->metadata().get<smtk::resource::NameTag>();

  // For every element in the filter map...
  for (auto& acceptable : m_acceptable)
  {
    // ...we access the metadata for that resource type...
    auto md = container.find(acceptable.first);
    // ...and ask (a) if our resource is of that type, and (b) if its associated
    // filter accepts the component.
    if (md != container.end() && metadata->isOfType(md->index()))
    {
      return true;
    }
  }

  return false;
}

smtk::attribute::ItemPtr ResourceItemDefinition::buildItem(
  Attribute* owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(new ResourceItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr ResourceItemDefinition::buildItem(
  Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new ResourceItem(owningItem, itemPosition, subGroupPosition));
}

smtk::attribute::ItemDefinitionPtr ResourceItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;
  auto copy = ResourceItemDefinition::New(this->name());
  this->copyTo(copy);
  return copy;
}
