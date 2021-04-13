//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Evaluator.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include <algorithm> // for std::find
#include <iostream>

using namespace smtk::attribute;

ValueItem::ValueItem(Attribute* owningAttribute, int itemPosition)
  : Item(owningAttribute, itemPosition)
{
}

ValueItem::ValueItem(Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

bool ValueItem::setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const ValueItemDefinition* def = dynamic_cast<const ValueItemDefinition*>(vdef.get());
  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if (!def || (!Item::setDefinition(vdef)))
  {
    return false;
  }

  // Build the item's children
  def->buildChildrenItems(this);

  // Find out how many values this item is suppose to have
  // if the size is 0 then its unbounded
  std::size_t n = def->numberOfRequiredValues();
  if (n)
  {
    if (def->hasDefault())
    {
      m_isSet.resize(n, true);
    }
    else
    {
      m_isSet.resize(n, false);
    }
    if (def->isDiscrete())
    {
      m_discreteIndices.resize(n, def->defaultDiscreteIndex());
      this->updateActiveChildrenItems();
    }
    if (def->allowsExpressions())
    {
      def->buildExpressionItem(this);
    }
  }

  return true;
}

ValueItem::~ValueItem()
{
  // we need to detach all items that are owned by this. i.e. the expression item
  if (m_expression)
  {
    m_expression->detachOwningItem();
  }
}

void ValueItem::unset(std::size_t elementIndex)
{
  assert(m_isSet.size() > elementIndex);
  m_isSet[elementIndex] = false;
  // Clear the current list of active children items
  m_activeChildrenItems.clear();
}

bool ValueItem::isSet(std::size_t elementIndex) const
{
  // Are we using an expression?
  if (this->allowsExpressions() && (m_expression->value() != nullptr))
  {
    return true;
  }

  return m_isSet.size() > elementIndex ? m_isSet[elementIndex] : false;
}

smtk::attribute::AttributePtr ValueItem::expression() const
{
  return (
    m_expression ? dynamic_pointer_cast<smtk::attribute::Attribute>(m_expression->value())
                 : smtk::attribute::AttributePtr());
}
bool ValueItem::isExpression() const
{
  return !((m_expression == nullptr) || (m_expression->value() == nullptr));
}

bool ValueItem::isValidInternal(bool useCategories, const std::set<std::string>& categories) const
{
  // If we have been given categories we need to see if the item passes its
  // category checks - if it doesn't it means its not be taken into account
  // for validity checking so just return true

  if (useCategories && !this->categories().passes(categories))
  {
    return true;
  }

  // If the item is not enabled or if all of its values are set then it is valid
  // else it is enabled and contains unset values making it invalid
  if (!this->isEnabled())
  {
    return true;
  }

  // Are we using an expression?  Note that if we have an expression we
  // are not discrete and don't have children
  if (this->allowsExpressions() && (m_expression->value() != nullptr))
  {
    if (
      (!useCategories || !m_expression->isValid(categories)) &&
      (useCategories && !m_expression->isValid()))
    {
      return false;
    }

    if (this->expression()->canEvaluate())
    {
      std::unique_ptr<smtk::attribute::Evaluator> evaluator = this->expression()->createEvaluator();
      if (evaluator)
      {
        return evaluator->doesEvaluate();
      }
      else
      {
        return false;
      }
    }

    return true;
  }

  for (std::size_t i = 0; i < m_isSet.size(); ++i)
  {
    if (!m_isSet[i])
    {
      return false;
    }
  }
  // Now we need to check the active items
  for (auto it = m_activeChildrenItems.begin(); it != m_activeChildrenItems.end(); ++it)
  {
    if (useCategories)
    {
      if (!(*it)->isValid(categories))
      {
        return false;
      }
    }
    else
    {
      if (!(*it)->isValid(false))
      {
        return false;
      }
    }
  }
  return true;
}

bool ValueItem::hasDefault() const
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (!def)
  {
    return false;
  }
  return def->hasDefault();
}

bool ValueItem::isDiscreteIndexValid(int value) const
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (!def)
  {
    return false;
  }
  return def->isDiscreteIndexValid(value);
}

std::string ValueItem::valueLabel(std::size_t element) const
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (!def)
  {
    return "";
  }
  return def->valueLabel(element);
}

bool ValueItem::isExtensible() const
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (!def)
  {
    return false;
  }
  return def->isExtensible();
}

std::size_t ValueItem::numberOfRequiredValues() const
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (!def)
  {
    return 0;
  }
  return def->numberOfRequiredValues();
}

std::size_t ValueItem::maxNumberOfValues() const
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (!def)
  {
    return 0;
  }
  return def->maxNumberOfValues();
}

bool ValueItem::allowsExpressions() const
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (!def)
  {
    return false;
  }
  return def->allowsExpressions();
}

