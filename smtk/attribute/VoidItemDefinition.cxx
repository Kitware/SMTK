//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/VoidItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/VoidItem.h"

using namespace smtk::attribute;

VoidItemDefinition::
VoidItemDefinition(const std::string &myName):
  ItemDefinition(myName)
{
}

VoidItemDefinition::~VoidItemDefinition()
{
}

smtk::attribute::ItemPtr
VoidItemDefinition::buildItem(Attribute *owningAttribute,
                              int itemPosition) const
{
  return smtk::attribute::ItemPtr(new VoidItem(owningAttribute,
                                              itemPosition));
}

smtk::attribute::ItemPtr
VoidItemDefinition::buildItem(Item *owningItem,
                              int itemPosition,
                              int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new VoidItem(owningItem,
                                              itemPosition,
                                              subGroupPosition));
}

Item::Type VoidItemDefinition::type() const
{
  return Item::VOID;
}

smtk::attribute::ItemDefinitionPtr
smtk::attribute::VoidItemDefinition::
createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;

  smtk::attribute::VoidItemDefinitionPtr instance =
    smtk::attribute::VoidItemDefinition::New(this->name());
  ItemDefinition::copyTo(instance);
  return instance;
}
