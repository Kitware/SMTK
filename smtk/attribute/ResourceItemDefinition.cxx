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
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Container.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

using namespace smtk::attribute;

/// Construct an item definition given a name. Names should be unique and non-empty.
ResourceItemDefinition::ResourceItemDefinition(const std::string& sname)
  : Superclass(sname)
{
  setOnlyResources(true);
}

/// Destructor.
ResourceItemDefinition::~ResourceItemDefinition() = default;

/// Return the type of storage used by items defined by this class.
Item::Type ResourceItemDefinition::type() const
{
  return Item::ResourceType;
}

bool ResourceItemDefinition::isValueValid(smtk::resource::ConstPersistentObjectPtr obj) const
{
  auto rsrc = std::dynamic_pointer_cast<const smtk::resource::Resource>(obj);
  return this->checkResource(rsrc);
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
