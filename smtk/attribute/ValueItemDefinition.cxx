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

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include <iostream>

using namespace smtk::attribute;

ValueItemDefinition::ValueItemDefinition(const std::string &myName):
  ItemDefinition(myName)
{
  this->m_defaultDiscreteIndex = -1;
  this->m_hasDefault = false;
  this->m_useCommonLabel = false;
  this->m_numberOfRequiredValues = 1;
  this->m_maxNumberOfValues = 0;
  this->m_isExtensible = false;
  this->m_expressionDefinition = RefItemDefinition::New("expression");
  this->m_expressionDefinition->setNumberOfRequiredValues(1);
}

ValueItemDefinition::~ValueItemDefinition()
{
}

bool ValueItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  if (esize == this->m_numberOfRequiredValues)
    {
    return true;
    }
  if (this->m_maxNumberOfValues && (esize > this->m_maxNumberOfValues))
    {
    return false;
    }

  this->m_numberOfRequiredValues = esize;
  if (!this->hasValueLabels())
    {
    return true;
    }
  if (!(this->m_useCommonLabel || this->m_isExtensible))
    {
    this->m_valueLabels.resize(esize);
    }
  return true;
}

bool  ValueItemDefinition::setMaxNumberOfValues(std::size_t esize)
{
  if (esize && (esize < this->m_numberOfRequiredValues))
    {
    return false;
    }
  this->m_maxNumberOfValues = esize;
  return true;
}

void ValueItemDefinition::setValueLabel(std::size_t element, const std::string &elabel)
{
  if (this->m_isExtensible)
    {
    return;
    }
  if (this->m_valueLabels.size() != this->m_numberOfRequiredValues)
    {
    this->m_valueLabels.resize(this->m_numberOfRequiredValues);
    }
  this->m_useCommonLabel = false;
  assert(this->m_valueLabels.size() > element);
  this->m_valueLabels[element] = elabel;
}

void ValueItemDefinition::setCommonValueLabel(const std::string &elabel)
{
  if (this->m_valueLabels.size() != 1)
    {
    this->m_valueLabels.resize(1);
    }
  this->m_useCommonLabel = true;
  assert(!this->m_valueLabels.empty());
  this->m_valueLabels[0] = elabel;
}

std::string ValueItemDefinition::valueLabel(std::size_t element) const
{
  if (this->m_useCommonLabel)
    {
    assert(!this->m_valueLabels.empty());
    return this->m_valueLabels[0];
    }
  if (element < this->m_valueLabels.size())
    {
    return this->m_valueLabels[element];
    }
  return ""; // If we threw execeptions this method could return const string &
}

bool ValueItemDefinition::isValidExpression(smtk::attribute::AttributePtr exp) const
{
  if (this->m_expressionDefinition->attributeDefinition() &&
      this->m_expressionDefinition->isValueValid(exp))
    {
    return true;
    }
  return false;
}

bool ValueItemDefinition::allowsExpressions() const
{
  return this->m_expressionDefinition->attributeDefinition() ? true : false;
}

smtk::attribute::DefinitionPtr ValueItemDefinition::expressionDefinition() const
{
  return this->m_expressionDefinition->attributeDefinition();
}

void
ValueItemDefinition::setExpressionDefinition(smtk::attribute::DefinitionPtr exp)
{
  this->m_expressionDefinition->setAttributeDefinition(exp);
}

void
ValueItemDefinition::buildExpressionItem(ValueItem *vitem, int position) const
{
  smtk::attribute::RefItemPtr aref =
    smtk::dynamic_pointer_cast<smtk::attribute::RefItem>
    (this->m_expressionDefinition->buildItem(vitem, position, -1));
  aref->setDefinition(this->m_expressionDefinition);
  assert(vitem->m_expressions.size() > static_cast<size_t>(position));
  vitem->m_expressions[static_cast<size_t>(position)] = aref;
}

void
ValueItemDefinition::buildChildrenItems(ValueItem *vitem) const
{
  std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator it;
  smtk::attribute::ItemPtr child;
  for (it = this->m_itemDefs.begin(); it != this->m_itemDefs.end(); it++)
    {
    child = it->second->buildItem(vitem, 0, -1);
    child->setDefinition(it->second);
    vitem->m_childrenItems[it->first] = child;
    }
}

