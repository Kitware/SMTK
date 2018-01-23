//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/ComponentItem.h"

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
ComponentItemDefinition::ComponentItemDefinition(const std::string& sname)
  : ItemDefinition(sname)
{
  this->m_numberOfRequiredValues = 1;
  this->m_useCommonLabel = false;
  this->m_isExtensible = false;
  this->m_maxNumberOfValues = 0;
  this->m_isWritable = true;
}

/// Destructor.
ComponentItemDefinition::~ComponentItemDefinition()
{
}

/// Return the type of storage used by items defined by this class.
Item::Type ComponentItemDefinition::type() const
{
  return Item::ComponentType;
}

bool ComponentItemDefinition::setAcceptsResourceComponents(
  const std::string& uniqueName, const std::string& filter, bool accept)
{
  if (accept)
  {
    m_acceptable.insert(std::make_pair(uniqueName, filter));
    return true;
  }
  else
  {
    auto range = m_acceptable.equal_range(uniqueName);
    auto found = std::find_if(
      range.first, range.second, [&](decltype(*range.first) it) { return it.second == filter; });

    if (found != m_acceptable.end())
    {
      m_acceptable.erase(found);
      return true;
    }
    else
    {
      return false;
    }
  }
}

/// Is an attribute's value consistent with its definition?
bool ComponentItemDefinition::isValueValid(smtk::resource::ComponentPtr comp) const
{
  // If the component is invalid, then no filtering is needed.
  if (!comp)
  {
    return false;
  }

  // All components are required to have resources in order to be valid.
  auto rsrc = comp->resource();
  if (!rsrc)
  {
    return false;
  }

  // If there are no filter values, then we accept all components.
  if (m_acceptable.empty())
  {
    return true;
  }

  // Search for the resource index in the resource metdata.
  const smtk::resource::Metadata* metadata = nullptr;

  auto manager = rsrc->manager();
  if (manager)
  {
    auto& container = manager->metadata().get<smtk::resource::IndexTag>();
    auto metadataIt = container.find(rsrc->index());
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
    auto range = m_acceptable.equal_range(rsrc->uniqueName());
    for (auto& it = range.first; it != range.second; ++it)
    {
      if (rsrc->queryOperation(it->second)(comp))
      {
        return true;
      }
    }
    return false;
  }

  // With the resource's metadata, we can resolve queries for derived resources.
  auto& container = manager->metadata().get<smtk::resource::NameTag>();

  // For every element in the filter map...
  for (auto& acceptable : m_acceptable)
  {
    // ...we access the metadata for that resource type...
    auto md = container.find(acceptable.first);
    // ...and ask (a) if our resource is of that type, and (b) if its associated
    // filter accepts the component.
    if (md != container.end() && metadata->isOfType(md->index()) &&
      rsrc->queryOperation(acceptable.second)(comp))
    {
      return true;
    }
  }

  return false;
}

//// Construct an item from the definition given its owning attribute and position.
smtk::attribute::ItemPtr ComponentItemDefinition::buildItem(
  Attribute* owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(new ComponentItem(owningAttribute, itemPosition));
}

//// Construct an item from the definition given its owning item and position.
smtk::attribute::ItemPtr ComponentItemDefinition::buildItem(
  Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new ComponentItem(owningItem, itemPosition, subGroupPosition));
}

/// Return the number of values (model entities) required by this definition.
std::size_t ComponentItemDefinition::numberOfRequiredValues() const
{
  return this->m_numberOfRequiredValues;
}

/// Set the number of values (model entities) required by this definition. Use 0 when there is no requirement.
void ComponentItemDefinition::setNumberOfRequiredValues(std::size_t esize)
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
void ComponentItemDefinition::setMaxNumberOfValues(std::size_t maxNum)
{
  this->m_maxNumberOfValues = maxNum;
}

/// Return whether the definition provides labels for each value.
bool ComponentItemDefinition::hasValueLabels() const
{
  return !this->m_valueLabels.empty();
}

/// Return the label for the \a i-th model entity.
std::string ComponentItemDefinition::valueLabel(std::size_t i) const
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
void ComponentItemDefinition::setValueLabel(std::size_t i, const std::string& elabel)
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
void ComponentItemDefinition::setCommonValueLabel(const std::string& elabel)
{
  this->m_useCommonLabel = true;
  this->m_valueLabels.resize(1);
  this->m_valueLabels[0] = elabel;
}

/// Returns true when all values share a common label and false otherwise.
bool ComponentItemDefinition::usingCommonLabel() const
{
  return this->m_useCommonLabel;
}

/// Builds and returns copy of self
ItemDefinitionPtr ComponentItemDefinition::createCopy(ItemDefinition::CopyInfo& info) const
{
  (void)info;
  std::size_t i;

  smtk::attribute::ComponentItemDefinitionPtr newDef =
    smtk::attribute::ComponentItemDefinition::New(this->name());
  ItemDefinition::copyTo(newDef);

  newDef->setNumberOfRequiredValues(m_numberOfRequiredValues);
  newDef->setMaxNumberOfValues(m_maxNumberOfValues);
  newDef->setIsExtensible(m_isExtensible);
  for (auto& acceptable : m_acceptable)
  {
    newDef->setAcceptsResourceComponents(acceptable.first, acceptable.second, true);
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
