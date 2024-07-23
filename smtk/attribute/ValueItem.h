//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_ValueItem_h
#define smtk_attribute_ValueItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/SearchStyle.h"

#include <cassert>

namespace smtk
{
namespace attribute
{
class Attribute;
class ComponentItem;
class ValueItemDefinition;

/**\brief A concrete base class for items that store a vector of plain-old-data (POD) as values.
  *
  * The `ValueItemTemplate<T>` template inherits this class; it in turn is inherited by
  * StringItem, IntItem, and DoubleItem.
  *
  * Besides POD, this class adds the concepts of items that can take on values
  * only from a discrete set and expressions that can be evaluated to produce
  * values rather than storing only raw values.
  */
class SMTKCORE_EXPORT ValueItem : public smtk::attribute::Item
{
public:
  smtkTypeMacro(smtk::attribute::ValueItem);
  friend class ValueItemDefinition;

  ~ValueItem() override;
  //TODO - Currently expressions should only be set on non-extensible items
  // once evaluation is possible, this constraint can be relaxed and this
  // method should be expanded to take this into consideration
  virtual std::size_t numberOfValues() const { return m_isSet.size(); }

  std::size_t numberOfRequiredValues() const;
  std::size_t maxNumberOfValues() const;

  std::string valueLabel(std::size_t element) const;

  bool isExtensible() const;

  bool allowsExpressions() const;
  bool isExpression() const;

  smtk::attribute::AttributePtr expression() const;
  bool setExpression(smtk::attribute::AttributePtr exp);

  ///\brief Returns true if the expression would be acceptable based om the item's
  /// definition and units requirements
  bool isAcceptable(const smtk::attribute::AttributePtr& exp) const;

  virtual bool setNumberOfValues(std::size_t newSize) = 0;
  /**
   * @brief visitChildren Invoke a function on each (or, if \a findInActiveChildren
   * is true, each active) child item. If a subclass presents children items(ValueItem,
   * Group, ComponentItem, ...) then this function should be overridden.
   * @param visitor a lambda function which would be applied on children items
   * @param activeChildren a flag indicating whether it should be applied to active children only or not
   */
  void visitChildren(
    std::function<void(smtk::attribute::ItemPtr, bool)> visitor,
    bool activeChildren = true) override;

  int discreteIndex(std::size_t elementIndex = 0) const
  {
    assert(m_discreteIndices.size() > elementIndex);
    return m_discreteIndices[elementIndex];
  }
  bool isDiscrete() const;
  bool isDiscreteIndexValid(int value) const;
  bool setDiscreteIndex(int value) { return this->setDiscreteIndex(0, value); }
  // Returns true if value is a valid index - else it returns false
  bool setDiscreteIndex(std::size_t elementIndex, int value);

  ///\brief Return the enum strings that pass the Resource's active categories and/or specified advance read access level.
  std::vector<std::string>
  relevantEnums(bool includeCategories, bool includeReadAccess, unsigned int readAccessLevel) const;

  /// Release the item's dependency on its parent attribute's Resource.
  void detachOwningResource() override;
  // Reset returns the item to its initial state.
  //If the item is of fixed size, then it's values  to their initial state.
  // If there is a default available it will use it, else
  // it will be marked as unset.
  //If the item's definition indicated a size of 0 then it will go back to
  // having no values
  void reset() override;

  /// Rotate the order between specified positions.
  bool rotate(std::size_t fromPosition, std::size_t toPosition) override;

  virtual bool setToDefault(std::size_t elementIndex = 0) = 0;
  // Returns true if there is a default defined and the item is curently set to it
  virtual bool isUsingDefault(std::size_t elementIndex) const = 0;
  // This method tests all of the values of the items w/r the default value
  virtual bool isUsingDefault() const = 0;
  // Does this item have a default value?
  bool hasDefault() const;
  virtual std::string valueAsString() const { return this->valueAsString(0); }

  virtual std::string valueAsString(std::size_t elementIndex) const = 0;
  virtual bool setValueFromString(const std::string& stringVal)
  {
    return this->setValueFromString(0, stringVal);
  }
  virtual bool setValueFromString(std::size_t elementIndex, const std::string& stringVal) = 0;
  virtual bool isSet(std::size_t elementIndex = 0) const;
  virtual void unset(std::size_t elementIndex = 0);
  smtk::attribute::ComponentItemPtr expressionReference() const { return m_expression; }

  // Interface for getting discrete-value based children items
  std::size_t numberOfChildrenItems() const { return m_childrenItems.size(); }

  const std::map<std::string, smtk::attribute::ItemPtr>& childrenItems() const
  {
    return m_childrenItems;
  }

  std::size_t numberOfActiveChildrenItems() const { return m_activeChildrenItems.size(); }

  smtk::attribute::ItemPtr activeChildItem(int i) const
  {
    if ((i < 0) || (static_cast<std::size_t>(i) >= m_activeChildrenItems.size()))
    {
      smtk::attribute::ItemPtr item;
      return item;
    }
    return m_activeChildrenItems[static_cast<std::size_t>(i)];
  }

  ///\brief  Returns the native units of the item.
  ///
  /// These are the units that the item's value methods are returned in and are by default
  /// specified by the item's definition
  virtual const std::string& units() const;

  ///\brief Return native units of the item that are supported by the units system assigned
  /// to its definition.
  ///
  /// If there is no units system assigned to the definition or if the item's native units are
  /// not supported by the units system, an empty string is returned.
  virtual std::string supportedUnits() const;

  using Item::assign;
  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occurred.  By default, an attribute being used by this
  // to represent an expression will be copied if needed.  Use itemOptions.setIgnoreExpressions option to prevent this.
  // In the case that the item uses an expression attribute that is in the same resource as the item, the attribute will
  // also be copied (as well as its definition).  Use itemOptions.setDisableCopyAttributes(true) if copying the expression
  // attribute should not happen and copyOptions.copyDefiniiton(true) if you want the Definition to be copied.
  // Use attributeOptions.setCopyAssociations(true) if you want them copied as well.These options are defined in CopyAssigmentOptions.h .
  Item::Status assign(
    const smtk::attribute::ConstItemPtr& sourceItem,
    const CopyAssignmentOptions& options,
    smtk::io::Logger& logger) override;

  /// @{
  /// \brief Search the item's children - Deprecated! Please use Item::find
  ItemPtr findChild(const std::string& name, smtk::attribute::SearchStyle);
  ConstItemPtr findChild(const std::string& name, smtk::attribute::SearchStyle) const;
  /// @}

protected:
  ValueItem(Attribute* owningAttribute, int itemPosition);
  ValueItem(Item* owningItem, int myPosition, int mySubGroupPosition);
  bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def) override;
  /// \brief Internal implementation of the find method
  smtk::attribute::ItemPtr findInternal(const std::string& name, SearchStyle style) override;
  smtk::attribute::ConstItemPtr findInternal(const std::string& name, SearchStyle style)
    const override;
  virtual void updateDiscreteValue(std::size_t elementIndex) = 0;
  virtual void updateActiveChildrenItems();
  bool isValidInternal(bool useCategories, const std::set<std::string>& categories) const override;
  virtual bool initializeValues() = 0;

  std::vector<int> m_discreteIndices;
  std::vector<bool> m_isSet;
  smtk::attribute::ComponentItemPtr m_expression;
  std::map<std::string, smtk::attribute::ItemPtr> m_childrenItems;
  std::vector<smtk::attribute::ItemPtr> m_activeChildrenItems;

private:
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_ValueItem_h */
