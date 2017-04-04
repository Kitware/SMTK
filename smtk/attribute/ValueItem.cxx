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
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include <algorithm> // for std::find
#include <iostream>

using namespace smtk::attribute;

ValueItem::ValueItem(Attribute *owningAttribute,
                     int itemPosition):
  Item(owningAttribute, itemPosition)
{
}

ValueItem::ValueItem(Item *inOwningItem,
                     int itemPosition,
                     int mySubGroupPosition):
  Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}

bool ValueItem::setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef)
{
   // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const ValueItemDefinition *def =
    dynamic_cast<const ValueItemDefinition *>(vdef.get());
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
      this->m_isSet.resize(n, true);
      }
    else
      {
      this->m_isSet.resize(n, false);
      }
    if (def->isDiscrete())
      {
      this->m_discreteIndices.resize(n, def->defaultDiscreteIndex());
      this->updateActiveChildrenItems();
      }
    if (def->allowsExpressions())
      {
      int i;
      this->m_expressions.resize(n);
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
  std::size_t i, n = this->m_expressions.size();
  for (i = 0; i < n; i++)
    {
    this->m_expressions[i]->detachOwningItem();
    }
}

bool ValueItem::isValid() const
{
  // If the item is not enabled or if all of its values are set then it is valid
  // else it is enabled and contains unset values making it invalid
  if (!this->isEnabled())
    {
    return true;
    }
  assert(!this->allowsExpressions() || this->m_expressions.size() >= this->m_isSet.size());
  for (std::size_t i = 0; i < this->m_isSet.size(); ++i)
    {
    if (!this->m_isSet[i])
      {
      return false;
      }
    // Is this using an expression?
    if (this->allowsExpressions() && (this->m_expressions[i]->value()!= nullptr))
      {
      if (!this->m_expressions[i]->isValid())
        {
         return false;
        }
      }
    }
  // Now we need to check the active items
    for (auto it = this->m_activeChildrenItems.begin();
         it != this->m_activeChildrenItems.end(); ++it)
    {
    if (!(*it)->isValid())
      {
       return false;
      }
    }
 return true;
}

bool ValueItem::hasDefault() const
{
  const ValueItemDefinition *def =
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return false;
    }
  return def->hasDefault();
}

bool ValueItem::isDiscreteIndexValid(int value) const
{
  const ValueItemDefinition *def =
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return false;
    }
  return def->isDiscreteIndexValid(value);
}

bool ValueItem::isExtensible() const
{
  const ValueItemDefinition *def =
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return false;
    }
  return def->isExtensible();
}

std::size_t ValueItem::numberOfRequiredValues() const
{
  const ValueItemDefinition *def =
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return 0;
    }
  return def->numberOfRequiredValues();
}

std::size_t ValueItem::maxNumberOfValues() const
{
  const ValueItemDefinition *def =
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return 0;
    }
  return def->maxNumberOfValues();
}

bool ValueItem::allowsExpressions() const
{
  const ValueItemDefinition *def =
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return false;
    }
  return def->allowsExpressions();
}

smtk::attribute::AttributePtr ValueItem::expression(std::size_t element) const
{
  assert(this->m_isSet.size() > element);
  if (this->m_isSet[element])
    {
    const ValueItemDefinition *def =
      static_cast<const ValueItemDefinition*>(this->m_definition.get());
    if (def->allowsExpressions())
      {
      assert(this->m_expressions.size() > element);
      return this->m_expressions[element]->value();
      }
    }
  return smtk::attribute::AttributePtr();
}

bool ValueItem::setExpression(std::size_t element,
                              smtk::attribute::AttributePtr exp)
{
  const ValueItemDefinition *def =
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (def->allowsExpressions())
    {
    if (!exp)
      {
      assert(this->m_expressions.size() > element);
      if (this->m_expressions[element]->value())
        {
        assert(this->m_isSet.size() > element);
        this->m_isSet[element] = false;
        this->m_expressions[element]->unset();
        }
      return true;
      }
    if (def->isValidExpression(exp))
      {
      assert(this->m_isSet.size() > element);
      this->m_isSet[element] = true;
      assert(this->m_expressions.size() > element);
      this->m_expressions[element]->setValue(exp);
      return true;
      }
    }
  return false;
}

bool ValueItem::appendExpression(smtk::attribute::AttributePtr exp)
{
  const ValueItemDefinition *def =
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
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
  this->m_expressions.resize(n+1);
  def->buildExpressionItem(this, static_cast<int>(n));
  this->m_expressions[n]->setValue(exp);
  this->m_isSet.push_back(true);
  return true;
}

bool ValueItem::isDiscrete() const
{
  return static_cast<const ValueItemDefinition*>(this->m_definition.get())->
    isDiscrete();
}

void ValueItem::reset()
{
  Item::reset();
}

