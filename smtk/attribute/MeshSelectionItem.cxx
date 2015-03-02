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
#include <algorithm>    // std::set_difference

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
std::size_t MeshSelectionItem::numberOfValues() const
{
  std::size_t total = 0;
  smtk::attribute::MeshSelectionItem::const_sel_map_it it;
  for(it = this->begin(); it != this->end(); ++it)
    total += it->second.size();
  return total;
}

//----------------------------------------------------------------------------
void MeshSelectionItem::setValues(const smtk::common::UUID& uuid,
                                  const std::set<int>& vals)
{
  this->m_selectionValues[uuid] = vals;
}

//----------------------------------------------------------------------------
void MeshSelectionItem::unionValues(const smtk::common::UUID& uuid,
                                     const std::set<int>& vals)
{
  this->m_selectionValues[uuid].insert(vals.begin(), vals.end());
}

//----------------------------------------------------------------------------
void MeshSelectionItem::removeValues(const smtk::common::UUID& uuid,
                                     const std::set<int>& vals)
{
  std::set<int> diffSet;
  std::set_difference(this->m_selectionValues[uuid].begin(),
                      this->m_selectionValues[uuid].end(),
                      vals.begin(), vals.end(),
                      std::inserter(diffSet, diffSet.end()));
  this->m_selectionValues[uuid] = diffSet;
}

//----------------------------------------------------------------------------
const std::set<int>& MeshSelectionItem::values(
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

//----------------------------------------------------------------------------
std::string MeshSelectionItem::selectMode2String(
  MeshSelectionItem::MeshSelectionMode m)
{
  switch (m)
    {
    case NONE:
      return "NONE";
    case RESET:
      return "RESET";
    case MERGE:
      return "MERGE";
    case SUBTRACT:
      return "SUBTRACT";
    case ACCEPT:
      return "ACCEPT";
    default:
      return "";
    }
  return "Error!";
}

//----------------------------------------------------------------------------
MeshSelectionItem::MeshSelectionMode MeshSelectionItem::string2SelectMode(
  const std::string &s)
{
  if (s == "NONE")
    {
    return NONE;
    }
  if (s == "RESET")
    {
    return RESET;
    }
  if (s == "MERGE")
    {
    return MERGE;
    }
  if (s == "SUBTRACT")
    {
    return SUBTRACT;
    }
  if (s == "ACCEPT")
    {
    return ACCEPT;
    }
 return NUM_OF_MODES;
}