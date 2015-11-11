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
#include <algorithm>    // for std::find and std::copy

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
bool MeshItem::setValue(const smtk::mesh::MeshSet& val)
{
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  if (!def->isExtensible() &&
      this->m_meshValues.size() >= def->numberOfRequiredValues())
    {
    // The maximum number of values is fixed
    return false;
    }

  this->m_meshValues.clear();
  this->m_meshValues.push_back(val);
  return true;
}

//----------------------------------------------------------------------------
bool MeshItem::appendValue(const smtk::mesh::MeshSet& val)
{
  smtk::mesh::MeshList meshes;
  meshes.push_back(val);
  return this->appendValues(meshes);
}

//----------------------------------------------------------------------------
bool MeshItem::appendValues(const smtk::mesh::MeshList& vals)
{
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  if (!def->isExtensible() &&
      this->m_meshValues.size() >= def->numberOfRequiredValues())
    {
    // The maximum number of values is fixed
    return false;
    }

  std::copy(vals.begin(),vals.end(),
    std::back_inserter(this->m_meshValues));

  return true;
}

//----------------------------------------------------------------------------
void MeshItem::removeValue(const smtk::mesh::MeshSet& val)
{
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  if(!def->isExtensible())
    return;
  smtk::attribute::MeshItem::mesh_it it = std::find(m_meshValues.begin(),
                                                    m_meshValues.end(), val);
  if(it != this->m_meshValues.end())
    {
    this->m_meshValues.erase(it);
    }
}

//----------------------------------------------------------------------------
const smtk::mesh::MeshList& MeshItem::values() const
{
  return this->m_meshValues;
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
  this->m_meshValues.clear();
  std::copy(sourceMeshItem->begin(),sourceMeshItem->end(),
    std::back_inserter(this->m_meshValues));
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
