//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"

#include "smtk/resource/Container.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/common/UUID.h"

#include <cassert>

using namespace smtk::attribute;

/// Construct an item definition given a name. Names should be unique and non-empty.
ResourceItemDefinition::ResourceItemDefinition(const std::string& sname)
  : ItemDefinition(sname)
{
  this->m_numberOfRequiredValues = 1;
  this->m_useCommonLabel = false;
  this->m_isExtensible = false;
  this->m_maxNumberOfValues = 0;
  this->m_isWritable = true;
}

/// Destructor.
ResourceItemDefinition::~ResourceItemDefinition()
{
}

/// Return the type of storage used by items defined by this class.
Item::Type ResourceItemDefinition::type() const
{
  return Item::ResourceType;
}

bool ResourceItemDefinition::acceptsResource(const smtk::resource::ResourcePtr& resource) const
{
  // If no resource types have been specified, we assume that all resource types
  // are acceptable.
  if (m_acceptable.empty())
  {
    return true;
  }

  // Search for the resource index in the resource metdata.
  const smtk::resource::Metadata* metadata = nullptr;

  auto manager = resource->manager();
  if (manager)
  {
    auto& container = manager->metadata().get<smtk::resource::IndexTag>();
    auto metadataIt = container.find(resource->index());
    if (metadataIt != container.end())
    {
      metadata = &(*metadataIt);
    }
  }

  if (metadata == nullptr)
  {
    // If we can't find the resource's metadata, that's ok. It just means we do
    // not have the ability to accept derived resources from base resource
    // indices. We can still check if the resource is explicitly accepted.
    return m_acceptable.find(resource->uniqueName()) != m_acceptable.end();
  }

  // With the resource's metadata, we can resolve queries for derived resources.
  auto& container = manager->metadata().get<smtk::resource::NameTag>();

  // For every element in the filter map...
  for (auto& acceptable : m_acceptable)
  {
    // ...we access the metadata for that resource type...
    auto md = container.find(acceptable);
    // ...and ask (a) if our resource is of that type, and (b) if its associated
    // filter accepts the component.
    if (md != container.end() && metadata->isOfType(md->index()))
    {
      return true;
    }
  }

  return false;
}

bool ResourceItemDefinition::setAcceptsResources(const std::string& uniqueName, bool accept)
{
  if (accept)
  {
    m_acceptable.insert(uniqueName);
    return true;
  }
  else
  {
    return m_acceptable.erase(uniqueName) != 0;
  }
}

/**\brief Is an attribute's value consistent with its definition?
  *
  * This returns false when the definition has an item bitmask
  * that does not include entities of the matching type.
  * However, if the model entity's type cannot be determined
  * because the model manager is NULL or it is not in the model
  * manager's storage, we silently assume that the value is valid.
  * Note that only UUIDs are stored
  */
bool ResourceItemDefinition::isValueValid(smtk::resource::ResourcePtr rsrc) const
{
  return rsrc ? this->acceptsResource(rsrc) : false;
}

//// Construct an item from the definition given its owning attribute and position.
smtk::attribute::ItemPtr ResourceItemDefinition::buildItem(
  Attribute* owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(new ResourceItem(owningAttribute, itemPosition));
}

//// Construct an item from the definition given its owning item and position.
smtk::attribute::ItemPtr ResourceItemDefinition::buildItem(
  Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new ResourceItem(owningItem, itemPosition, subGroupPosition));
}

/// Return the number of values (model entities) required by this definition.
std::size_t ResourceItemDefinition::numberOfRequiredValues() const
{
  return this->m_numberOfRequiredValues;
}

/// Set the number of values (model entities) required by this definition. Use 0 when there is no requirement.
void ResourceItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  if (esize == this->m_numberOfRequiredValues)
  {
    return;
  }
  this->m_numberOfRequiredValues = esize;
  if (!this->m_useCommonLabel)
  {
    this->m_valueLabels.resize(esize);
  }
}

/// Set the maximum number of values accepted (or 0 for no limit).
void ResourceItemDefinition::setMaxNumberOfValues(std::size_t maxNum)
{
  this->m_maxNumberOfValues = maxNum;
}

/// Return whether the definition provides labels for each value.
bool ResourceItemDefinition::hasValueLabels() const
{
  return !this->m_valueLabels.empty();
}

/// Return the label for the \a i-th model entity.
std::string ResourceItemDefinition::valueLabel(std::size_t i) const
{
  if (this->m_useCommonLabel)
  {
    assert(!this->m_valueLabels.empty());
    return this->m_valueLabels[0];
  }
  if (this->m_valueLabels.size())
  {
    assert(this->m_valueLabels.size() > i);
    return this->m_valueLabels[i];
  }
  return "";
}

/// Set the label for the \a i-th entity.
void ResourceItemDefinition::setValueLabel(std::size_t i, const std::string& elabel)
{
  if (this->m_numberOfRequiredValues == 0)
  {
    return;
  }
  if (this->m_valueLabels.size() != this->m_numberOfRequiredValues)
  {
    this->m_valueLabels.resize(this->m_numberOfRequiredValues);
  }
  this->m_useCommonLabel = false;
  this->m_valueLabels[i] = elabel;
}

/// Indicate that all values share the \a elabel provided.
void ResourceItemDefinition::setCommonValueLabel(const std::string& elabel)
{
  this->m_useCommonLabel = true;
  this->m_valueLabels.resize(1);
  this->m_valueLabels[0] = elabel;
}

/// Returns true when all values share a common label and false otherwise.
bool ResourceItemDefinition::usingCommonLabel() const
{
  return this->m_useCommonLabel;
}

/// Builds and returns copy of self
ItemDefinitionPtr ResourceItemDefinition::createCopy(ItemDefinition::CopyInfo& info) const
{
  (void)info;
  std::size_t i;

  smtk::attribute::ResourceItemDefinitionPtr newDef =
    smtk::attribute::ResourceItemDefinition::New(this->name());
  ItemDefinition::copyTo(newDef);

  newDef->setNumberOfRequiredValues(m_numberOfRequiredValues);
  newDef->setMaxNumberOfValues(m_maxNumberOfValues);
  newDef->setIsExtensible(m_isExtensible);
  for (auto& acceptable : m_acceptable)
  {
    newDef->setAcceptsResources(acceptable, true);
  }
  if (m_useCommonLabel)
  {
    newDef->setCommonValueLabel(m_valueLabels[0]);
  }
  else if (this->hasValueLabels())
  {
    for (i = 0; i < m_valueLabels.size(); ++i)
    {
      newDef->setValueLabel(i, m_valueLabels[i]);
    }
  }

  return newDef;
}
