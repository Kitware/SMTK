//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <algorithm>

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include "units/System.h"

#include <iostream>

using namespace smtk::attribute;

ValueItemDefinition::ValueItemDefinition(const std::string& myName)
  : ItemDefinition(myName)
{
  m_defaultDiscreteIndex = -1;
  m_hasDefault = false;
  m_useCommonLabel = false;
  m_numberOfRequiredValues = 1;
  m_maxNumberOfValues = 0;
  m_isExtensible = false;
  m_expressionInformation = ComponentItemDefinition::New("expression");
  m_expressionInformation->setNumberOfRequiredValues(1);
}

ValueItemDefinition::~ValueItemDefinition() = default;

bool ValueItemDefinition::setUnits(const std::string& newUnits)
{
  m_units = newUnits;
  return true;
}

bool ValueItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  if (esize == m_numberOfRequiredValues)
  {
    return true;
  }
  if (m_maxNumberOfValues && (esize > m_maxNumberOfValues))
  {
    return false;
  }

  m_numberOfRequiredValues = esize;
  if (!this->hasValueLabels())
  {
    return true;
  }
  if (!(m_useCommonLabel || m_isExtensible))
  {
    m_valueLabels.resize(esize);
  }
  return true;
}

bool ValueItemDefinition::setMaxNumberOfValues(std::size_t esize)
{
  if (esize && (esize < m_numberOfRequiredValues))
  {
    return false;
  }
  m_maxNumberOfValues = esize;
  return true;
}

void ValueItemDefinition::setValueLabel(std::size_t element, const std::string& elabel)
{
  if (m_isExtensible)
  {
    return;
  }
  if (m_valueLabels.size() != m_numberOfRequiredValues)
  {
    m_valueLabels.resize(m_numberOfRequiredValues);
  }
  m_useCommonLabel = false;
  assert(m_valueLabels.size() > element);
  m_valueLabels[element] = elabel;
}

void ValueItemDefinition::setCommonValueLabel(const std::string& elabel)
{
  if (m_valueLabels.size() != 1)
  {
    m_valueLabels.resize(1);
  }
  m_useCommonLabel = true;
  assert(!m_valueLabels.empty());
  m_valueLabels[0] = elabel;
}

std::string ValueItemDefinition::valueLabel(std::size_t element) const
{
  if (m_useCommonLabel)
  {
    assert(!m_valueLabels.empty());
    return m_valueLabels[0];
  }
  if (element < m_valueLabels.size())
  {
    return m_valueLabels[element];
  }
  return ""; // If we threw execeptions this method could return const string &
}

bool ValueItemDefinition::isValidExpression(const smtk::attribute::AttributePtr& exp) const
{
  return this->allowsExpressions() && m_expressionInformation->isValueValid(exp);
}

bool ValueItemDefinition::allowsExpressions() const
{
  return (!m_expressionInformation->acceptableEntries().empty());
}

void ValueItemDefinition::setExpressionType(const std::string& etype)
{
  if (m_expressionType == etype)
  {
    return;
  }

  m_expressionInformation->clearAcceptableEntries();
  m_expressionInformation->clearRejectedEntries();

  m_expressionType = etype;

  if (!m_expressionType.empty())
  {
    std::string a = smtk::attribute::Resource::createAttributeQuery(m_expressionType);
    m_expressionInformation->setAcceptsEntries(
      smtk::common::typeName<attribute::Resource>(), a, true);
  }
}

std::string ValueItemDefinition::expressionType() const
{
  // return an empty string if there is not exactly 1 acceptable entry or
  // if there are are any rejected entries.
  if (
    (m_expressionInformation->acceptableEntries().size() != 1) ||
    (!m_expressionInformation->rejectedEntries().empty()))
  {
    return std::string();
  }

  auto entry = m_expressionInformation->acceptableEntries().begin();
  if (entry->first != smtk::common::typeName<attribute::Resource>())
  {
    return std::string(); // The entry does not refer to an attribute resource
  }

  auto entryInfo = Resource::extractGrammarInfo(entry->second);
  if (entryInfo.isRegex() || entryInfo.hasProperties())
  {
    return std::string(); // The entry is a Regex or contains property constraints
  }
  return entryInfo.typeInfo();
}

void ValueItemDefinition::setExpressionDefinition(const smtk::attribute::DefinitionPtr& exp)
{
  if (exp == nullptr)
  {
    this->setExpressionType("");
  }
  else
  {
    this->setExpressionType(exp->type());
  }
}