bool ValueItem::setDiscreteIndex(std::size_t element, int index)
{
  if (!this->isDiscrete())
    {
    return false;
    }
  const ValueItemDefinition *def =
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (def->isDiscreteIndexValid(index))
    {
    // Is this a different value then what is set already?
    assert(this->m_isSet.size() > element);
    assert(this->m_discreteIndices.size() > element);
    if (this->m_isSet[element] && (this->m_discreteIndices[element] == index))
      {
      // Nothing is changed
      return true;
      }
    this->m_discreteIndices[element] = index;
    if (def->allowsExpressions())
      {
      assert(this->m_expressions.size() > element);
      this->m_expressions[element]->unset();
      }
    this->m_isSet[element] = true;
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
  this->m_activeChildrenItems.clear();

  // Note that for the current implementation only value items with 1
  // required value is support for conditional children.
  // Check to see if the index is valid
  const ValueItemDefinition *def =
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  assert(!this->m_discreteIndices.empty());
  if (!def->isDiscreteIndexValid(this->m_discreteIndices[0]))
    {
    return;
    }

  // Get the children that should be active for the current value
  std::string v =
     def->discreteEnum(static_cast<size_t>(this->m_discreteIndices[0]));
  std::vector<std::string> citems = def->conditionalItems(v);
  std::size_t i, n = citems.size();
  for (i = 0; i < n; i++)
    {
    this->m_activeChildrenItems.push_back(this->m_childrenItems[citems[i]]);
    }
}

bool ValueItem::assign(ConstItemPtr &sourceItem, unsigned int options)
{
  // Assigns my contents to be same as sourceItem
  // Cast input pointer to ValueItem
  smtk::shared_ptr<const ValueItem > sourceValueItem =
    smtk::dynamic_pointer_cast<const ValueItem>(sourceItem);

  if (!sourceValueItem)
    {
    return false; //Source is not a value item!
    }

  this->setNumberOfValues(sourceValueItem->numberOfValues());

  // Get reference to attribute system
  System *system = this->attribute()->system();

  // Update values
  for (std::size_t i=0; i<sourceValueItem->numberOfValues(); ++i)
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
        AttributePtr att = system->findAttribute(nameStr);
        if (!att)
          {
          att = system->copyAttribute(sourceValueItem->expression(i),
                                      (options & Item::COPY_MODEL_ASSOCIATIONS) != 0,
                                      options);
          if (att == nullptr)
            {
            std::cerr << "ERROR: Could not copy Attribute:"
                      << sourceValueItem->expression(i)->name()
                      << " used as an expression by item: "
                      << sourceItem->name() << "\n";
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
      std::cerr << "ERROR:Could not find child item \"" << sourceIter->first
                << "\" -- cannot copy" << std::endl;
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

/**\brief Find a child of this item with the given name.
  *
  * If the \a style is ALL_CHILDREN or ACTIVE_CHILDREN,
  * any of *this* item's children that are ValueItems
  * will be asked to search their children recursively.
  */
///@{
smtk::attribute::ItemPtr ValueItem::findChild(
  const std::string& cname,
  SearchStyle style)
{
  std::vector<smtk::attribute::ItemPtr>::const_iterator ait;
  std::map<std::string, smtk::attribute::ItemPtr>::const_iterator it;

  // First, ask if we have a match at all.
  it = this->m_childrenItems.find(cname);
  if (it != this->m_childrenItems.end())
    { // Now, if we have a match, see if it is active should that be required.
    if (style == ACTIVE_CHILDREN)
      {
      ait =
        std::find(
          this->m_activeChildrenItems.begin(), this->m_activeChildrenItems.end(),
          it->second);
      if (ait != this->m_activeChildrenItems.end())
        return it->second; // Our match is active, return it.
      }
    else
      { // We have a match and do not need it to be active.
      return it->second;
      }
    }

  // None of our children match, but perhaps they have children that do.
  switch (style)
    {
  case ACTIVE_CHILDREN:
    for (ait = this->m_activeChildrenItems.begin(); ait != this->m_activeChildrenItems.end(); ++ait)
      {
      ValueItem::Ptr vchild = dynamic_pointer_cast<ValueItem>(*ait);
      if (vchild)
        {
        ItemPtr match = vchild->findChild(cname, style);
        if (match)
          return match;
        }
      }
    break;
  case ALL_CHILDREN:
    for (it = this->m_childrenItems.begin(); it != this->m_childrenItems.end(); ++it)
      {
      ValueItem::Ptr vchild = dynamic_pointer_cast<ValueItem>(it->second);
      if (vchild)
        {
        ItemPtr match = vchild->findChild(cname, style);
        if (match)
          return match;
        }
      }
    break;
  case NO_CHILDREN:
  default:
    break;
    }
  return ItemPtr();
}

smtk::attribute::ConstItemPtr ValueItem::findChild(
  const std::string& cname,
  SearchStyle style) const
{
  std::vector<smtk::attribute::ItemPtr>::const_iterator ait;
  std::map<std::string, smtk::attribute::ItemPtr>::const_iterator it;

  // First, ask if we have a match at all.
  it = this->m_childrenItems.find(cname);
  if (it != this->m_childrenItems.end())
    { // Now, if we have a match, see if it is active should that be required.
    if (style == ACTIVE_CHILDREN)
      {
      ait =
        std::find(
          this->m_activeChildrenItems.begin(),
          this->m_activeChildrenItems.end(),
          it->second);
      if (ait != this->m_activeChildrenItems.end())
        return it->second; // Our match is active, return it.
      }
    else
      { // We have a match and do not need it to be active.
      return it->second;
      }
    }

  // None of our children match, but perhaps they have children that do.
  switch (style)
    {
  case ACTIVE_CHILDREN:
    for (ait = this->m_activeChildrenItems.begin(); ait != this->m_activeChildrenItems.end(); ++ait)
      {
      ConstValueItemPtr vchild = dynamic_pointer_cast<const ValueItem>(*ait);
      if (vchild)
        {
        ConstItemPtr match = vchild->findChild(cname, style);
        if (match)
          return match;
        }
      }
    break;
  case ALL_CHILDREN:
    for (it = this->m_childrenItems.begin(); it != this->m_childrenItems.end(); ++it)
      {
      ConstValueItemPtr vchild = dynamic_pointer_cast<const ValueItem>(it->second);
      if (vchild)
        {
        ConstItemPtr match = vchild->findChild(cname, style);
        if (match)
          return match;
        }
      }
    break;
  case NO_CHILDREN:
  default:
    break;
    }
  return ConstItemPtr();
}
///@}
