//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
IntItem::IntItem(Attribute *owningAttribute,
                 int itemPosition):
  ValueItemTemplate<int>(owningAttribute, itemPosition)
{
}

//----------------------------------------------------------------------------
IntItem::IntItem(Item *inOwningItem,
                 int itemPosition,
                 int mySubGroupPosition):
  ValueItemTemplate<int>(inOwningItem, itemPosition, mySubGroupPosition)
{
}
//----------------------------------------------------------------------------
IntItem::~IntItem()
{
}
//----------------------------------------------------------------------------
Item::Type IntItem::type() const
{
  return INT;
}
//----------------------------------------------------------------------------
bool IntItem::assign(ConstItemPtr &sourceItem, unsigned int options)
{
  // Assigns my contents to be same as sourceItem
  return ValueItemTemplate<int>::assign(sourceItem, options);
}
//----------------------------------------------------------------------------
