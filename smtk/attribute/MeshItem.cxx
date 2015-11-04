//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include <iostream>
#include <stdio.h>
#include <algorithm>    // std::set_difference

using namespace smtk::attribute;

//----------------------------------------------------------------------------
MeshItem::MeshItem(Attribute* owningAttribute,
                   int itemPosition):
  Item(owningAttribute, itemPosition)
{
}

//----------------------------------------------------------------------------
MeshItem::MeshItem(Item* inOwningItem,
                   int itemPosition,
                   int inSubGroupPosition):
  Item(inOwningItem, itemPosition, inSubGroupPosition)
{
}

//----------------------------------------------------------------------------
bool MeshItem::
setDefinition(smtk::attribute::ConstItemDefinitionPtr adef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const MeshItemDefinition* def =
    dynamic_cast<const MeshItemDefinition*>(adef.get());

  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!Item::setDefinition(adef)))
    {
    return false;
    }
  this->m_meshValues.clear();
  return true;
}

//----------------------------------------------------------------------------
MeshItem::~MeshItem()
{
}
//----------------------------------------------------------------------------
Item::Type MeshItem::type() const
{
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition*>(this->definition().get());
  if (def != NULL)
    {
    return def->type();
    }
  return Item::MESH_ENTITY;
}
//----------------------------------------------------------------------------
std::size_t MeshItem::numberOfValues() const
{
  return this->m_meshValues.size();
}
//----------------------------------------------------------------------------
/// Return the number of values required by this item's definition (if it has one).
std::size_t MeshItem::numberOfRequiredValues() const
{
  const MeshItemDefinition *def =
    static_cast<const MeshItemDefinition*>(this->m_definition.get());
  if (def == NULL)
    {
    return 0;
    }
  return def->numberOfRequiredValues();
}
// A convenience method returning whether the item's definition is extensible.
bool MeshItem::isExtensible() const
{
  smtk::attribute::ConstMeshItemDefinitionPtr def =
    smtk::dynamic_pointer_cast<const MeshItemDefinition>(
      this->definition());
  if (!def)
    return false;
  return def->isExtensible();
}

//----------------------------------------------------------------------------
bool MeshItem::setValue(const smtk::common::UUID& uuid,
                         const smtk::mesh::MeshSet& vals)
{
  smtk::attribute::MeshItem::const_mesh_it it =
    this->m_meshValues.find(uuid);
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  if (!def->isExtensible() && it == this->m_meshValues.end() &&
      this->m_meshValues.size() >= def->numberOfRequiredValues())
    {
    // The maximum number of values is fixed
    return false;
    }

  this->m_meshValues[uuid] = vals;
  return true;
}

//----------------------------------------------------------------------------
bool MeshItem::appendValue(const smtk::common::UUID& uuid,
                           const smtk::mesh::MeshSet& vals)
{
  smtk::attribute::MeshItem::const_mesh_it it =
    this->m_meshValues.find(uuid);
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  if (!def->isExtensible() && it == this->m_meshValues.end() &&
      this->m_meshValues.size() >= def->numberOfRequiredValues())
    {
    // The maximum number of values is fixed
    return false;
    }

  this->m_meshValues[uuid] = smtk::mesh::set_union(
    this->m_meshValues[uuid], vals);
  return true;
}

//----------------------------------------------------------------------------
void MeshItem::removeValue(const smtk::common::UUID& uuid,
                            const smtk::mesh::MeshSet& vals)
{
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  smtk::attribute::MeshItem::mesh_it it =
    this->m_meshValues.find(uuid);
  if(it != this->m_meshValues.end())
    {
    if(!vals.is_empty()) // remove the vals from collection map
      {
      this->m_meshValues[uuid] = smtk::mesh::set_difference(
      this->m_meshValues[uuid], vals);
      }
    else if(def->isExtensible())
      {
      this->m_meshValues.erase(it);
      }
    }
}

//----------------------------------------------------------------------------
smtk::mesh::MeshSet MeshItem::value(
  const smtk::common::UUID& uuid) const
{
  smtk::attribute::MeshItem::const_mesh_it it =
    this->m_meshValues.find(uuid);
  if(it != this->m_meshValues.end())
    {
    return it->second;
    }
  return smtk::mesh::MeshSet();
}

//----------------------------------------------------------------------------
void MeshItem::reset()
{
  this->m_meshValues.clear();
}
//----------------------------------------------------------------------------
void MeshItem::copyFrom(ItemPtr sourceItem, CopyInfo& info)
{
  // Assigns the contents to be same as sourceItem
  // Assuming they have the same number of required values
  Item::copyFrom(sourceItem, info);
  this->reset();
  MeshItemPtr sourceMeshItem =
    smtk::dynamic_pointer_cast<MeshItem>(sourceItem);
  this->m_meshValues.insert(sourceMeshItem->begin(),
                            sourceMeshItem->end());
}
//----------------------------------------------------------------------------
smtk::attribute::MeshItem::const_mesh_it MeshItem::begin() const
{
  return this->m_meshValues.begin();
}

smtk::attribute::MeshItem::const_mesh_it MeshItem::end() const
{
  return this->m_meshValues.end();
}
