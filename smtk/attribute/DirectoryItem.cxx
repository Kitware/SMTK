//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include <iostream>
#include <stdio.h>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
DirectoryItem::DirectoryItem(Attribute *owningAttribute,
                             int itemPosition):
  FileSystemItem(owningAttribute, itemPosition)
{
}

//----------------------------------------------------------------------------
DirectoryItem::DirectoryItem(Item *inOwningItem,
                             int itemPosition,
                             int inSubGroupPosition):
  FileSystemItem(inOwningItem, itemPosition, inSubGroupPosition)
{
}

//----------------------------------------------------------------------------
DirectoryItem::~DirectoryItem()
{
}

//----------------------------------------------------------------------------
Item::Type DirectoryItem::type() const
{
  return DIRECTORY;
}
