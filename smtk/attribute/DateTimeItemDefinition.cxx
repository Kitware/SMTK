//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/DateTimeItemDefinition.h"
using namespace smtk::attribute;

//----------------------------------------------------------------------------
DateTimeItemDefinition::DateTimeItemDefinition(const std::string &myName):
  ValueItemDefinitionTemplate<DateTime>(myName), m_useTimeZone(true)
{
}

//----------------------------------------------------------------------------
DateTimeItemDefinition::~DateTimeItemDefinition()
{
}

//----------------------------------------------------------------------------
Item::Type DateTimeItemDefinition::type() const
{
  return Item::DATE_TIME;
}

//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
DateTimeItemDefinition::buildItem(Attribute *owningAttribute,
                                int itemPosition) const
{
  // return smtk::attribute::ItemPtr(
  //   new DateTimeItem(owningAttribute, itemPosition));
  return smtk::attribute::ItemPtr(0);  // todo
}

//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
DateTimeItemDefinition::buildItem(Item *owningItem,
                                int itemPosition,
                                int subGroupPosition) const
{
  // return smtk::attribute::ItemPtr(
  //   new DateTimeItem(owningItem, itemPosition, subGroupPosition));
  return smtk::attribute::ItemPtr(0);  // todo
}

//----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr
smtk::attribute::DateTimeItemDefinition::
createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  smtk::attribute::DateTimeItemDefinitionPtr newDef =
    smtk::attribute::DateTimeItemDefinition::New(this->name());

  ValueItemDefinitionTemplate<DateTime>::copyTo(newDef, info);
  return newDef;
}
//----------------------------------------------------------------------------