bool ValueItem::setExpression(smtk::attribute::AttributePtr exp)
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (def->allowsExpressions())
  {
    if (!exp)
    {
      m_expression->unset();
      return true;
    }
    if (def->isValidExpression(exp))
    {
      m_expression->setValue(exp);
      return true;
    }
  }
  return false;
}

void ValueItem::visitChildren(std::function<void(ItemPtr, bool)> visitor, bool activeChildren)
{
  if (activeChildren)
  {
    for (std::size_t index = 0; index < this->numberOfActiveChildrenItems(); index++)
    {
      visitor(this->activeChildItem(static_cast<int>(index)), activeChildren);
    }
  }
  else
  {
    std::map<std::string, smtk::attribute::ItemPtr>::const_iterator iter;
    const std::map<std::string, smtk::attribute::ItemPtr>& childrenItems = this->childrenItems();
    for (iter = childrenItems.begin(); iter != childrenItems.end(); iter++)
    {
      visitor(iter->second, activeChildren);
    }
  }
}

bool ValueItem::isDiscrete() const
{
  return static_cast<const ValueItemDefinition*>(m_definition.get())->isDiscrete();
}

void ValueItem::detachOwningResource()
{
  std::map<std::string, smtk::attribute::ItemPtr>::const_iterator iter;
  const std::map<std::string, smtk::attribute::ItemPtr>& childrenItems = this->childrenItems();
  for (iter = childrenItems.begin(); iter != childrenItems.end(); iter++)
  {
    iter->second->detachOwningResource();
  }
}

void ValueItem::reset()
{
  Item::reset();
}

bool ValueItem::rotate(std::size_t fromPosition, std::size_t toPosition)
{
  // We can't rotate an expression
  if (this->isExpression())
  {
    return false;
  }
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (!def)
  {
    return false;
  }

  if (!this->rotateVector(m_isSet, fromPosition, toPosition))
  {
    // There is a problem with the rotation fromPosition/toPosition info
    return false;
  }
  if (def->isDiscrete())
  {
    this->rotateVector(m_discreteIndices, fromPosition, toPosition);
  }

  return true;
}

bool ValueItem::setDiscreteIndex(std::size_t element, int index)
{
  if (!this->isDiscrete())
  {
    return false;
  }
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (def->isDiscreteIndexValid(index))
  {
    // Is this a different value then what is set already?
    assert(m_isSet.size() > element);
    assert(m_discreteIndices.size() > element);
    if (m_isSet[element] && (m_discreteIndices[element] == index))
    {
      // Nothing is changed
      return true;
    }
    m_discreteIndices[element] = index;
    if (def->allowsExpressions())
    {
      m_expression->unset();
    }
    m_isSet[element] = true;
    this->updateDiscreteValue(element);
    this->updateActiveChildrenItems();
    return true;
  }
  return false;
}

void ValueItem::updateActiveChildrenItems()
{
  // This is only for Discrete Value Items
  if (!this->isDiscrete())
  {
    return;
  }

  // Clear the current list of active children items
  m_activeChildrenItems.clear();

  // Note that for the current implementation only value items with 1
  // required value is support for conditional children.
  // Check to see if the index is valid
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  assert(!m_discreteIndices.empty());
  if (!def->isDiscreteIndexValid(m_discreteIndices[0]))
  {
    return;
  }

  // Get the children that should be active for the current value
  const std::string& v = def->discreteEnum(static_cast<size_t>(m_discreteIndices[0]));
  std::vector<std::string> citems = def->conditionalItems(v);
  std::size_t i, n = citems.size();
  for (i = 0; i < n; i++)
  {
    m_activeChildrenItems.push_back(m_childrenItems[citems[i]]);
  }
}

