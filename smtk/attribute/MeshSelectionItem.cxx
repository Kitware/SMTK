//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include <iostream>
#include <stdio.h>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
MeshSelectionItem::MeshSelectionItem(Attribute *owningAttribute,
                   int itemPosition):
  Item(owningAttribute, itemPosition)
{
  m_selectMode = NONE;
  m_isCtrlKeyDown = false;
}

//----------------------------------------------------------------------------
MeshSelectionItem::MeshSelectionItem(Item *inOwningItem,
                   int itemPosition,
                   int inSubGroupPosition):
  Item(inOwningItem, itemPosition, inSubGroupPosition)
{
}

//----------------------------------------------------------------------------
bool MeshSelectionItem::
setDefinition(smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const MeshSelectionItemDefinition *def =
    dynamic_cast<const MeshSelectionItemDefinition *>(adef.get());

  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Item::setDefinition(adef)))
    {
    return false;
    }
  this->m_selectionValues.clear();
  return true;
}

//----------------------------------------------------------------------------
MeshSelectionItem::~MeshSelectionItem()
{
}
//----------------------------------------------------------------------------
Item::Type MeshSelectionItem::type() const
{
  const MeshSelectionItemDefinition *def =
    static_cast<const MeshSelectionItemDefinition *>(this->definition().get());
  if (def != NULL)
    {
    return def->type();
    }
  return Item::MESH_SELECTION;
}

//----------------------------------------------------------------------------
void MeshSelectionItem::setValues(const smtk::common::UUID& uuid,
                                  const std::vector<int>& vals)
{
  this->m_selectionValues[uuid] = vals;
}

//----------------------------------------------------------------------------
void MeshSelectionItem::appendValues(const smtk::common::UUID& uuid,
                                     const std::vector<int>& vals)
{
  this->m_selectionValues[uuid].insert(
    this->m_selectionValues[uuid].end(), vals.begin(), vals.end());
}

//----------------------------------------------------------------------------
void MeshSelectionItem::removeValues(const smtk::common::UUID& uuid,
                                     const std::vector<int>& vals)
{
  for(std::vector<int>::const_iterator it=vals.begin(); it!= vals.end(); ++it)
    this->m_selectionValues[uuid].erase(std::remove(
      this->m_selectionValues[uuid].begin(),
      this->m_selectionValues[uuid].end(), *it),
      this->m_selectionValues[uuid].end());

}

//----------------------------------------------------------------------------
const std::vector<int>& MeshSelectionItem::values(
  const smtk::common::UUID& uuid)
{
  //std::advance(it,element);
  return this->m_selectionValues[uuid];
}

//----------------------------------------------------------------------------
void MeshSelectionItem::reset()
{
  this->m_selectionValues.clear();
}
//----------------------------------------------------------------------------
void MeshSelectionItem::copyFrom(ItemPtr sourceItem, CopyInfo& info)
{
  // Assigns my contents to be same as sourceItem
  Item::copyFrom(sourceItem, info);

  MeshSelectionItemPtr sourceMeshSelectionItem =
    smtk::dynamic_pointer_cast<MeshSelectionItem>(sourceItem);
  this->m_selectMode = sourceMeshSelectionItem->meshSelectMode();
  this->m_isCtrlKeyDown = sourceMeshSelectionItem->isCtrlKeyDown();
  this->m_selectionValues = sourceMeshSelectionItem->m_selectionValues;
}
//----------------------------------------------------------------------------
smtk::attribute::MeshSelectionItem::const_sel_map_it MeshSelectionItem::begin() const
{
  return this->m_selectionValues.begin();
}

smtk::attribute::MeshSelectionItem::const_sel_map_it MeshSelectionItem::end() const
{
  return this->m_selectionValues.end();
}