smtk::attribute::DefinitionPtr ValueItemDefinition::expressionDefinition(
  const smtk::attribute::ResourcePtr& attResource) const
{
  auto expType = this->expressionType();
  if (expType.empty())
  {
    return nullptr;
  }
  return attResource->findDefinition(expType);
}

void ValueItemDefinition::buildExpressionItem(ValueItem* vitem) const
{
  auto expItem = smtk::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
    m_expressionInformation->buildItem(vitem, 0, -1));
  expItem->setDefinition(m_expressionInformation);
  vitem->m_expression = expItem;
}

void ValueItemDefinition::buildChildrenItems(ValueItem* vitem) const
{
  std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator it;
  smtk::attribute::ItemPtr child;
  for (it = m_itemDefs.begin(); it != m_itemDefs.end(); it++)
  {
    child = it->second->buildItem(vitem, 0, -1);
    child->setDefinition(it->second);
    vitem->m_childrenItems[it->first] = child;
  }
}

void ValueItemDefinition::setDefaultDiscreteIndex(int discreteIndex)
{
  m_defaultDiscreteIndex = discreteIndex;
  this->updateDiscreteValue();
  m_hasDefault = true;
}

bool ValueItemDefinition::addChildItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
{
  return this->addItemDefinition(cdef);
}

bool ValueItemDefinition::addConditionalItem(
  const std::string& valueName,
  const std::string& itemName)
{
  // Do we have this valueName?
  if (
    std::find(m_discreteValueEnums.begin(), m_discreteValueEnums.end(), valueName) ==
    m_discreteValueEnums.end())
  {
    return false;
  }
  // Next do we have such an item definition?
  if (!this->hasChildItemDefinition(itemName))
  {
    return false;
  }

  // Finally we need to verify that we don't already have this item assigned
  if (this->hasChildItemDefinition(valueName, itemName))
  {
    return false;
  }

  // create the association
  m_valueToItemAssociations[valueName].push_back(itemName);
  m_itemToValueAssociations[itemName].insert(valueName);
  return true;
}

std::vector<std::string> ValueItemDefinition::conditionalItems(const std::string& valueName) const
{
  // Do we have this valueName?
  if (
    std::find(m_discreteValueEnums.begin(), m_discreteValueEnums.end(), valueName) ==
    m_discreteValueEnums.end())
  {
    std::vector<std::string> temp;
    return temp;
  }
  std::map<std::string, std::vector<std::string>>::const_iterator citer =
    m_valueToItemAssociations.find(valueName);
  // Does the value have conditional items associated with it?
  if (citer == m_valueToItemAssociations.end())
  {
    std::vector<std::string> dummy;
    return dummy;
  }
  return citer->second;
}

void ValueItemDefinition::applyCategories(
  const smtk::common::Categories::Stack& inheritedFromParent,
  smtk::common::Categories& inheritedToParent)
{
  // Lets first determine the set of categories this item definition could inherit
  m_categories.reset();
  smtk::common::Categories::Stack myCats = inheritedFromParent;
  myCats.append(m_combinationMode, m_localCategories);
  // Lets insert the combination of this Item's categories with those that were inherited
  m_categories.insert(myCats);

  smtk::common::Categories myChildrenCats;

  // Now process the children item defs - this will also assembly the categories
  // this item def will inherit from its children based on their local categories
  for (auto& i : m_itemDefs)
  {
    i.second->applyCategories(myCats, myChildrenCats);
  }

  // NOTE - concerning category information associated with enums - We assume that this
  // information is used by the UI to show the user what values are available based on the
  // active categories but they DO NOT effect the Item's validity or relevance.

  // Add the children categories to this one
  m_categories.insert(myChildrenCats);
  // update the set of categories being inherited by the owning item/attribute
  // definition
  inheritedToParent.insert(m_categories);
}

void ValueItemDefinition::applyAdvanceLevels(
  const unsigned int& readLevelFromParent,
  const unsigned int& writeLevelFromParent)
{
  ItemDefinition::applyAdvanceLevels(readLevelFromParent, writeLevelFromParent);
  for (auto& item : m_itemDefs)
  {
    item.second->applyAdvanceLevels(m_advanceLevel[0], m_advanceLevel[1]);
  }
}

void ValueItemDefinition::setIsExtensible(bool mode)
{
  m_isExtensible = mode;
  if (!this->hasValueLabels())
  {
    // If there are no value labels there is nothing to do
    return;
  }

  if (mode && !this->usingCommonLabel())
  {
    // Need to clear individual labels - can only use common label with
    // extensible values
    this->setCommonValueLabel("");
  }
}

