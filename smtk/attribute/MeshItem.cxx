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
  this->m_meshValues.insert(val);
  return true;
}

//----------------------------------------------------------------------------
bool MeshItem::appendValue(const smtk::mesh::MeshSet& val)
{
  smtk::mesh::MeshSets meshes;
  meshes.insert(val);
  return this->appendValues(meshes);
}

//----------------------------------------------------------------------------
bool MeshItem::appendValues(const smtk::mesh::MeshList& vals)
{
  smtk::mesh::MeshSets meshes;
  meshes.insert(vals.begin(), vals.end());
  return this->appendValues(meshes);
}

//----------------------------------------------------------------------------
bool MeshItem::appendValues(const smtk::mesh::MeshSets& vals)
{
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  if (!def->isExtensible() &&
      this->m_meshValues.size() >= def->numberOfRequiredValues())
    {
    // The maximum number of values is fixed
    return false;
    }

  this->m_meshValues.insert(vals.begin(), vals.end());

  return true;
}

//----------------------------------------------------------------------------
void MeshItem::removeValue(const smtk::mesh::MeshSet& val)
{
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  if(!def->isExtensible())
    return;

  if(this->m_meshValues.find(val) != this->m_meshValues.end())
    {
    this->m_meshValues.erase(val);
    }
}

//----------------------------------------------------------------------------
bool MeshItem::hasValue(const smtk::mesh::MeshSet& val) const
{
  return this->m_meshValues.find(val) != this->m_meshValues.end();
}

//----------------------------------------------------------------------------
const smtk::mesh::MeshSets& MeshItem::values() const
{
  return this->m_meshValues;
}

//----------------------------------------------------------------------------
void MeshItem::reset()
{
  this->m_meshValues.clear();
}

/// Assigns contents to be same as source item
//----------------------------------------------------------------------------
bool MeshItem::assign(ConstItemPtr &sourceItem, unsigned int options)
{
  // Cast input pointer to ModelEntityItem
  smtk::shared_ptr<const MeshItem > sourceMeshItem =
    smtk::dynamic_pointer_cast<const MeshItem>(sourceItem);

  if (!sourceMeshItem)
    {
    return false; // Source is not a mesh item
    }

  // Update values
  this->reset();
  this->m_meshValues.insert(sourceMeshItem->begin(),sourceMeshItem->end());
  return Item::assign(sourceItem, options);
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
