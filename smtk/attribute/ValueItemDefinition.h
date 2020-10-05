//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ValueItemDefinition.h -
// .SECTION Description
// The base class for attribute items that have an input value.
// It can have a specified number of values (m_numberOfRequiredValues > 0)
// or an unbounded number of values (m_numberOfRequredValues == 0).
// m_valueLabels is used to store labels for individual component values
// but can hold a common label that should be used for all components.
// m_valueLabels should only be used if m_numberOfRequiredValues is not 1.
// .SECTION See Also

#ifndef __smtk_attribute_ValueItemDefinition_h
#define __smtk_attribute_ValueItemDefinition_h

#include <cassert>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

namespace smtk
{
namespace attribute
{
class Attribute;
class Cluster;
class ValueItem;
class SMTKCORE_EXPORT ValueItemDefinition : public smtk::attribute::ItemDefinition
{
public:
  smtkTypeMacro(smtk::attribute::ValueItemDefinition);
  ValueItemDefinition(const std::string& myname);
  ~ValueItemDefinition() override;

  const std::string& units() const { return m_units; }
  void setUnits(const std::string& newUnits) { m_units = newUnits; }

  bool isDiscrete() const { return (m_discreteValueEnums.size() != 0); }
  std::size_t numberOfDiscreteValues() const { return m_discreteValueEnums.size(); }
  const std::string& discreteEnum(std::size_t ith) const
  {
    assert(m_discreteValueEnums.size() > ith);
    return m_discreteValueEnums[ith];
  }
  bool getEnumIndex(const std::string& enumVal, std::size_t& index) const;
  int defaultDiscreteIndex() const { return m_defaultDiscreteIndex; }
  void setDefaultDiscreteIndex(int discreteIndex);

  bool allowsExpressions() const;
  bool isValidExpression(const smtk::attribute::AttributePtr& exp) const;
  std::string expressionType() const { return m_expressionType; }
  void setExpressionType(const std::string& etype) { m_expressionType = etype; }
  void setExpressionDefinition(const smtk::attribute::DefinitionPtr& exp);
  smtk::attribute::DefinitionPtr expressionDefinition(
    const smtk::attribute::ResourcePtr& attResource) const;
  // Should only be called internally by the ValueItem
  void buildExpressionItem(ValueItem* vitem) const;
  void buildChildrenItems(ValueItem* vitem) const;

  bool hasDefault() const { return m_hasDefault; }

  virtual bool hasRange() const = 0;

  // Returns or Sets the minimum number of values that items from this def can have
  // Default value is 1
  std::size_t numberOfRequiredValues() const { return m_numberOfRequiredValues; }

  // Returns false if esize is greater than max number of values (and max number > 0)
  bool setNumberOfRequiredValues(std::size_t esize);

  // Returns or Sets the maximum number of values that items from this def can have.
  // if 0 is returned then there is no max limit.  Default value is 0
  // Note that this is used only when the def is extensible
  std::size_t maxNumberOfValues() const { return m_maxNumberOfValues; }

  // Returns false if esize is less than number of required values (and esize > 0)
  bool setMaxNumberOfValues(std::size_t esize);

  // Returns or Sets the def's extensiblity property.  If true then items from this def
  // can have a variable number of values.  The number of values is always <= to number of
  // required values and max number of values (provided max number of values > 0)
  // Default value is false.
  bool isExtensible() const { return m_isExtensible; }
  void setIsExtensible(bool mode);

  // Description:
  // Return whether or not there are labels for components.
  // There should only be labels if there is more than a single
  // component (i.e. m_numberOfRequiredValues != 1).
  bool hasValueLabels() const { return !m_valueLabels.empty(); }

  // Description:
  // Specify whether the components label is coming from a common
  // label that is repeated.
  bool usingCommonLabel() const { return m_useCommonLabel; }
  void setValueLabel(std::size_t element, const std::string& elabel);
  void setCommonValueLabel(const std::string& elabel);
  // Description:
  // Get the component label for specified element component. This
  // takes into account whether to use the common label or specific
  // component label.
  std::string valueLabel(std::size_t element) const;
  bool isDiscreteIndexValid(int index) const
  {
    return ((index > -1) && (static_cast<unsigned int>(index) < m_discreteValueEnums.size()));
  }