bool ValueItem::assign(ConstItemPtr& sourceItem, unsigned int options)
{
  // Assigns my contents to be same as sourceItem
  // Cast input pointer to ValueItem
  smtk::shared_ptr<const ValueItem> sourceValueItem =
    smtk::dynamic_pointer_cast<const ValueItem>(sourceItem);

  if (!sourceValueItem)
  {
    return false; //Source is not a value item!
  }

  // Get reference to attribute resource
  ResourcePtr resource = this->attribute()->attributeResource();

  this->setNumberOfValues(sourceValueItem->numberOfValues());

  // Are we dealing with Expressions?
  if (sourceValueItem->isExpression())
  {
    if (options & Item::IGNORE_EXPRESSIONS)
    {
      // OK we are not to copy the expression so instead
      // we need to clear all of the values
      this->reset();
    }
    else
    {
      std::string nameStr = sourceValueItem->expression()->name();
      AttributePtr att = resource->findAttribute(nameStr);
      if (!att)
      {
        att = resource->copyAttribute(
          sourceValueItem->expression(), (options & Item::COPY_MODEL_ASSOCIATIONS) != 0, options);
        if (att == nullptr)
        {
          std::cerr << "ERROR: Could not copy Attribute:" << sourceValueItem->expression()->name()
                    << " used as an expression by item: " << sourceItem->name() << "\n";
          return false; // Something went wrong!
        }
      }
      this->setExpression(att);
    }
  }
  else
  {
    // Update values for discrete values - note that the derived classes
    // will take care of the non-discrete values.
    for (std::size_t i = 0; i < sourceValueItem->numberOfValues(); ++i)
    {
      if (!sourceValueItem->isSet(i))
      {
        this->unset(i);
      }
      else if (sourceValueItem->isDiscrete())
      {
        this->setDiscreteIndex(i, sourceValueItem->discreteIndex(i));
      }
    } // for
  }

  // Update children items
  std::map<std::string, smtk::attribute::ItemPtr>::const_iterator sourceIter =
    sourceValueItem->m_childrenItems.begin();
  std::map<std::string, smtk::attribute::ItemPtr>::const_iterator newIter;
  for (; sourceIter != sourceValueItem->m_childrenItems.end(); sourceIter++)
  {
    ConstItemPtr sourceChild = smtk::const_pointer_cast<const Item>(sourceIter->second);
    newIter = m_childrenItems.find(sourceIter->first);
    if (newIter == m_childrenItems.end())
    {
      std::cerr << "ERROR:Could not find child item \"" << sourceIter->first << "\" -- cannot copy"
                << std::endl;
      continue;
    }
    ItemPtr newChild = newIter->second;
    if (!newChild->assign(sourceChild, options))
    {
      std::cerr << "ERROR:Could not properly assign child item: " << newChild->name() << "\n";
      return false;
    }
  }
  return Item::assign(sourceItem, options);
}

smtk::attribute::ItemPtr ValueItem::findInternal(const std::string& childName, SearchStyle style)
{
  // Do we have it among our children?

  // Are we only caring about active children?
  if ((style == RECURSIVE_ACTIVE) || (style == IMMEDIATE_ACTIVE))
  {
    for (auto& item : m_activeChildrenItems)
    {
      if (item->name() == childName)
      {
        return item;
      }
    }
    if (style == RECURSIVE_ACTIVE)
    {
      // Ok - we didn't find it so lets recursively check its active chiildren
      for (auto& item : m_activeChildrenItems)
      {
        ItemPtr result = item->find(childName, style);
        if (result)
        {
          return result;
        }
      }
    }
    // Couldn't find anything
    return nullptr;
  }

  // Ok lets see if we can find the name in the item's children
  auto it = m_childrenItems.find(childName);
  if (it != m_childrenItems.end())
  {
    return it->second;
  }

  if (style == IMMEDIATE)
  {
    // We are not suppose to recursively look for a match
    return nullptr;
  }

  for (auto& child : m_childrenItems)
  {
    ItemPtr result = child.second->find(childName, style);
    if (result)
    {
      return result;
    }
  }
  return nullptr;
}

smtk::attribute::ConstItemPtr ValueItem::findInternal(
  const std::string& childName,
  SearchStyle style) const
{
  // Do we have it among our children?

  // Are we only caring about active children?
  if ((style == RECURSIVE_ACTIVE) || (style == IMMEDIATE_ACTIVE))
  {
    for (auto& item : m_activeChildrenItems)
    {
      if (item->name() == childName)
      {
        return item;
      }
    }
    if (style == RECURSIVE_ACTIVE)
    {
      // Ok - we didn't find it so lets recursively check its active chiildren
      for (auto& item : m_activeChildrenItems)
      {
        ConstItemPtr result = item->find(childName, style);
        if (result)
        {
          return result;
        }
      }
    }
    // Couldn't find anything
    return nullptr;
  }

  // Ok lets see if we can find the name in the item's children
  auto it = m_childrenItems.find(childName);
  if (it != m_childrenItems.end())
  {
    return it->second;
  }

  if (style == IMMEDIATE)
  {
    // We are not suppose to recursively look for a match
    return nullptr;
  }

  for (auto& child : m_childrenItems)
  {
    ConstItemPtr result = child.second->find(childName, style);
    if (result)
    {
      return result;
    }
  }
  return nullptr;
}

/**\brief Find a child of this item with the given name - To Be Deprecated (please use find!)
  *
  * If the \a style is ALL_CHILDREN or ACTIVE_CHILDREN,
  * any of *this* item's children that are ValueItems
  * will be asked to search their children recursively.
  */
smtk::attribute::ItemPtr ValueItem::findChild(const std::string& cname, SearchStyle style)
{
  return this->findInternal(cname, style);
}

smtk::attribute::ConstItemPtr ValueItem::findChild(const std::string& cname, SearchStyle style)
  const
{
  return this->findInternal(cname, style);
}
