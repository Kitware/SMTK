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
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/MeshItemDefinition.h"
#include <algorithm>    // for std::find and std::copy
#include <iostream>
#include <stdio.h>

using namespace smtk::attribute;

MeshItem::MeshItem(Attribute* owningAttribute,
                   int itemPosition):
  Item(owningAttribute, itemPosition)
{
}

MeshItem::MeshItem(Item* inOwningItem,
                   int itemPosition,
                   int inSubGroupPosition):
  Item(inOwningItem, itemPosition, inSubGroupPosition)
{
}

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
  std::size_t n = def->numberOfRequiredValues();
  if (n != 0)
    {
    this->m_meshValues.resize(n);
    }
  return true;
}

MeshItem::~MeshItem()
{
}

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

std::size_t MeshItem::numberOfValues() const
{
  return this->m_meshValues.size();
}

// Set the number of entities to be associated with this item (returns true if permitted).
bool MeshItem::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
    {
    return true;
    }

  // Next - are we allowed to change the number of values?
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  if (!def->isExtensible())
    return false; // You may not resize.

  // Next - are we within the prescribed limits?
  std::size_t n = def->numberOfRequiredValues();
  if (newSize < n)
    return false; // The number of values requested is too small.

  n = def->maxNumberOfValues();
  if (n > 0 && newSize > n)
    return false; // The number of values requested is too large.

  this->m_meshValues.resize(newSize);
  return true;
}

bool MeshItem::isValid() const
{
  // If the item is not enabled or if it contains atleast the number of 
  // required values
  if (!this->isEnabled())
    {
    return true;
    }
  // Do we have atleats the number of required values present?
  if(this->numberOfValues() < this->numberOfRequiredValues())
    {
    return false;
    }
  for (auto it = this->m_meshValues.begin(); it != this->m_meshValues.end(); ++it)
    {
    // If the mesh is empty
    if ((*it).is_empty())
      {
      return false;
      }
    }
  return true;
 }

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

/// Return the \a i-th meshset stored in this item.
smtk::mesh::MeshSet MeshItem::value(std::size_t i) const
{
  if (i >= static_cast<std::size_t>(this->m_meshValues.size()))
    return smtk::mesh::MeshSet();
  return this->m_meshValues[i];
}

bool MeshItem::setValue(const smtk::mesh::MeshSet& val)
{
  return this->setValue(0, val);
}

bool MeshItem::isSet(std::size_t i) const
{
  return i < this->m_meshValues.size() ?
    !this->m_meshValues[i].is_empty() :
    false;
}

/// Force the \a i-th value of the item to be invalid.
void MeshItem::unset(std::size_t i)
{
  if (i<this->m_meshValues.size())
    {
    this->m_meshValues[i] = smtk::mesh::MeshSet();
    }
}

/// Set the \a i-th value to the given item. This method does no checking to see if \a i is valid.
bool MeshItem::setValue(std::size_t i, const smtk::mesh::MeshSet& val)
{
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  if (i<this->m_meshValues.size() && def->isValueValid(val))
    {
    this->m_meshValues[i] = val;
    return true;
    }
  return false;
}

bool MeshItem::appendValue(const smtk::mesh::MeshSet& val)
{
  // First - are there unset values waiting to be set?
  std::size_t n = this->numberOfValues();
  for (std::size_t i = 0; i < n; ++i)
    {
    if (!this->isSet(i))
      {
      return this->setValue(i, val);
      }
    }
  // Second - are we allowed to change the number of values?
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  if (
    (def->isExtensible() && def->maxNumberOfValues() &&
     this->m_meshValues.size() >= def->maxNumberOfValues()) ||
    (!def->isExtensible() && this->m_meshValues.size() >= def->numberOfRequiredValues()))
    {
    // The maximum number of values is fixed
    return false;
    }

  if (def->isValueValid(val))
    {
    this->m_meshValues.push_back(val);
    return true;
    }
  return false;
}

bool MeshItem::appendValues(const smtk::mesh::MeshList& vals)
{
  smtk::mesh::MeshSets meshes;
  meshes.insert(vals.begin(), vals.end());
  return this->appendValues(meshes);
}

bool MeshItem::appendValues(const smtk::mesh::MeshSets& vals)
{
  bool valAppended = false;
  for(smtk::mesh::MeshSets::const_iterator it = vals.begin();
      it != vals.end(); ++it)
    {
    if(this->appendValue(*it))
      {
      valAppended = true;
      }
    }
  // return true if any of the values is appended
  return valAppended;
}

bool MeshItem::removeValue(std::size_t i)
{
  const MeshItemDefinition* def =
    static_cast<const MeshItemDefinition *>(this->definition().get());
  if(!def->isExtensible())
    return false;

  if(i < this->m_meshValues.size())
    {
    this->m_meshValues.erase(this->m_meshValues.begin()+i);
    return true;
    }
  return false;
}

bool MeshItem::hasValue(const smtk::mesh::MeshSet& val) const
{
  return std::find(this->m_meshValues.begin(), this->m_meshValues.end(), val)
    != this->m_meshValues.end();
}

const smtk::mesh::MeshList& MeshItem::values() const
{
  return this->m_meshValues;
}

void MeshItem::reset()
{
  this->m_meshValues.clear();
  if (this->numberOfRequiredValues() > 0)
    this->m_meshValues.resize(this->numberOfRequiredValues());
}

/// Assigns contents to be same as source item
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
  // Update values
  // Only set values if both att systems are using the same model
  this->setNumberOfValues(sourceMeshItem->numberOfValues());
  for (std::size_t i=0; i<sourceMeshItem->numberOfValues(); ++i)
    {
    if (sourceMeshItem->isSet(i))
      {
      smtk::mesh::MeshSet val = sourceMeshItem->value(i);
      this->setValue(i, val);
      }
    else
      {
      this->unset(i);
      }
    }
  return Item::assign(sourceItem, options);
}

std::ptrdiff_t MeshItem::find(const smtk::mesh::MeshSet& mesh) const
{
  std::ptrdiff_t idx = 0;
  smtk::mesh::MeshList::const_iterator it;
  for (it = this->begin(); it != this->end(); ++it, ++idx)
    {
    if ((*it) == mesh)
      {
      return idx;
      }
    }
  return -1;
}

smtk::attribute::MeshItem::const_mesh_it MeshItem::begin() const
{
  return this->m_meshValues.begin();
}

smtk::attribute::MeshItem::const_mesh_it MeshItem::end() const
{
  return this->m_meshValues.end();
}
