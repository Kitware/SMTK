//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

using namespace smtk::attribute;

StringItem::StringItem(Attribute* owningAttribute, int itemPosition)
  : ValueItemTemplate<std::string>(owningAttribute, itemPosition)
{
}

StringItem::StringItem(Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : ValueItemTemplate<std::string>(inOwningItem, itemPosition, mySubGroupPosition)
{
}

StringItem::~StringItem() = default;

Item::Type StringItem::type() const
{
  return StringType;
}

bool StringItem::assign(
  const smtk::attribute::ConstItemPtr& sourceItem,
  const CopyAssignmentOptions& options,
  smtk::io::Logger& logger)
{
  // Assigns my contents to be same as sourceItem
  return ValueItemTemplate<std::string>::assign(sourceItem, options, logger);
}

bool StringItem::isSecure() const
{
  const StringItemDefinition* sdef =
    static_cast<const StringItemDefinition*>(this->definition().get());
  if (!sdef)
  {
    return false;
  }
  return sdef->isSecure();
}
