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
      int i;
      m_expressions.resize(n);
      for (i = 0; i < static_cast<int>(n); i++)
      {
        def->buildExpressionItem(this, i);
      }
    }
  }

  return true;
}

ValueItem::~ValueItem()
{
  // we need to detach all items that are owned by this. i.e. the expression items
  std::size_t i, n = m_expressions.size();
  for (i = 0; i < n; i++)
  {
    m_expressions[i]->detachOwningItem();
  }
}

void ValueItem::unset(std::size_t elementIndex)
{
  assert(m_expressions.size() > elementIndex);
  m_isSet[elementIndex] = false;
  // Clear the current list of active children items
  m_activeChildrenItems.clear();
}

bool ValueItem::isValid(const std::set<std::string>& cats) const
{
  // If we have been given categories we need to see if the item passes its
  // category checks - if it doesn't it means its not be taken into account
  // for validity checking so just return true

  if (cats.size() && !this->passCategoryCheck(cats))
  {
    return true;
  }

  // If the item is not enabled or if all of its values are set then it is valid
  // else it is enabled and contains unset values making it invalid
  if (!this->isEnabled())
  {
    return true;
  }
  assert(!this->allowsExpressions() || m_expressions.size() >= m_isSet.size());
  for (std::size_t i = 0; i < m_isSet.size(); ++i)
  {
    if (!m_isSet[i])
    {
      return false;
    }
    // Is this using an expression?
    if (this->allowsExpressions() && (m_expressions[i]->value() != nullptr))
    {
      if (!m_expressions[i]->isValid(cats))
      {
        return false;
      }
    }
  }
  // Now we need to check the active items
  for (auto it = m_activeChildrenItems.begin(); it != m_activeChildrenItems.end(); ++it)
  {
    if (!(*it)->isValid(cats))
    {
      return false;
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

smtk::attribute::AttributePtr ValueItem::expression(std::size_t element) const
{
  assert(m_isSet.size() > element);
  if (m_isSet[element])
  {
    const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
    if (def->allowsExpressions())
    {
      assert(m_expressions.size() > element);
      return dynamic_pointer_cast<smtk::attribute::Attribute>(m_expressions[element]->value());
    }
  }
  return smtk::attribute::AttributePtr();
}

bool ValueItem::setExpression(std::size_t element, smtk::attribute::AttributePtr exp)
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (def->allowsExpressions())
  {
    if (!exp)
    {
      assert(m_expressions.size() > element);
      if (m_expressions[element]->value())
      {
        assert(m_isSet.size() > element);
        m_isSet[element] = false;
        m_expressions[element]->unset();
      }
      return true;
    }
    if (def->isValidExpression(exp))
    {
      assert(m_isSet.size() > element);
      m_isSet[element] = true;
      assert(m_expressions.size() > element);
      m_expressions[element]->setValue(exp);
      return true;
    }
  }
  return false;
}

bool ValueItem::appendExpression(smtk::attribute::AttributePtr exp)
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  size_t n = m_expressions.size(), maxN = def->maxNumberOfValues();

  if (!def->allowsExpressions())
  {
    return false;
  }
  if (!def->isExtensible())
  {
    return false; // The number of values is fixed
  }
  if (maxN && (n >= maxN))
  {
    return false; // max number reached
  }

  if (!def->isValidExpression(exp))
  {
    return false; // Attribute is of the proper type
  }
  m_expressions.resize(n + 1);
  def->buildExpressionItem(this, static_cast<int>(n));
  m_expressions[n]->setValue(exp);
  m_isSet.push_back(true);
  return true;
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
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (!def)
  {
    return false;
  }

  this->rotateVector(m_isSet, fromPosition, toPosition);
  if (def->isDiscrete())
  {
    this->rotateVector(m_discreteIndices, fromPosition, toPosition);
  }
  if (def->allowsExpressions())
  {
    this->rotateVector(m_expressions, fromPosition, toPosition);
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
      assert(m_expressions.size() > element);
      m_expressions[element]->unset();
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
  std::string v = def->discreteEnum(static_cast<size_t>(m_discreteIndices[0]));
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

  this->setNumberOfValues(sourceValueItem->numberOfValues());

  // Get reference to attribute resource
  ResourcePtr resource = this->attribute()->attributeResource();

  // Update values
  for (std::size_t i = 0; i < sourceValueItem->numberOfValues(); ++i)
  {
    if (!sourceValueItem->isSet(i))
    {
      this->unset(i);
    }
    else if (sourceValueItem->isExpression(i))
    {
      // Are we copying expressions?
      if (options & Item::IGNORE_EXPRESSIONS)
      {
        this->unset(i);
      }
      else
      {
        std::string nameStr = sourceValueItem->expression(i)->name();
        AttributePtr att = resource->findAttribute(nameStr);
        if (!att)
        {
          att = resource->copyAttribute(sourceValueItem->expression(i),
            (options & Item::COPY_MODEL_ASSOCIATIONS) != 0, options);
          if (att == nullptr)
          {
            std::cerr << "ERROR: Could not copy Attribute:"
                      << sourceValueItem->expression(i)->name()
                      << " used as an expression by item: " << sourceItem->name() << "\n";
            return false; // Something went wrong!
          }
        }
        this->setExpression(i, att);
      }
    }
    else if (sourceValueItem->isDiscrete())
    {
      this->setDiscreteIndex(i, sourceValueItem->discreteIndex(i));
    }
  } // for

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
  const std::string& childName, SearchStyle style) const
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
///@{
smtk::attribute::ItemPtr ValueItem::findChild(const std::string& cname, SearchStyle style)
{
  return this->findInternal(cname, style);
}

smtk::attribute::ConstItemPtr ValueItem::findChild(
  const std::string& cname, SearchStyle style) const
{
  return this->findInternal(cname, style);
}
///@}
