//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/IntItem.h"
using namespace smtk::attribute;

IntItemDefinition::IntItemDefinition(const std::string& myName)
  : ValueItemDefinitionTemplate<int>(myName)
{
}

IntItemDefinition::~IntItemDefinition()
{
}

Item::Type IntItemDefinition::type() const
{
  return Item::INT;
}

smtk::attribute::ItemPtr IntItemDefinition::buildItem(
  Attribute* owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(new IntItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr IntItemDefinition::buildItem(
  Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new IntItem(owningItem, itemPosition, subGroupPosition));
}

smtk::attribute::ItemDefinitionPtr smtk::attribute::IntItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  smtk::attribute::IntItemDefinitionPtr newDef =
    smtk::attribute::IntItemDefinition::New(this->name());

  ValueItemDefinitionTemplate<int>::copyTo(newDef, info);
  return newDef;
}
