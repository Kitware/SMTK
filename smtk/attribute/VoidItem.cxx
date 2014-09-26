//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/VoidItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include <iostream>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
VoidItem::VoidItem(Attribute *owningAttribute,
                   int itemPosition):
  Item(owningAttribute, itemPosition)
{
}

//----------------------------------------------------------------------------
VoidItem::VoidItem(Item *inOwningItem,
                   int itemPosition,
                   int mySubGroupPosition):
  Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

//----------------------------------------------------------------------------
bool VoidItem::
setDefinition(smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const VoidItemDefinition *def =
    dynamic_cast<const VoidItemDefinition *>(adef.get());

  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Item::setDefinition(adef)))
    {
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
VoidItem::~VoidItem()
{
}
//----------------------------------------------------------------------------
Item::Type VoidItem::type() const
{
  return VOID;
}

//----------------------------------------------------------------------------
