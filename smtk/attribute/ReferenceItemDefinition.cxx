//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_ReferenceItemDefinition_txx
#define smtk_attribute_ReferenceItemDefinition_txx

#include "smtk/attribute/ReferenceItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"

#include "smtk/resource/Container.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include <algorithm>
#include <cassert>

namespace smtk
{
namespace attribute
{

ReferenceItemDefinition::ReferenceItemDefinition(const std::string& sname)
  : Superclass(sname)
{
  m_numberOfRequiredValues = 1;
  m_useCommonLabel = false;
  m_isExtensible = false;
  m_maxNumberOfValues = 0;
  m_lockType = smtk::resource::LockType::Write;
  m_role = smtk::attribute::Resource::ReferenceRole;
  m_holdReference = false;
  m_onlyResources = false;
}

ReferenceItemDefinition::~ReferenceItemDefinition() = default;

bool ReferenceItemDefinition::setAcceptsEntries(
  const std::string& typeName,
  const std::string& filter,
  bool accept)
{
  if (accept)
  {
    m_acceptable.insert(std::make_pair(typeName, filter));
    return true;
  }
  else
  {
    auto range = m_acceptable.equal_range(typeName);
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

bool ReferenceItemDefinition::setRejectsEntries(
  const std::string& typeName,
  const std::string& filter,
  bool add)
{
  if (add)
  {
    m_rejected.insert(std::make_pair(typeName, filter));
    return true;
  }
  else
  {
    auto range = m_rejected.equal_range(typeName);
    auto found = std::find_if(
      range.first, range.second, [&](decltype(*range.first) it) { return it.second == filter; });

    if (found != m_rejected.end())
    {
      m_rejected.erase(found);
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool ReferenceItemDefinition::isValueValid(resource::ConstPersistentObjectPtr entity) const
{
  bool ok = false;
  if (!entity)
  {
    return ok;
  }

  const smtk::resource::Resource* rsrc;
  const smtk::resource::Component* comp;
  if ((rsrc = dynamic_cast<const smtk::resource::Resource*>(entity.get())))
  {
    ok = this->checkResource(*rsrc);
  }
  else if ((comp = dynamic_cast<const smtk::resource::Component*>(entity.get())))
  {
    ok = this->checkComponent(comp);
  }
  return ok;
}

std::size_t ReferenceItemDefinition::numberOfRequiredValues() const
{
  return m_numberOfRequiredValues;
}

void ReferenceItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  if (esize == m_numberOfRequiredValues)
  {
    return;
  }
  m_numberOfRequiredValues = esize;
  if (!m_useCommonLabel)
  {
    m_valueLabels.resize(esize);
  }
}

void ReferenceItemDefinition::setMaxNumberOfValues(std::size_t maxNum)
{
  m_maxNumberOfValues = maxNum;
}

bool ReferenceItemDefinition::hasValueLabels() const
{
  return !m_valueLabels.empty();
}

std::string ReferenceItemDefinition::valueLabel(std::size_t i) const
{
  if (m_useCommonLabel)
  {
    assert(!m_valueLabels.empty());
    return m_valueLabels[0];
  }
  if (!m_valueLabels.empty())
  {
    assert(m_valueLabels.size() > i);
    return m_valueLabels[i];
  }
  return "";
}

void ReferenceItemDefinition::setValueLabel(std::size_t i, const std::string& elabel)
{
  if (m_numberOfRequiredValues == 0)
  {
    return;
  }
  if (m_valueLabels.size() != m_numberOfRequiredValues)
  {
    m_valueLabels.resize(m_numberOfRequiredValues);
  }
  m_useCommonLabel = false;
  m_valueLabels[i] = elabel;
}

void ReferenceItemDefinition::setCommonValueLabel(const std::string& elabel)
{
  m_useCommonLabel = true;
  m_valueLabels.resize(1);
  m_valueLabels[0] = elabel;
}

bool ReferenceItemDefinition::usingCommonLabel() const
{
  return m_useCommonLabel;
}

smtk::attribute::ItemPtr ReferenceItemDefinition::buildItem(
  Attribute* owningAttribute,
  int itemPosition) const
{
  return smtk::attribute::ItemPtr(new ReferenceItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr
ReferenceItemDefinition::buildItem(Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new ReferenceItem(owningItem, itemPosition, subGroupPosition));
}

smtk::attribute::ItemDefinitionPtr ReferenceItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;
  auto copy = ReferenceItemDefinition::New(this->name());
  this->copyTo(copy, info);
  return copy;
}

void ReferenceItemDefinition::copyTo(Ptr dest, smtk::attribute::ItemDefinition::CopyInfo& info)
  const
{
  if (!dest)
  {
    return;
  }

  Superclass::copyTo(dest);

  dest->setNumberOfRequiredValues(m_numberOfRequiredValues);
  dest->setMaxNumberOfValues(m_maxNumberOfValues);
  dest->setIsExtensible(m_isExtensible);
  for (const auto& acceptable : m_acceptable)
  {
    dest->setAcceptsEntries(acceptable.first, acceptable.second, true);
  }
  for (const auto& rejected : m_rejected)
  {
    dest->setRejectsEntries(rejected.first, rejected.second, true);
  }
  if (m_useCommonLabel)
  {
    dest->setCommonValueLabel(m_valueLabels[0]);
  }
  else if (this->hasValueLabels())
  {
    std::size_t ii;
    for (ii = 0; ii < m_valueLabels.size(); ++ii)
    {
      dest->setValueLabel(ii, m_valueLabels[ii]);
    }
  }

  // Add children item definitions
  if (!m_itemDefs.empty())
  {
    for (auto itemDefMapIter = m_itemDefs.begin(); itemDefMapIter != m_itemDefs.end();
         itemDefMapIter++)
    {
      smtk::attribute::ItemDefinitionPtr itemDef = itemDefMapIter->second->createCopy(info);
      dest->addChildItemDefinition(itemDef);
    }
  }

  // Add conditional items
  dest->m_resourceQueries = m_resourceQueries;
  dest->m_componentQueries = m_componentQueries;
  dest->m_conditionalItemNames = m_conditionalItemNames;
}

bool ReferenceItemDefinition::checkResource(const smtk::resource::Resource& rsrc) const
{
  // TODO:
  // Queries to filter resources as acceptable have not been implemented.
  // See smtk::attribute::ComponentItemDefinition::isValueValid() for
  // a pattern to follow when implementing this.
  //
  // For now all we do is test the resource names in m_acceptable
  // to see if any are exact matches for rsrc.

  // For every element in the rejected filter map...
  for (const auto& rejected : m_rejected)
  {
    // ...we check if the resource in question is of that type. Rejected
    // entries for resources do not have a filter string, so we check that
    // the filter string is empty.
    if ((rejected.second.empty() || m_onlyResources) && rsrc.isOfType(rejected.first))
    {
      return false;
    }
  }

  // If there are no filter values, then we accept all resources.
  if (m_acceptable.empty())
  {
    return true;
  }

  // Finally, for every element in the accepted filter map...
  using ValueType = std::multimap<std::string, std::string>::value_type;
  return std::any_of(
    m_acceptable.begin(),
    m_acceptable.end(),
    // ...we check if the resource in question is of that type. Acceptable
    // entries for resources do not have a filter string, so we check that
    // the filter string is empty.
    [this, &rsrc](const ValueType& acceptable) {
      return (acceptable.second.empty() || this->m_onlyResources) &&
        rsrc.isOfType(acceptable.first);
    });
}

bool ReferenceItemDefinition::checkCategories(const smtk::resource::Component* comp) const
{
  if (!m_enforcesCategories)
  {
    return true;
  }

  const auto* const att = dynamic_cast<const smtk::attribute::Attribute*>(comp);
  if (!att)
  {
    return true;
  }
  auto attRes = att->attributeResource();
  if (attRes && attRes->activeCategoriesEnabled())
  {
    return att->categories().passes(attRes->activeCategories());
  }

  return true;
}

bool ReferenceItemDefinition::checkComponent(const smtk::resource::Component* comp) const
{
  // All components are required to have resources in order to be valid.
  auto rsrc = comp->resource();
  if (m_onlyResources || !rsrc)
  {
    return false;
  }

  // For every element in the rejected filter map...
  for (const auto& rejected : m_rejected)
  {
    // ...ask (a) if the filter explicitly rejects components, (b) if our
    // resource is of the right type, and (b) if its associated filter accepts
    // the component.
    if (rsrc->isOfType(rejected.first) && rsrc->queryOperation(rejected.second)(*comp))
    {
      return false;
    }
  }

  // If there are no filter values, then we accept all components.
  if (m_acceptable.empty())
  {
    return this->checkCategories(comp);
  }

  // For every element in the accepted filter map...
  for (const auto& acceptable : m_acceptable)
  {
    // ...ask (a) if the filter explicitly rejects components, (b) if our
    // resource is of the right type, and (b) if its associated filter accepts
    // the component.
    if (
      !m_onlyResources && rsrc->isOfType(acceptable.first) &&
      rsrc->queryOperation(acceptable.second)(*comp))
    {
      return this->checkCategories(comp);
    }
  }

  return false;
}

std::size_t ReferenceItemDefinition::addConditional(
  const std::string& resourceQuery,
  const std::string& componentQuery,
  const std::vector<std::string>& itemNames)
{
  // Both query strings can not both be empty
  if (resourceQuery.empty() && componentQuery.empty())
  {
    return s_invalidIndex;
  }

  // Lets make sure all of the item names exist
  for (const auto& itemName : itemNames)
  {
    if (!this->hasChildItemDefinition(itemName))
    {
      return s_invalidIndex;
    }
  }
  m_resourceQueries.push_back(resourceQuery);
  m_componentQueries.push_back(componentQuery);
  m_conditionalItemNames.push_back(itemNames);
  return m_conditionalItemNames.size() - 1;
}

const std::vector<std::string>& ReferenceItemDefinition::conditionalItems(std::size_t ith) const
{
  // Is this a valid index
  static std::vector<std::string> dummy;
  if (ith < m_conditionalItemNames.size())
  {
    return m_conditionalItemNames[ith];
  }
  return dummy;
}

void ReferenceItemDefinition::applyCategories(
  const smtk::attribute::Categories::Stack& inheritedFromParent,
  smtk::attribute::Categories& inheritedToParent)
{
  Categories::Stack myCats = inheritedFromParent;
  myCats.append(m_combinationMode, m_localCategories);
  // Lets first determine the set of categories this item definition could inherit
  m_categories.reset();
  m_categories.insert(myCats);

  smtk::attribute::Categories myChildrenCats;

  // Now process the children item defs - this will also assembly the categories
  // this item def will inherit from its children based on their local categories
  for (auto& i : m_itemDefs)
  {
    i.second->applyCategories(myCats, myChildrenCats);
  }

  // Add the children categories to this one
  m_categories.insert(myChildrenCats);
  // update the set of categories being inherited by the owning item/attribute
  // definition
  inheritedToParent.insert(m_categories);
}

bool ReferenceItemDefinition::addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
{
  return this->addChildItemDefinition(cdef);
}

bool ReferenceItemDefinition::addChildItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
{
  // First see if there is a item by the same name
  if (this->hasChildItemDefinition(cdef->name()))
  {
    return false;
  }
  m_itemDefs[cdef->name()] = cdef;
  return true;
}

void ReferenceItemDefinition::buildChildrenItems(ReferenceItem* ritem) const
{
  std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator it;
  smtk::attribute::ItemPtr child;
  for (it = m_itemDefs.begin(); it != m_itemDefs.end(); it++)
  {
    child = it->second->buildItem(ritem, 0, -1);
    child->setDefinition(it->second);
    ritem->m_childrenItems[it->first] = child;
  }
}

std::size_t ReferenceItemDefinition::testConditionals(PersistentObjectPtr& object) const
{
  if (object == nullptr)
  {
    return s_invalidIndex; // No match is there is no object
  }

  // Are we dealing with a Resource?
  const smtk::resource::Resource* rsrc =
    dynamic_cast<const smtk::resource::Resource*>(object.get());
  std::size_t i, n = m_conditionalItemNames.size();
  if (rsrc)
  {
    for (i = 0; i < n; i++)
    {
      if (rsrc->isOfType(m_resourceQueries[i]))
      {
        return i; // return the index of the matched conditional
      }
    }
    return s_invalidIndex; // No match
  }

  // We dealing with a Component?
  const smtk::resource::Component* comp =
    dynamic_cast<const smtk::resource::Component*>(object.get());
  rsrc = comp->resource().get();

  if (comp == nullptr)
  {
    return s_invalidIndex; // Object is neither a resource or component
  }
  for (i = 0; i < n; i++)
  {
    if (!((m_resourceQueries[i].empty()) || rsrc->isOfType(m_resourceQueries[i])))
    {
      continue; // failed to match the resource condition
    }
    auto queryOp = rsrc->queryOperation(m_componentQueries[i]);
    if ((queryOp != nullptr) && queryOp(*comp))
    {
      return i;
    }
  }
  return s_invalidIndex;
}
} // namespace attribute
} // namespace smtk
#endif
