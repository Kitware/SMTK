//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ValueItemDefinitionTemplate.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_ValueItemDefinitionTemplate_h
#define smtk_attribute_ValueItemDefinitionTemplate_h

#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/common/CompilerInformation.h"

#include <cassert>
#include <sstream>

namespace smtk
{
namespace attribute
{
template<typename DataT>
class SMTK_ALWAYS_EXPORT ValueItemDefinitionTemplate : public smtk::attribute::ValueItemDefinition
{
public:
  typedef DataT DataType;

  ~ValueItemDefinitionTemplate() override = default;

  const DataT& defaultValue() const;
  const DataT& defaultValue(std::size_t element) const;
  const std::vector<DataT>& defaultValues() const;
  virtual bool setDefaultValue(const DataT& val);
  virtual bool setDefaultValue(const std::vector<DataT>& val);
  const DataT& discreteValue(std::size_t element) const { return m_discreteValues[element]; }
  void addDiscreteValue(const DataT& val);
  void addDiscreteValue(const DataT& val, const std::string& discreteEnum);
  void clearDiscreteValues();
  bool hasRange() const override { return m_minRangeSet || m_maxRangeSet; }
  bool hasMinRange() const { return m_minRangeSet; }
  const DataT& minRange() const { return m_minRange; }
  bool minRangeInclusive() const { return m_minRangeInclusive; }
  bool setMinRange(const DataT& minVal, bool isInclusive);
  bool hasMaxRange() const { return m_maxRangeSet; }
  const DataT& maxRange() const { return m_maxRange; }
  bool maxRangeInclusive() const { return m_maxRangeInclusive; }
  bool setMaxRange(const DataT& maxVal, bool isInclusive);
  void clearRange();
  int findDiscreteIndex(const DataT& val) const;
  bool isValueValid(const DataT& val) const;

protected:
  ValueItemDefinitionTemplate(const std::string& myname);
  void copyTo(ValueItemDefinitionPtr def, smtk::attribute::ItemDefinition::CopyInfo& info) const;
  void updateDiscreteValue() override;
  std::vector<DataT> m_defaultValue;
  DataT m_minRange;
  bool m_minRangeSet;
  bool m_minRangeInclusive;
  DataT m_maxRange;
  bool m_maxRangeSet;
  bool m_maxRangeInclusive;
  std::vector<DataT> m_discreteValues;
  DataT m_dummy;

private:
};

template<typename DataT>
ValueItemDefinitionTemplate<DataT>::ValueItemDefinitionTemplate(const std::string& myname)
  : ValueItemDefinition(myname)
{
  m_minRangeSet = false;
  m_minRangeInclusive = false;
  m_maxRangeSet = false;
  m_maxRangeInclusive = false;
  m_dummy = DataT();
}

/**\brief Set the default value for an attribute.
      *
      */
template<typename DataT>
bool ValueItemDefinitionTemplate<DataT>::setDefaultValue(const DataT& dvalue)
{
  std::vector<DataT> defaultTuple(1, dvalue);
  return this->setDefaultValue(defaultTuple);
}

template<typename DataT>
void ValueItemDefinitionTemplate<DataT>::updateDiscreteValue()
{
  assert(static_cast<int>(m_discreteValues.size()) > m_defaultDiscreteIndex);
  this->setDefaultValue(m_discreteValues[m_defaultDiscreteIndex]);
}

/**\brief Set the default value for an attribute.
      *
      * This variant takes an array of values so that
      * vector quantities can have a different default
      * for each component.
      *
      * However, a single default can always be specified
      * (even for vector-valued attributes), and **must**
      * be specified for discrete and extensible attributes.
      *
      * When a single default is provided but the attribute
      * is vector-valued, then the default is used for each
      * component.
      */
template<typename DataT>
bool ValueItemDefinitionTemplate<DataT>::setDefaultValue(const std::vector<DataT>& dvalue)
{
  if (dvalue.empty())
    return false; // *some* value must be provided.
  if (
    dvalue.size() > 1 &&
    (this->isDiscrete() || this->isExtensible() ||
     (dvalue.size() != this->numberOfRequiredValues())))
    return false; // only fixed-size attributes can have vector defaults.
  typename std::vector<DataT>::const_iterator it;
  for (it = dvalue.begin(); it != dvalue.end(); ++it)
  {
    if (!this->isValueValid(*it))
    {
      return false; // Is each value valid?
    }
  }
  m_defaultValue = dvalue;
  m_hasDefault = true;
  return true;
}

template<typename DataT>
void ValueItemDefinitionTemplate<DataT>::addDiscreteValue(const DataT& dvalue)
{
  // Set the label to be based on the value
  std::ostringstream oss;
  oss << dvalue;
  this->addDiscreteValue(dvalue, oss.str());
}

template<typename DataT>
void ValueItemDefinitionTemplate<DataT>::addDiscreteValue(
  const DataT& dvalue,
  const std::string& dlabel)
{
  m_discreteValues.push_back(dvalue);
  m_discreteValueEnums.push_back(dlabel);
}

template<typename DataT>
void ValueItemDefinitionTemplate<DataT>::clearDiscreteValues()
{
  m_discreteValues.clear();
  m_discreteValueEnums.clear();
}

template<typename DataT>
bool ValueItemDefinitionTemplate<DataT>::setMinRange(const DataT& minVal, bool isInclusive)
{
  // If there is a default value is it within the new range?
  if (m_hasDefault)
  {
    typename std::vector<DataT>::const_iterator it;
    for (it = m_defaultValue.begin(); it != m_defaultValue.end(); ++it)
    {
      if (*it < minVal)
        return false;
      if ((!isInclusive) && (*it == minVal))
        return false;
    }
  }
  if ((!m_maxRangeSet) || (minVal < m_maxRange))
  {
    m_minRangeSet = true;
    m_minRange = minVal;
    m_minRangeInclusive = isInclusive;
    return true;
  }
  return false;
}

template<typename DataT>
bool ValueItemDefinitionTemplate<DataT>::setMaxRange(const DataT& maxVal, bool isInclusive)
{
  // If there is a default value is it within the new range?
  if (m_hasDefault)
  {
    typename std::vector<DataT>::const_iterator it;
    for (it = m_defaultValue.begin(); it != m_defaultValue.end(); ++it)
    {
      if (*it > maxVal)
        return false;
      if ((!isInclusive) && (*it == maxVal))
        return false;
    }
  }
  if ((!m_minRangeSet) || (maxVal > m_minRange))
  {
    m_maxRangeSet = true;
    m_maxRange = maxVal;
    m_maxRangeInclusive = isInclusive;
    return true;
  }
  return false;
}

template<typename DataT>
void ValueItemDefinitionTemplate<DataT>::clearRange()
{
  m_minRangeSet = false;
  m_maxRangeSet = false;
}

template<typename DataT>
int ValueItemDefinitionTemplate<DataT>::findDiscreteIndex(const DataT& val) const
{
  // Are we dealing with Discrete Values?
  if (!this->isDiscrete())
  {
    return -1;
  }
  std::size_t i, n = m_discreteValues.size();
  for (i = 0; i < n; i++)
  {
    if (m_discreteValues[i] == val)
    {
      return static_cast<int>(i);
    }
  }
  return -1;
}

template<typename DataT>
bool ValueItemDefinitionTemplate<DataT>::isValueValid(const DataT& val) const
{
  // Are we dealing with Discrete Values?
  if (this->isDiscrete())
  {
    return (this->findDiscreteIndex(val) != -1);
  }
  if (!this->hasRange())
  {
    return true;
  }
  if (m_minRangeSet && ((val < m_minRange) || ((!m_minRangeInclusive) && (val == m_minRange))))
  {
    return false;
  }
  if (m_maxRangeSet && ((val > m_maxRange) || ((!m_maxRangeInclusive) && (val == m_maxRange))))
  {
    return false;
  }
  return true;
}

template<typename DataT>
const DataT& ValueItemDefinitionTemplate<DataT>::defaultValue() const
{
  return m_defaultValue.empty() ? m_dummy : m_defaultValue[0];
}

template<typename DataT>
const DataT& ValueItemDefinitionTemplate<DataT>::defaultValue(std::size_t element) const
{
  bool vectorDefault = m_defaultValue.size() == this->numberOfRequiredValues();
  assert(!vectorDefault || m_defaultValue.size() > element);
  return m_defaultValue.empty() ? m_dummy : m_defaultValue[vectorDefault ? element : 0];
}

template<typename DataT>
const std::vector<DataT>& ValueItemDefinitionTemplate<DataT>::defaultValues() const
{
  return m_defaultValue;
}

// Copies my contents to input definition
// Input argument is ValueItemDefinition shared pointer, which must be
// cast to (raw) ValueItemTemplateDefinition pointer.
template<typename DataT>
void ValueItemDefinitionTemplate<DataT>::copyTo(
  ValueItemDefinitionPtr def,
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  // Get raw pointer and cast to ValueItemDefinitionTemplate*
  ValueItemDefinition* rawDef = def.get();
  ValueItemDefinitionTemplate<DataT>* vdef =
    dynamic_cast<ValueItemDefinitionTemplate<DataT>*>(rawDef);

  if (this->hasDefault())
  {
    vdef->setDefaultValue(m_defaultValue);
  }

  if (m_minRangeSet)
  {
    vdef->setMinRange(m_minRange, m_minRangeInclusive);
  }

  if (m_maxRangeSet)
  {
    vdef->setMaxRange(m_maxRange, m_maxRangeInclusive);
  }

  if (this->isDiscrete())
  {
    // Copy values & labels
    DataT value;
    std::string labelStr;
    assert(m_discreteValueEnums.size() >= m_discreteValues.size());
    for (std::size_t i = 0; i < m_discreteValues.size(); ++i)
    {
      value = m_discreteValues[i];
      labelStr = m_discreteValueEnums[i];
      vdef->addDiscreteValue(value, labelStr);
    }
    if (this->hasDefault())
    {
      vdef->setDefaultDiscreteIndex(m_defaultDiscreteIndex);
    }
  }

  // Copy superclass *after* our stuff, so that discrete values are set up
  ValueItemDefinition::copyTo(def, info);
}

} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_ValueItemDefinitionTemplate_h */
