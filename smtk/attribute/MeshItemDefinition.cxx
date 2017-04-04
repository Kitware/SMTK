//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/MeshItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/MeshItem.h"

using namespace smtk::attribute;

MeshItemDefinition::
MeshItemDefinition(const std::string &myName):
  ItemDefinition(myName)
{
  this->m_numberOfRequiredValues = 0;
  this->m_isExtensible = false;
  this->m_maxNumberOfValues = 0;
}

MeshItemDefinition::~MeshItemDefinition()
{
}

Item::Type MeshItemDefinition::type() const
{
  return Item::MESH_ENTITY;
}
/// Return the number of values (mesh entities) required by this definition.
std::size_t MeshItemDefinition::numberOfRequiredValues() const
{
  return this->m_numberOfRequiredValues;
}

/// Set the number of values (model entities) required by this definition. Use 0 when there is no requirement.
void MeshItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  if (esize == this->m_numberOfRequiredValues)
    {
    return;
    }
  this->m_numberOfRequiredValues = esize;
}

/// Set the maximum number of values accepted (or 0 for no limit).
void MeshItemDefinition::setMaxNumberOfValues(std::size_t maxNum)
{
  this->m_maxNumberOfValues = maxNum;
}

bool MeshItemDefinition::isValueValid(const smtk::mesh::MeshSet &val) const
{
  return !val.is_empty(); // should we allow empty meshset?
}

smtk::attribute::ItemPtr MeshItemDefinition::buildItem(
  Attribute *owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(new MeshItem(owningAttribute,
                                              itemPosition));
}

smtk::attribute::ItemPtr MeshItemDefinition::buildItem(
Item *owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new MeshItem(owningItem,
                                              itemPosition,
                                              subGroupPosition));
}

smtk::attribute::ItemDefinitionPtr
smtk::attribute::MeshItemDefinition::
createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;

  smtk::attribute::MeshItemDefinitionPtr newDef =
    smtk::attribute::MeshItemDefinition::New(this->name());
  ItemDefinition::copyTo(newDef);
  newDef->setNumberOfRequiredValues(this->numberOfRequiredValues());
  newDef->setIsExtensible(m_isExtensible);
  newDef->setMaxNumberOfValues(m_maxNumberOfValues);
  return newDef;
}
