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

#include "units/Converter.h"
#include "units/System.h"

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
    if (!this->initializeValues())
    {
      return false;
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

  // Let check to make sure all of the values are set.  In the case of discrete values
  // we need to also make sure values are still valid since enumerations can depend on
  // the set of categories enabled and possibly a custom isEnumRelevant function

  if (!this->isDiscrete()) // just need to test to make sure all values are set
  {
    for (std::size_t i = 0; i < m_isSet.size(); ++i)
    {
      if (!m_isSet[i])
      {
        return false;
      }
    }
  }
  else
  {
    const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
    if (!def)
    {
      return false;
    }

    // Is there a custom function to determine if an enum is relevant?
    auto customIsEnumRelevant = def->customEnumIsRelevant();
    if (customIsEnumRelevant)
    {
      for (std::size_t i = 0; i < m_isSet.size(); ++i)
      {
        if (!(m_isSet[i] &&
              customIsEnumRelevant(
                this, m_discreteIndices[i], useCategories, categories, false, 0)))
        {
          return false;
        }
      }
    }
    else if (useCategories)
    {
      for (std::size_t i = 0; i < m_isSet.size(); ++i)
      {
        if (!(m_isSet[i] && def->isDiscreteIndexValid(m_discreteIndices[i], categories)))
        {
          return false;
        }
      }
    }
    else // all enum values are valid - just need to make sure all values are set
    {
      for (std::size_t i = 0; i < m_isSet.size(); ++i)
      {
        if (!m_isSet[i])
        {
          return false;
        }
      }
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

bool ValueItem::isAcceptable(const smtk::attribute::AttributePtr& exp) const
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  if (!def->allowsExpressions())
  {
    return false;
  }

  if (!exp)
  {
    return true;
  }

  if (!def->isValidExpression(exp))
  {
    return false;
  }

  if (this->units() == exp->units())
  {
    return true;
  }

  // Are the units (if any) compatible?
  if (exp->units().empty())
  {
    return true;
  }

  if (this->units().empty())
  {
    return false;
  }

  const auto& unitSys = def->unitsSystem();
  // Can't determine if the units are compatible w/o units system
  if (!unitSys)
  {
    return false;
  }

  bool unitsSupported;

  // Are the item units supported by the units system
  auto iUnits = unitSys->unit(this->units(), &unitsSupported);
  if (!unitsSupported)
  {
    // the item's units are not supported by the units system
    return false;
  }

  // Are the expression units supported by the units system
  auto eUnits = unitSys->unit(exp->units(), &unitsSupported);
  if (!unitsSupported)
  {
    // the expression's units are not supported by the units system
    return false;
  }

  return (unitSys->convert(iUnits, eUnits) != nullptr);
}

bool ValueItem::setExpression(smtk::attribute::AttributePtr exp)
{
  if (!this->isAcceptable(exp))
  {
    return false;
  }

  if (!exp)
  {
    m_expression->unset();
  }
  else
  {
    m_expression->setValue(exp);
  }
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

Item::Status ValueItem::assign(
  const smtk::attribute::ConstItemPtr& sourceItem,
  const CopyAssignmentOptions& options,
  smtk::io::Logger& logger)
{
  Item::Status result;
  // Assigns my contents to be same as sourceItem
  // Cast input pointer to ValueItem
  smtk::shared_ptr<const ValueItem> sourceValueItem =
    smtk::dynamic_pointer_cast<const ValueItem>(sourceItem);

  if (!sourceValueItem)
  {
    result.markFailed();
    smtkErrorMacro(logger, "Source Item: " << name() << " is not a ValueItem");
    return result; //Source is not a value item!
  }

  // Get reference to attribute resource
  ResourcePtr resource = this->attribute()->attributeResource();

  bool status;
  if (this->numberOfValues() != sourceValueItem->numberOfValues())
  {
    status = this->setNumberOfValues(sourceValueItem->numberOfValues());
    if (status)
    {
      result.markModified();
    }
  }

  // Were we able to allocate enough space to fit all of the source's values?
  std::size_t myNumVals, sourceNumVals, numVals;
  myNumVals = this->numberOfValues();
  sourceNumVals = sourceValueItem->numberOfValues();
  if (myNumVals < sourceNumVals)
  {
    // Ok so the source has more values than we can deal with - was partial copying permitted?
    if (options.itemOptions.allowPartialValues())
    {
      numVals = myNumVals;
      smtkInfoMacro(
        logger,
        "ValueItem: " << this->name() << "'s number of values (" << myNumVals
                      << ") is smaller than source Item's number of values (" << sourceNumVals
                      << ") - will partially copy the values");
    }
    else
    {
      result.markFailed();
      smtkErrorMacro(
        logger,
        "ValueItem: " << name() << "'s number of values (" << myNumVals
                      << ") can not hold source ValueItem's number of values (" << sourceNumVals
                      << ") and Partial Copying was not permitted");
      return result;
    }
  }
  else
  {
    numVals = sourceNumVals;
  }

  // Are we dealing with Expressions?
  if (sourceValueItem->isExpression())
  {
    if (options.itemOptions.ignoreExpressions())
    {
      // OK we are not to copy the expression so instead
      // we need to clear all of the values
      this->reset();
      result.markModified();
    }
    else
    {
      // Do we have the expression in the options' mapping information?
      Attribute* rawDestExpAtt = options.itemOptions.targetObjectFromSourceId<Attribute>(
        sourceValueItem->expression()->id());
      if (rawDestExpAtt)
      {
        status = this->setExpression(rawDestExpAtt->shared_from_this());
        if (!status)
        {
          if (options.itemOptions.allowPartialValues())
          {
            smtkInfoMacro(
              logger,
              "Could not assign Expression:" << rawDestExpAtt->name()
                                             << " to ValueItem: " << sourceItem->name());
            this->setExpression(nullptr);
            result.markModified();
          }
          else
          {
            result.markFailed();
            smtkErrorMacro(
              logger,
              "Could not assign Expression:"
                << rawDestExpAtt->name() << " to ValueItem: " << sourceItem->name()
                << " and allowPartialValues options was not specified.");
            return result;
          }
        }
      }
      // If the expression is contained in the same resource as the item, then find it or create a copy of it
      else if (
        sourceValueItem->attribute()->resource() == sourceValueItem->expression()->resource())
      {
        std::string nameStr = sourceValueItem->expression()->name();
        AttributePtr att = resource->findAttribute(nameStr);
        if (!att)
        {
          // Are we allowed to create new attributes?
          if (!options.itemOptions.disableCopyAttributes())
          {
            att = resource->copyAttribute(sourceValueItem->expression(), options, logger);
            if (att == nullptr)
            {
              result.markFailed();
              smtkErrorMacro(
                logger,
                "Could not copy Attribute:" << sourceValueItem->expression()->name()
                                            << " used as an expression by item: "
                                            << sourceItem->name());
              return result;
            }
          }
          else
          {
            smtkWarningMacro(
              logger,
              "Could not assign Attribute:" << sourceValueItem->expression()->name()
                                            << " used as an expression by item: "
                                            << sourceItem->name()
                                            << " because it is in the same resource as the source "
                                               "item and disableCopyAttributes option was set");
          }
          status = this->setExpression(att);
          if (!status)
          {
            if (options.itemOptions.allowPartialValues())
            {
              smtkInfoMacro(
                logger,
                "Could not assign Expression:" << att->name()
                                               << " to ValueItem: " << sourceItem->name());
              this->setExpression(nullptr);
              result.markModified();
            }
            else
            {
              result.markFailed();
              smtkErrorMacro(
                logger,
                "Could not assign Expression:"
                  << att->name() << " to ValueItem: " << sourceItem->name()
                  << " and allowPartialValues options was not specified.");
              return result;
            }
          }
          if (status)
          {
            result.markModified();
          }
        }
      }
      else
      {
        // The source expression is located in a different resource than the source Item's attribute
        // so simply assign it
        status = this->setExpression(sourceValueItem->expression());
        if (!status)
        {
          if (options.itemOptions.allowPartialValues())
          {
            smtkInfoMacro(
              logger,
              "Could not assign Expression:" << sourceValueItem->expression()->name()
                                             << " to ValueItem: " << sourceItem->name());
            this->setExpression(nullptr);
            result.markModified();
          }
          else
          {
            result.markFailed();
            smtkErrorMacro(
              logger,
              "Could not assign Expression:"
                << sourceValueItem->expression()->name() << " to ValueItem: " << sourceItem->name()
                << " and allowPartialValues options was not specified.");
            return result;
          }
        }
        if (status)
        {
          result.markModified();
        }
      }
    }
  }
  else
  {
    // Update values for discrete values - note that the derived classes
    // will take care of the non-discrete values.
    for (std::size_t i = 0; i < numVals; ++i)
    {
      if (!sourceValueItem->isSet(i))
      {
        result.markModified();
        this->unset(i);
      }
      else if (sourceValueItem->isDiscrete())
      {
        status = this->setDiscreteIndex(i, sourceValueItem->discreteIndex(i));
        if (!status)
        {
          if (options.itemOptions.allowPartialValues())
          {
            smtkInfoMacro(
              logger,
              "Could not assign Discrete Index:" << sourceValueItem->discreteIndex(i)
                                                 << " to ValueItem: " << sourceItem->name());
            this->unset(i);
            result.markModified();
          }
          else
          {
            result.markFailed();
            smtkErrorMacro(
              logger,
              "Could not assign Discrete Index:"
                << sourceValueItem->discreteIndex(i) << " to ValueItem: " << sourceItem->name()
                << " and allowPartialValues options was not specified.");
            return result;
          }
        }
        if (status)
        {
          result.markModified();
        }
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
      // Are missing items allowed?
      if (!options.itemOptions.ignoreMissingChildren())
      {
        result.markFailed();
        smtkErrorMacro(
          logger,
          "Could not find Child Item: " << sourceIter->first << " in ValueItem: " << this->name()
                                        << " and ignoreMissingChildren option was not set");
        return result;
      }
      continue;
    }
    ItemPtr newChild = newIter->second;
    auto childResult = newChild->assign(sourceChild, options);
    result &= childResult;
    if (!childResult.success())
    {
      smtkErrorMacro(
        logger,
        "Could not assign ValueItem: " << this->name() << "'s Child Item: " << newChild->name());
      return result;
    }
  }
  result &= Item::assign(sourceItem, options, logger);
  return result;
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
    for (const auto& item : m_activeChildrenItems)
    {
      if (item->name() == childName)
      {
        return item;
      }
    }
    if (style == RECURSIVE_ACTIVE)
    {
      // Ok - we didn't find it so lets recursively check its active chiildren
      for (const auto& item : m_activeChildrenItems)
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

  for (const auto& child : m_childrenItems)
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

std::vector<std::string> ValueItem::relevantEnums(
  bool includeCategories,
  bool includeReadAccess,
  unsigned int readAccessLevel) const
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  std::set<std::string> activeCategories;
  bool useCategories = false;

  if (includeCategories)
  {
    // See if we can get the active categories of the related Resource, else ignore categories
    auto myAttribute = this->attribute();
    if (myAttribute)
    {
      auto aResource = myAttribute->attributeResource();
      if (aResource && aResource->activeCategoriesEnabled())
      {
        useCategories = true;
        activeCategories = aResource->activeCategories();
      }
    }
  }

  // Is there a custom enum relevant function?
  auto customIsEnumRelevant = def->customEnumIsRelevant();

  if (!customIsEnumRelevant)
  {
    return def->relevantEnums(useCategories, activeCategories, includeReadAccess, readAccessLevel);
  }

  std::vector<std::string> result;
  std::size_t i, n = def->numberOfDiscreteValues();
  for (i = 0; i < n; i++)
  {
    if (customIsEnumRelevant(
          this, i, useCategories, activeCategories, includeReadAccess, readAccessLevel))
    {
      result.push_back(def->discreteEnum(i));
    }
  }
  return result;
}

const std::string& ValueItem::units() const
{
  const ValueItemDefinition* def = static_cast<const ValueItemDefinition*>(m_definition.get());
  return def->units();
}

std::string ValueItem::supportedUnits() const
{
  bool status = false;
  auto munits = this->units();
  auto unitsSystem = m_definition->unitsSystem();

  if (!(munits.empty() || (unitsSystem == nullptr)))
  {
    auto myUnit = unitsSystem->unit(munits, &status);
  }
  return (status ? munits : std::string());
}