  // For conditional children items based on the item's current discrete value
  std::size_t numberOfChildrenItemDefinitions() const { return m_itemDefs.size(); }

  const std::map<std::string, smtk::attribute::ItemDefinitionPtr>& childrenItemDefinitions() const
  {
    return m_itemDefs;
  }

  // returns true if this item has a child item definition of itemName
  bool hasChildItemDefinition(const std::string& itemName) const
  {
    return (m_itemDefs.find(itemName) != m_itemDefs.end());
  }

  // returns true if valueName has a child item definition of itemName
  bool hasChildItemDefinition(const std::string& valueName, const std::string& itemName);

  bool addChildItemDefinition(smtk::attribute::ItemDefinitionPtr cdef);

  /// @{
  ///\brief Set/Get the set of categories associated with a value Enum
  void setEnumCategories(
    const std::string& enumValue, const smtk::attribute::Categories::Set& cats);
  void addEnumCategory(const std::string& enumValue, const std::string& cat);
  const smtk::attribute::Categories::Set& enumCategories(const std::string& enumValue) const;
  /// @}

  /// @{
  ///\brief Set/Get advance level associated with associated with a value Enum
  void setEnumAdvanceLevel(const std::string& enumValue, unsigned int level);
  void unsetEnumAdvanceLevel(const std::string& enumValue);
  unsigned int enumAdvanceLevel(const std::string& enumValue) const;
  bool hasEnumAdvanceLevel(const std::string& enumValue) const;
  const std::map<std::string, unsigned int> enumAdvanceLevelInfo() const
  {
    return m_valueToAdvanceLevelAssociations;
  }
  ///@}

  // Description:
  // Create an item definition based on a given idName. If an item
  // with that name already exists then return a shared_pointer
  // that points to NULL.
  bool addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef);
  template <typename T>
  typename smtk::internal::shared_ptr_type<T>::SharedPointerType addItemDefinition(
    const std::string& idName)
  {
    typedef smtk::internal::shared_ptr_type<T> SharedTypes;
    typename SharedTypes::SharedPointerType item;

    // First see if there is a item by the same name
    if (this->hasChildItemDefinition(idName))
    {
      // Already has an item of this name - do nothing
      return item;
    }
    item = SharedTypes::RawPointerType::New(idName);
    m_itemDefs[item->name()] = item;
    return item;
  }

  bool addConditionalItem(const std::string& enumValue, const std::string& itemName);
  std::vector<std::string> conditionalItems(const std::string& enumValue) const;

protected:
  void copyTo(ValueItemDefinitionPtr def, smtk::attribute::ItemDefinition::CopyInfo& info) const;
  void applyCategories(const smtk::attribute::Categories& inheritedFromParent,
    smtk::attribute::Categories& inheritedToParent) override;
  void applyAdvanceLevels(
    const unsigned int& readLevelFromParent, const unsigned int& writeLevelFromParent) override;

  virtual void updateDiscreteValue() = 0;
  bool m_hasDefault;
  bool m_useCommonLabel;
  std::vector<std::string> m_valueLabels;
  std::vector<std::string> m_discreteValueEnums;
  int m_defaultDiscreteIndex;
  std::size_t m_numberOfRequiredValues;
  std::size_t m_maxNumberOfValues;
  bool m_isExtensible;
  std::string m_units;
  std::string m_expressionType;
  smtk::attribute::ComponentItemDefinitionPtr m_expressionDefinition;
  std::map<std::string, smtk::attribute::ItemDefinitionPtr> m_itemDefs;
  std::map<std::string, std::set<std::string> > m_itemToValueAssociations;
  std::map<std::string, std::vector<std::string> > m_valueToItemAssociations;
  std::map<std::string, smtk::attribute::Categories::Set> m_valueToCategoryAssociations;
  std::map<std::string, unsigned int> m_valueToAdvanceLevelAssociations;

private:
};
}
}

// returns true if valueName has a child item definition of itemName
inline bool smtk::attribute::ValueItemDefinition::hasChildItemDefinition(
  const std::string& valueName, const std::string& itemName)
{
  // First we need to check to see if we have this child item
  if (!this->hasChildItemDefinition(itemName))
  {
    return false;
  }
  return (m_itemToValueAssociations[itemName].find(valueName) !=
    m_itemToValueAssociations[itemName].end());
}

#endif /* __smtk_attribute_ValueItemDefinition_h */
