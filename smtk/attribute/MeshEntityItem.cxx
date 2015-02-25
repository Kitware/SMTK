//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/MeshEntityItem.h"
#include "smtk/attribute/MeshEntityItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include <iostream>
#include <stdio.h>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
MeshEntityItem::MeshEntityItem(Attribute *owningAttribute,
                   int itemPosition):
  Item(owningAttribute, itemPosition)
{
  m_selectMode = NONE;
}

//----------------------------------------------------------------------------
MeshEntityItem::MeshEntityItem(Item *inOwningItem,
                   int itemPosition,
                   int inSubGroupPosition):
  Item(inOwningItem, itemPosition, inSubGroupPosition)
{
}

//----------------------------------------------------------------------------
bool MeshEntityItem::
setDefinition(smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const MeshEntityItemDefinition *def =
    dynamic_cast<const MeshEntityItemDefinition *>(adef.get());

  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Item::setDefinition(adef)))
    {
    return false;
    }
  this->m_values.clear();
  return true;
}

//----------------------------------------------------------------------------
MeshEntityItem::~MeshEntityItem()
{
}
//----------------------------------------------------------------------------
Item::Type MeshEntityItem::type() const
{
  const MeshEntityItemDefinition *def =
    static_cast<const MeshEntityItemDefinition *>(this->definition().get());
  if (def != NULL)
    {
    return def->type();
    }
  return Item::MESH_ENTITY;
}

//----------------------------------------------------------------------------
void MeshEntityItem::setValues(const std::set<int>& vals)
{
  this->reset();
  this->m_values.insert(vals.begin(), vals.end());
}

//----------------------------------------------------------------------------
void MeshEntityItem::insertValues(const std::set<int>& vals)
{
  this->m_values.insert(vals.begin(), vals.end());
}

//----------------------------------------------------------------------------
void MeshEntityItem::removeValues(const std::set<int>& vals)
{
  for(std::set<int>::const_iterator it=vals.begin(); it!= vals.end(); ++it)
    this->m_values.erase(*it);
}

//----------------------------------------------------------------------------
int MeshEntityItem::value(std::size_t element) const
{
  std::set<int>::iterator it = this->m_values.begin();
  std::advance(it,element);
  return *it;
}

//----------------------------------------------------------------------------
std::string MeshEntityItem::valueAsString(std::size_t element) const
{
  std::set<int>::iterator it = this->m_values.begin();
  std::advance(it,element);
  std::stringstream buffer;
  buffer << *it;
  return buffer.str();
}
//----------------------------------------------------------------------------
bool MeshEntityItem::insertValue(const int &val)
{
  //First - are we allowed to change the number of values?
  const MeshEntityItemDefinition *def =
    static_cast<const MeshEntityItemDefinition *>(this->definition().get());
  if (def->isValueValid(val))
    {
    this->m_values.insert(val);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool MeshEntityItem::removeValue(const int &val)
{
  this->m_values.erase(val);
  return true;
}

//----------------------------------------------------------------------------
void MeshEntityItem::reset()
{
  this->m_values.clear();
}
//----------------------------------------------------------------------------
void MeshEntityItem::copyFrom(ItemPtr sourceItem, CopyInfo& info)
{
  // Assigns my contents to be same as sourceItem
  Item::copyFrom(sourceItem, info);

  MeshEntityItemPtr sourceMeshEntityItem =
    smtk::dynamic_pointer_cast<MeshEntityItem>(sourceItem);

  this->m_values.clear();
  this->m_values.insert(sourceMeshEntityItem->begin(), sourceMeshEntityItem->end());
}
//----------------------------------------------------------------------------
std::set<int>::const_iterator MeshEntityItem::begin() const
{
  return this->m_values.begin();
}

std::set<int>::const_iterator MeshEntityItem::end() const
{
  return this->m_values.end();
}
