//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/DateTimeItem.h"

namespace smtk {
  namespace attribute {


//----------------------------------------------------------------------------
DateTimeItem::DateTimeItem(Attribute *owningAttribute, int itemPosition):
  ValueItemTemplate<DateTimeZonePair>(owningAttribute, itemPosition)
{
}

//----------------------------------------------------------------------------
DateTimeItem::DateTimeItem(
  Item *inOwningAttribute, int itemPosition, int mySubGroupPosition):
  ValueItemTemplate<DateTimeZonePair>(
    inOwningAttribute, itemPosition, mySubGroupPosition)
{
}

//----------------------------------------------------------------------------
DateTimeItem::~DateTimeItem()
{
}

//----------------------------------------------------------------------------
Item::Type DateTimeItem::type() const
{
  return Item::DATE_TIME;
}

//----------------------------------------------------------------------------
bool DateTimeItem::assign(ConstItemPtr &sourceItem, unsigned int options)
{
  // Assigns my contents to be same as sourceItem
  return ValueItemTemplate<DateTimeZonePair>::assign(sourceItem, options);
}

//----------------------------------------------------------------------------

  }  // namespace attribute
}  // namespace smtk
