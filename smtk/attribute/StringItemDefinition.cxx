//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/StringItem.h"
using namespace smtk::attribute;

StringItemDefinition::StringItemDefinition(const std::string& myName)
  : ValueItemDefinitionTemplate<std::string>(myName)
{
}

StringItemDefinition::~StringItemDefinition() = default;

Item::Type StringItemDefinition::type() const
{
  return Item::StringType;
}

smtk::attribute::ItemPtr StringItemDefinition::buildItem(
  Attribute* owningAttribute,
  int itemPosition) const
{
  return smtk::attribute::ItemPtr(new StringItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr
StringItemDefinition::buildItem(Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new StringItem(owningItem, itemPosition, subGroupPosition));
}

smtk::attribute::ItemDefinitionPtr smtk::attribute::StringItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  smtk::attribute::StringItemDefinitionPtr newDef =
    smtk::attribute::StringItemDefinition::New(this->name());

  ValueItemDefinitionTemplate<std::string>::copyTo(newDef, info);
  return newDef;
}
