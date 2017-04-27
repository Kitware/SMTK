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

namespace smtk
{
namespace attribute
{
class Attribute;
class AttributeRefItem;
class AttributeRefItemDefinition;
class Cluster;
class ValueItem;
class SMTKCORE_EXPORT ValueItemDefinition : public smtk::attribute::ItemDefinition
{
public:
  smtkTypeMacro(ValueItemDefinition);
  ValueItemDefinition(const std::string& myname);
  virtual ~ValueItemDefinition();

  const std::string& units() const { return this->m_units; }
  void setUnits(const std::string& newUnits) { this->m_units = newUnits; }

  bool isDiscrete() const { return (this->m_discreteValueEnums.size() != 0); }
  std::size_t numberOfDiscreteValues() const { return this->m_discreteValueEnums.size(); }
  const std::string& discreteEnum(std::size_t ith) const
  {
    assert(this->m_discreteValueEnums.size() > ith);
    return this->m_discreteValueEnums[ith];
  }
  int defaultDiscreteIndex() const { return this->m_defaultDiscreteIndex; }
  void setDefaultDiscreteIndex(int discreteIndex);

  bool allowsExpressions() const;
  bool isValidExpression(smtk::attribute::AttributePtr exp) const;
  smtk::attribute::DefinitionPtr expressionDefinition() const;
  void setExpressionDefinition(smtk::attribute::DefinitionPtr exp);
  // Should only be called internally by the ValueItem
  void buildExpressionItem(ValueItem* vitem, int position) const;
  void buildChildrenItems(ValueItem* vitem) const;

  bool hasDefault() const { return this->m_hasDefault; }

  virtual bool hasRange() const = 0;

  // Returns or Sets the minimum number of values that items from this def can have
  // Default value is 1
  std::size_t numberOfRequiredValues() const { return this->m_numberOfRequiredValues; }

  // Returns false if esize is greater than max number of values (and max number > 0)
  bool setNumberOfRequiredValues(std::size_t esize);

  // Returns or Sets the maximum number of values that items from this def can have.
  // if 0 is returned then there is no max limit.  Default value is 0
  // Note that this is used only when the def is extensible
  std::size_t maxNumberOfValues() const { return this->m_maxNumberOfValues; }

  // Returns false if esize is less than number of required values (and esize > 0)
  bool setMaxNumberOfValues(std::size_t esize);

  // Returns or Sets the def's extensiblity property.  If true then items from this def
  // can have a variable number of values.  The number of values is always <= to number of
  // required values and max number of values (provided max number of values > 0)
  // Default value is false.
  bool isExtensible() const { return this->m_isExtensible; }
  void setIsExtensible(bool mode);

  // Description:
  // Return whether or not there are labels for components.
  // There should only be labels if there is more than a single
  // component (i.e. m_numberOfRequiredValues != 1).
  bool hasValueLabels() const { return !this->m_valueLabels.empty(); }

  // Description:
  // Specify whether the components label is coming from a common
  // label that is repeated.
  bool usingCommonLabel() const { return this->m_useCommonLabel; }
  void setValueLabel(std::size_t element, const std::string& elabel);
  void setCommonValueLabel(const std::string& elabel);
  // Description:
  // Get the component label for specified element component. This
  // takes into account whether to use the common label or specific
  // component label.
  std::string valueLabel(std::size_t element) const;
  bool isDiscreteIndexValid(int index) const
  {
    return ((index > -1) && (static_cast<unsigned int>(index) < this->m_discreteValueEnums.size()));
  }

  // For conditional children items based on the item's current discrete value
  std::size_t numberOfChildrenItemDefinitions() const { return this->m_itemDefs.size(); }

  const std::map<std::string, smtk::attribute::ItemDefinitionPtr>& childrenItemDefinitions() const
  {
    return this->m_itemDefs;
  }

  // returns true if this item has a child item definition of itemName
  bool hasChildItemDefinition(const std::string& itemName) const
  {
    return (this->m_itemDefs.find(itemName) != this->m_itemDefs.end());
  }

  // returns true if valueName has a child item definition of itemName
  bool hasChildItemDefinition(const std::string& valueName, const std::string& itemName);

  bool addChildItemDefinition(smtk::attribute::ItemDefinitionPtr cdef);

  // Description:
  // Create an item definition based on a given idName. If an item
  // with that name already exists then return a shared_pointer
  // that points to NULL.
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
    this->m_itemDefs[item->name()] = item;
    return item;
  }

  bool addConditionalItem(const std::string& enumValue, const std::string& itemName);
  std::vector<std::string> conditionalItems(const std::string& enumValue) const;

protected:
  void copyTo(ValueItemDefinitionPtr def, smtk::attribute::ItemDefinition::CopyInfo& info) const;

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
  smtk::attribute::RefItemDefinitionPtr m_expressionDefinition;
  std::map<std::string, smtk::attribute::ItemDefinitionPtr> m_itemDefs;
  std::map<std::string, std::set<std::string> > m_itemToValueAssociations;
  std::map<std::string, std::vector<std::string> > m_valueToItemAssociations;

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
  return (this->m_itemToValueAssociations[itemName].find(valueName) !=
    this->m_itemToValueAssociations[itemName].end());
}

#endif /* __smtk_attribute_ValueItemDefinition_h */