void ValueItemDefinition::copyTo(
  ValueItemDefinitionPtr def,
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  std::size_t i;

  ItemDefinition::copyTo(def);

  if (!m_units.empty())
  {
    def->setUnits(m_units);
  }

  if (this->allowsExpressions())
  {

    // Set expression definition (if possible)
    smtk::attribute::DefinitionPtr exp = info.ToResource.findDefinition(m_expressionType);
    if (exp)
    {
      def->setExpressionDefinition(exp);
    }
    else
    {
      // In this case we have yet to find the Expression Definition.
      // Lets copy the reference Item Definition Information at least.  This would be useful
      // if the Expression Definition does not exists in the Source Attribute Resource.  For example
      // a workflow could be storing all of the Expressions in their Attribute Resource.
      def->m_expressionType = m_expressionType;
      m_expressionInformation->copyTo(def->m_expressionInformation, info);

      // In the case that the Expression Definition exists in the source Attribute Resource,
      // Lets queue it up so that this Item Definition's Expression Definition can be set once
      // it has been copied.
      info.UnresolvedExpItems.emplace(m_expressionType, def);
    }
  }

  def->setNumberOfRequiredValues(m_numberOfRequiredValues);
  def->setMaxNumberOfValues(m_maxNumberOfValues);
  def->setIsExtensible(m_isExtensible);

  // Add label(s)
  if (m_useCommonLabel)
  {
    assert(!m_valueLabels.empty());
    def->setCommonValueLabel(m_valueLabels[0]);
  }
  else if (this->hasValueLabels())
  {
    for (i = 0; i < m_valueLabels.size(); ++i)
    {
      def->setValueLabel(i, m_valueLabels[i]);
    }
  }

  // Add children item definitions
  if (!m_itemDefs.empty())
  {
    std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator itemDefMapIter =
      m_itemDefs.begin();
    for (; itemDefMapIter != m_itemDefs.end(); itemDefMapIter++)
    {
      smtk::attribute::ItemDefinitionPtr itemDef = itemDefMapIter->second->createCopy(info);
      def->addChildItemDefinition(itemDef);
    }
  }

  // Add condition items
  if (!m_valueToItemAssociations.empty())
  {
    std::map<std::string, std::vector<std::string>>::const_iterator mapIter =
      m_valueToItemAssociations.begin();
    std::string value;
    std::vector<std::string>::const_iterator itemIter;
    for (; mapIter != m_valueToItemAssociations.end(); mapIter++)
    {
      value = mapIter->first;
      itemIter = mapIter->second.begin();
      for (; itemIter != mapIter->second.end(); itemIter++)
      {
        def->addConditionalItem(value, *itemIter);
      }
    }
  }
}

bool ValueItemDefinition::getEnumIndex(const std::string& enumVal, std::size_t& index) const
{
  std::size_t i, n = m_discreteValueEnums.size();
  for (i = static_cast<std::size_t>(0); i < n; i++)
  {
    if (m_discreteValueEnums.at(i) == enumVal)
    {
      index = i;
      return true;
    }
  }
  return false;
}

void ValueItemDefinition::setEnumCategories(
  const std::string& enumValue,
  const smtk::common::Categories::Expression& exp)
{
  if (
    std::find(m_discreteValueEnums.begin(), m_discreteValueEnums.end(), enumValue) ==
    m_discreteValueEnums.end())
  {
    return; // enum not defined
  }
  m_valueToCategoryAssociations[enumValue] = exp;
}

void ValueItemDefinition::addEnumCategory(const std::string& enumValue, const std::string& cat)
{
  if (
    std::find(m_discreteValueEnums.begin(), m_discreteValueEnums.end(), enumValue) ==
    m_discreteValueEnums.end())
  {
    return; // enum not defined
  }
  std::string expString("'");
  expString.append(cat).append("'");
  m_valueToCategoryAssociations[enumValue].setExpression(expString);
}

const smtk::common::Categories::Expression& ValueItemDefinition::enumCategories(
  const std::string& enumValue) const
{
  static smtk::common::Categories::Expression dummy;
  dummy.setAllPass();
  auto result = m_valueToCategoryAssociations.find(enumValue);
  if (result == m_valueToCategoryAssociations.end())
  {
    return dummy; // enum does not have explicit categories
  }
  return result->second;
}

