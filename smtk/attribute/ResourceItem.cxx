//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"

#include "smtk/attribute/ReferenceItem.txx"

using namespace smtk::attribute;

ResourceItem::ResourceItem(Attribute* owningAttribute, int itemPosition)
  : Superclass(owningAttribute, itemPosition)
{
}

ResourceItem::ResourceItem(Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : Superclass(inOwningItem, itemPosition, mySubGroupPosition)
{
}

ResourceItem::~ResourceItem()
{
}

Item::Type ResourceItem::type() const
{
  return ResourceType;
}

std::string ResourceItem::valueAsString(std::size_t i) const
{
  std::ostringstream str;
  auto val = this->value(i);
  return val ? val->id().toString() : "";
}