void ValueItemDefinition::setDefaultDiscreteIndex(int discreteIndex)
{
  this->m_defaultDiscreteIndex = discreteIndex;
  this->updateDiscreteValue();
  this->m_hasDefault = true;
}

bool ValueItemDefinition::
addChildItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
{
  // First see if there is a item by the same name
  if (this->hasChildItemDefinition(cdef->name()))
    {
    return false;
    }
  this->m_itemDefs[cdef->name()] = cdef;
  return true;
}

bool ValueItemDefinition::
addConditionalItem(const std::string &valueName, const std::string &itemName)
{
  // Do we have this valueName?
  if (std::find(this->m_discreteValueEnums.begin(),
                this->m_discreteValueEnums.end(), valueName) ==
      this->m_discreteValueEnums.end())
    {
    return false;
    }
  // Next do we have such an iten definition?
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
  this->m_valueToItemAssociations[valueName].push_back(itemName);
  this->m_itemToValueAssociations[itemName].insert(valueName);
  return true;
}

std::vector<std::string>
ValueItemDefinition::conditionalItems(const std::string &valueName) const
{
  // Do we have this valueName?
  if (std::find(this->m_discreteValueEnums.begin(),
                this->m_discreteValueEnums.end(), valueName) ==
      this->m_discreteValueEnums.end())
    {
    std::vector<std::string> temp;
    return temp;
    }
  std::map<std::string, std::vector<std::string> >::const_iterator citer =
    this->m_valueToItemAssociations.find(valueName);
  // Does the value have conditional items associated with it?
  if (citer == this->m_valueToItemAssociations.end())
    {
    std::vector<std::string> dummy;
    return dummy;
    }
  return citer->second;
}

void ValueItemDefinition::setIsExtensible(bool mode)
{
  this->m_isExtensible = mode;
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

void ValueItemDefinition::
copyTo(ValueItemDefinitionPtr def,
       smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  std::size_t i;

  ItemDefinition::copyTo(def);

  if (this->m_units != "")
    {
    def->setUnits(this->m_units);
    }

  if (this->allowsExpressions())
    {
    // Set expression definition (if possible)
    std::string typeStr = this->expressionDefinition()->type();
    smtk::attribute::DefinitionPtr exp = info.ToSystem.findDefinition(typeStr);
    if (exp)
      {
      def->setExpressionDefinition(exp);
      }
    else
      {
      std::cout << "Adding definition \"" << typeStr
                << "\" to copy-expression queue"
                << std::endl;

      info.UnresolvedExpItems.push(std::make_pair(typeStr, def));
      }
    }

  def->setNumberOfRequiredValues(this->m_numberOfRequiredValues);
  def->setMaxNumberOfValues(this->m_maxNumberOfValues);
  def->setIsExtensible(this->m_isExtensible);

  // Add label(s)
  if (this->m_useCommonLabel)
    {
    assert(!this->m_valueLabels.empty());
    def->setCommonValueLabel(this->m_valueLabels[0]);
    }
  else if (this->hasValueLabels())
    {
    for (i=0; i<this->m_valueLabels.size(); ++i)
      {
      def->setValueLabel(i, this->m_valueLabels[i]);
      }
    }

  // Add children item definitions
  if (this->m_itemDefs.size() > 0)
    {
    std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator
      itemDefMapIter = this->m_itemDefs.begin();
    for (; itemDefMapIter != this->m_itemDefs.end(); itemDefMapIter++)
      {
      smtk::attribute::ItemDefinitionPtr itemDef =
        itemDefMapIter->second->createCopy(info);
      def->addChildItemDefinition(itemDef);
      }
    }

  // Add condition items
  if (this->m_valueToItemAssociations.size() > 0)
    {
    std::map<std::string, std::vector<std::string> >::const_iterator mapIter =
      this->m_valueToItemAssociations.begin();
    std::string value;
    std::vector<std::string>::const_iterator itemIter;
    for (; mapIter != this->m_valueToItemAssociations.end(); mapIter++)
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