void ValueItemDefinition::setEnumAdvanceLevel(const std::string& enumValue, unsigned int level)
{
  if (
    std::find(m_discreteValueEnums.begin(), m_discreteValueEnums.end(), enumValue) ==
    m_discreteValueEnums.end())
  {
    return; // enum not defined
  }
  m_valueToAdvanceLevelAssociations[enumValue] = level;
}

void ValueItemDefinition::unsetEnumAdvanceLevel(const std::string& enumValue)
{
  if (
    std::find(m_discreteValueEnums.begin(), m_discreteValueEnums.end(), enumValue) ==
    m_discreteValueEnums.end())
  {
    return; // enum not defined
  }
  m_valueToAdvanceLevelAssociations.erase(enumValue);
}

unsigned int ValueItemDefinition::enumAdvanceLevel(const std::string& enumValue) const
{
  auto result = m_valueToAdvanceLevelAssociations.find(enumValue);
  if (result == m_valueToAdvanceLevelAssociations.end())
  {
    return 0;
  }
  return result->second;
}

bool ValueItemDefinition::hasEnumAdvanceLevel(const std::string& enumValue) const
{
  auto result = m_valueToAdvanceLevelAssociations.find(enumValue);
  return (result != m_valueToAdvanceLevelAssociations.end());
}

bool ValueItemDefinition::addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
{
  if (this->hasChildItemDefinition(cdef->name()))
  {
    return false;
  }
  m_itemDefs[cdef->name()] = cdef;
  cdef->setUnitSystem(m_unitSystem);
  return true;
}

bool ValueItemDefinition::defaultIsEnumRelevant(
  int enumIndex,
  bool includeCategories,
  const std::set<std::string>& testCategories,
  bool includeReadAccess,
  unsigned int readAccessLevel) const
{
  if (includeCategories)
  {
    const auto& catExp = this->enumCategories(m_discreteValueEnums[enumIndex]);
    if (catExp.allPass() || catExp.passes(testCategories))
    {
      if (
        (!includeReadAccess) ||
        (this->enumAdvanceLevel(m_discreteValueEnums[enumIndex]) <= readAccessLevel))
      {
        return true;
      }
    }
    return false;
  }

  return (
    (!includeReadAccess) ||
    (this->enumAdvanceLevel(m_discreteValueEnums[enumIndex]) <= readAccessLevel));
}

std::vector<std::string> ValueItemDefinition::relevantEnums(
  bool includeCategories,
  const std::set<std::string>& testCategories,
  bool includeReadAccess,
  unsigned int readAccessLevel) const
{
  std::vector<std::string> result;
  if (!(includeCategories || includeReadAccess))
  {
    return m_discreteValueEnums;
  }

  for (std::size_t i = 0; i < m_discreteValueEnums.size(); i++)
  {
    if (this->defaultIsEnumRelevant(
          i, includeCategories, testCategories, includeReadAccess, readAccessLevel))
    {
      result.push_back(m_discreteValueEnums[i]);
    }
  }
  return result;
}

void ValueItemDefinition::setUnitSystem(const shared_ptr<units::System>& unitSystem)
{
  m_unitSystem = unitSystem;

  for (const auto& item : m_itemDefs)
  {
    item.second->setUnitSystem(m_unitSystem);
  }
}

bool ValueItemDefinition::isDiscreteIndexValid(int index, const std::set<std::string>& categories)
  const
{
  // Is the index out of range?
  if ((index < 0) || (static_cast<unsigned int>(index) >= m_discreteValueEnums.size()))
  {
    return false;
  }

  const auto& catExp = this->enumCategories(m_discreteValueEnums[index]);
  return (catExp.allPass() || catExp.passes(categories));
}

bool ValueItemDefinition::isDiscreteIndexValid(int index) const
{
  return ((index > -1) && (static_cast<unsigned int>(index) < m_discreteValueEnums.size()));
}

bool ValueItemDefinition::hasSupportedUnits() const
{
  if (!(m_units.empty() || (m_unitSystem == nullptr)))
  {
    bool status;
    auto defUnit = m_unitSystem->unit(m_units, &status);
    return status;
  }
  return false;
}

std::string ValueItemDefinition::supportedUnits() const
{
  bool status = false;
  if (!(m_units.empty() || (m_unitSystem == nullptr)))
  {
    auto defUnit = m_unitSystem->unit(m_units, &status);
  }
  return (status ? m_units : std::string());
}
