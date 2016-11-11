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
#include "smtk/attribute/DateTimeItem.h"
using namespace smtk::attribute;

//----------------------------------------------------------------------------
DateTimeItemDefinition::DateTimeItemDefinition(const std::string &myName):
  ValueItemDefinitionTemplate<DateTimeZonePair>(myName),
  m_useTimeZone(true), m_useCalendarPopup(true)
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
  return smtk::attribute::ItemPtr(
    new DateTimeItem(owningAttribute, itemPosition));
}

//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
DateTimeItemDefinition::buildItem(Item *owningItem,
                                int itemPosition,
                                int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(
    new DateTimeItem(owningItem, itemPosition, subGroupPosition));
}

//----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr
smtk::attribute::DateTimeItemDefinition::
createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  smtk::attribute::DateTimeItemDefinitionPtr newDef =
    smtk::attribute::DateTimeItemDefinition::New(this->name());

  ValueItemDefinitionTemplate<DateTimeZonePair>::copyTo(newDef, info);
  return newDef;
}
//----------------------------------------------------------------------------
