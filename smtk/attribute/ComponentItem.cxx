//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include <sstream>

using namespace smtk::attribute;

ComponentItem::ComponentItem(Attribute* owningAttribute, int itemPosition)
  : Superclass(owningAttribute, itemPosition)
{
}

ComponentItem::ComponentItem(Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : Superclass(inOwningItem, itemPosition, mySubGroupPosition)
{
}

ComponentItem::~ComponentItem() = default;

Item::Type ComponentItem::type() const
{
  return ComponentType;
}

bool ComponentItem::setValue(std::size_t ii, ComponentPtr value)
{
  if (this->isValueValid(ii, value))
  {
    return ReferenceItem::setValue(ii, value);
  }
  return false;
}

std::string ComponentItem::valueAsString(std::size_t i) const
{
  std::ostringstream str;
  auto val = this->value(i);
  auto rsrc = val ? val->resource() : smtk::resource::ResourcePtr();
  if (val && rsrc)
  {
    str << "[" << val->resource()->id() << "," << val->id() << "]";
  }
  else
  {
    str << "[]";
  }
  return str.str();
}
