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
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Container.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/common/UUID.h"

#include <cassert>

using namespace smtk::attribute;

/// Construct an item definition given a name. Names should be unique and non-empty.
ComponentItemDefinition::ComponentItemDefinition(const std::string& sname)
  : Superclass(sname)
{
  setOnlyResources(false);
}

/// Destructor.
ComponentItemDefinition::~ComponentItemDefinition() = default;

/// Return the type of storage used by items defined by this class.
Item::Type ComponentItemDefinition::type() const
{
  return Item::ComponentType;
}

bool ComponentItemDefinition::isValueValid(smtk::resource::ConstPersistentObjectPtr obj) const
{
  auto comp = dynamic_cast<const smtk::resource::Component*>(obj.get());
  if (comp == nullptr)
  {
    return false;
  }
  return this->checkComponent(comp);
}

smtk::attribute::ItemPtr ComponentItemDefinition::buildItem(
  Attribute* owningAttribute,
  int itemPosition) const
{
  return smtk::attribute::ItemPtr(new ComponentItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr
ComponentItemDefinition::buildItem(Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new ComponentItem(owningItem, itemPosition, subGroupPosition));
}

smtk::attribute::ItemDefinitionPtr ComponentItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;
  auto copy = ComponentItemDefinition::New(this->name());
  this->copyTo(copy, info);
  return copy;
}
