//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItemDefinition.h"
#include <algorithm> // for std::find
#include <cstdio>
#include <iostream>

using namespace smtk::attribute;

FileItem::FileItem(Attribute* owningAttribute, int itemPosition)
  : FileSystemItem(owningAttribute, itemPosition)
{
}

FileItem::FileItem(Item* inOwningItem, int itemPosition, int inSubGroupPosition)
  : FileSystemItem(inOwningItem, itemPosition, inSubGroupPosition)
{
}

bool FileItem::setDefinition(smtk::attribute::ConstItemDefinitionPtr adef)
{
  bool isSet = FileSystemItem::setDefinition(adef);

  if (isSet && this->numberOfRequiredValues())
  {
    m_recentValues.clear();
  }

  return isSet;
}

FileItem::~FileItem()
{
}

Item::Type FileItem::type() const
{
  return FileType;
}

void FileItem::addRecentValue(const std::string& val)
{
  if (std::find(m_recentValues.begin(), m_recentValues.end(), val) == m_recentValues.end())
    m_recentValues.push_back(val);
}
