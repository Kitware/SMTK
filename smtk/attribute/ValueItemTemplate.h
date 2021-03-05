//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ValueItemTemplate.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_ValueItemTemplate_h
#define __smtk_attribute_ValueItemTemplate_h

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Evaluator.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinitionTemplate.h"
#include "smtk/io/Logger.h"
#include <cassert>
#include <limits>
#include <sstream>
#include <stdio.h>
#include <vector>

namespace smtk
{
namespace attribute
{
template <typename DataT>
class ValueItemTemplate : public ValueItem
{
  //template<DataT> friend class ValueItemDefinitionTemplate;
public:
  typedef DataT DataType;
  typedef typename std::vector<DataT> value_type;
  typedef value_type const_iterator;
  typedef ValueItemDefinitionTemplate<DataType> DefType;

  ~ValueItemTemplate() override {}
  typename std::vector<DataT>::const_iterator begin() const { return m_values.begin(); }
  typename std::vector<DataT>::const_iterator end() const { return m_values.end(); }
  bool setNumberOfValues(std::size_t newSize) override;

  DataT value(std::size_t element = 0) const;
  DataT value(smtk::io::Logger& log) const { return this->value(0, log); }
  DataT value(std::size_t element, smtk::io::Logger& log) const;

  std::string valueAsString() const override { return this->valueAsString(0); }
  std::string valueAsString(std::size_t element) const override;
  bool setValue(const DataT& val) { return this->setValue(0, val); }
  bool setValue(std::size_t element, const DataT& val);
  bool setValueFromString(std::size_t element, const std::string& val) override;
  template <typename I>
  bool setValues(I vbegin, I vend)
  {
    bool ok = false;
    std::size_t num = vend - vbegin;
    if (this->setNumberOfValues(num))
    {
      ok = true;
      std::size_t i = 0;
      for (I it = vbegin; it != vend; ++it, ++i)
      {
        if (!this->setValue(i, *it))
        {
          ok = false;
          break;
        }
      }
    }
    return ok;
  }
  bool appendValue(const DataT& val);
  bool removeValue(std::size_t element);
  void reset() override;
  bool rotate(std::size_t fromPosition, std::size_t toPosition) override;
  bool setToDefault(std::size_t element = 0) override;
  bool isUsingDefault(std::size_t element) const override;
  bool isUsingDefault() const override;
  DataT defaultValue() const;
  const std::vector<DataT>& defaultValues() const;
  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occured.  By default, an attribute being used by this
  // to represent an expression will be copied if needed.  Use IGNORE_EXPRESSIONS option to prevent this
  // When an expression attribute is copied, its model associations are by default not.
  // Use COPY_MODEL_ASSOCIATIONS if you want them copied as well.These options are defined in Item.h .
  bool assign(smtk::attribute::ConstItemPtr& sourceItem, unsigned int options = 0) override;
  shared_ptr<const DefType> concreteDefinition() const
  {
    return dynamic_pointer_cast<const DefType>(this->definition());
  }

protected:
  ValueItemTemplate(Attribute* owningAttribute, int itemPosition);
  ValueItemTemplate(Item* owningItem, int myPosition, int mySubGroupPosition);
  bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef) override;
  void updateDiscreteValue(std::size_t element) override;
  std::vector<DataT> m_values;
  const std::vector<DataT> m_dummy; //(1, DataT());

private:
  std::string streamValue(const DataT& val) const;
};

template <typename DataT>
ValueItemTemplate<DataT>::ValueItemTemplate(Attribute* owningAttribute, int itemPosition)
  : ValueItem(owningAttribute, itemPosition)
  , m_dummy(1, DataT())
{
}

template <typename DataT>
ValueItemTemplate<DataT>::ValueItemTemplate(
  Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : ValueItem(inOwningItem, itemPosition, mySubGroupPosition)
  , m_dummy(1, DataT())
{
}

template <typename DataT>
DataT ValueItemTemplate<DataT>::value(std::size_t element) const
{
  return this->value(element, smtk::io::Logger::instance());
}

// When DataT does not have a default constructor, as in the case of double or
// int. DataT() results in a zero-initialized value:
// https://en.cppreference.com/w/cpp/language/value_initialization
template <typename DataT>
DataT ValueItemTemplate<DataT>::value(std::size_t element, smtk::io::Logger& log) const
{
  if (!this->isSet(element))
  {
    log.addRecord(smtk::io::Logger::ERROR, std::to_string(element) + " is not set.");
    return DataT();
  }

  if (isExpression())
  {
    smtk::attribute::AttributePtr expAtt = expression();
    if (!expAtt)
    {
      log.addRecord(smtk::io::Logger::ERROR, "Could not find referenced expression.");
      return DataT();
    }

    std::unique_ptr<smtk::attribute::Evaluator> evaluator = expAtt->createEvaluator();
    if (!evaluator)
    {
      log.addRecord(smtk::io::Logger::ERROR, "Expression is not evaluatable.");
      return DataT();
    }

    smtk::attribute::Evaluator::ValueType result;
    // |evaluator| will report errors in |log| for the caller.
    if (!evaluator->evaluate(
          result, log, element, Evaluator::DependentEvaluationMode::EVALUATE_DEPENDENTS))
    {
      return DataT();
    }

    DataT resultAsDataT;
    try
    {
      resultAsDataT = boost::get<DataT>(result);
    }
    catch (const boost::bad_get&)
    {
      log.addRecord(smtk::io::Logger::ERROR, "Evaluation result was not compatible.");
      return DataT();
    }

    return resultAsDataT;
  }
  else
  {
    return m_values[element];
  }
}

template <typename DataT>
bool ValueItemTemplate<DataT>::setDefinition(smtk::attribute::ConstItemDefinitionPtr tdef)
{
  // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const DefType* def = dynamic_cast<const DefType*>(tdef.get());
  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if ((def == NULL) || (!ValueItem::setDefinition(tdef)))
  {
    return false;
  }
  size_t n = def->numberOfRequiredValues();
  if (n)
  {
    if (def->hasDefault())
    {
      // Assumes that if the definition is discrete then default value
      // will be based on the default discrete index
      if (def->defaultValues().size() > 1)
      {
        m_values = def->defaultValues();
      }
      else
      {
        m_values.resize(n, def->defaultValue());
      }
    }
    else
    {
      m_values.resize(n);
    }
  }
  return true;
}

template <typename DataT>
bool ValueItemTemplate<DataT>::setValueFromString(std::size_t element, const std::string& sval)
{
  // If the string is empty then unset the value.
  if (sval.empty())
  {
    this->unset(element);
    return true;
  }

  std::istringstream iss(sval);
  DataT val;
  iss >> val;
  if (iss.fail() || !this->setValue(element, val))
  {
    return false;
  }
  return true;
}

template <typename DataT>
bool ValueItemTemplate<DataT>::setValue(std::size_t element, const DataT& val)
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  if (def->isDiscrete())
  {
    int index = def->findDiscreteIndex(val);
    // Is this the current value?
    assert(m_discreteIndices.size() > element);
    if (index == m_discreteIndices[element])
    {
      return true;
    }

    if (index != -1)
    {
      m_discreteIndices[element] = index;
      assert(m_values.size() > element);
      m_values[element] = val;
      if (def->allowsExpressions())
      {
        m_expression->unset();
      }
      assert(m_isSet.size() > element);
      m_isSet[element] = true;
      // Update active children if needed - note that
      // we currently only support active children based on the
      // 0th value changing
      if (element == 0)
      {
        this->updateActiveChildrenItems();
      }
      return true;
    }
    return false;
  }
  if (def->isValueValid(val))
  {
    assert(m_values.size() > element);
    m_values[element] = val;
    assert(m_isSet.size() > element);
    m_isSet[element] = true;
    if (def->allowsExpressions())
    {
      m_expression->unset();
    }
    return true;
  }
  return false;
}

template <typename DataT>
void ValueItemTemplate<DataT>::updateDiscreteValue(std::size_t element)
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  m_values[element] = def->discreteValue(static_cast<size_t>(m_discreteIndices[element]));
}

template <typename DataT>
std::string ValueItemTemplate<DataT>::valueAsString(std::size_t element) const
{
  if (this->isExpression())
  {
    // Can the expression be evaluated by value()? |log| will have errors when
    // evaluation is not possible or failed.
    smtk::io::Logger log;
    DataT val = value(element, log);
    if (log.hasErrors())
    {
      return "CANNOT_EVALUATE";
    }

    return streamValue(val);
  }
  else
  {
    assert(m_isSet.size() > element);
    if (m_isSet[element])
    {
      assert(m_values.size() > element);
      return streamValue(m_values[element]);
    }

    return "VALUE_IS_NOT_SET";
  }
}

template <typename DataT>
bool ValueItemTemplate<DataT>::appendValue(const DataT& val)
{
  //First - are we allowed to change the number of values?
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  if (!this->isExtensible())
  {
    return false; // The number of values is fixed
  }

  std::size_t n = this->maxNumberOfValues();
  if (n && (this->numberOfValues() >= n))
  {
    return false; // max number reached
  }

  if (def->isDiscrete())
  {
    int index = def->findDiscreteIndex(val);
    if (index != -1)
    {
      m_values.push_back(val);
      m_discreteIndices.push_back(index);
      m_isSet.push_back(true);
      return true;
    }
    return false;
  }
  if (def->isValueValid(val))
  {
    if (def->allowsExpressions())
    {
      m_expression->unset();
    }
    m_values.push_back(val);
    m_isSet.push_back(true);
    return true;
  }
  return false;
}

template <typename DataT>
bool ValueItemTemplate<DataT>::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
  {
    return true;
  }

  //Next - are we allowed to change the number of values?
  if (!this->isExtensible())
  {
    return false; // The number of values is fixed
  }

  // Is this size between the required number and the max?
  if (newSize < this->numberOfRequiredValues())
  {
    return false;
  }

  std::size_t n = this->maxNumberOfValues();
  if (n && (newSize > n))
  {
    return false; // greater than max number
  }

  const DefType* def = static_cast<const DefType*>(this->definition().get());
  n = this->numberOfValues();
  // Are we increasing or decreasing?
  if (newSize < n)
  {
    m_values.resize(newSize);
    m_isSet.resize(newSize);
    if (def->isDiscrete())
    {
      m_discreteIndices.resize(newSize);
    }
    return true;
  }
  if (def->hasDefault())
  {
    if (def->defaultValues().size() == newSize)
    {
      m_values = def->defaultValues();
    }
    else
    {
      m_values.resize(newSize, def->defaultValue());
    }
    m_isSet.resize(newSize, true);
  }
  else
  {
    m_values.resize(newSize);
    m_isSet.resize(newSize, false);
  }
  if (def->isDiscrete())
  {
    m_discreteIndices.resize(newSize, def->defaultDiscreteIndex());
  }
  return true;
}

template <typename DataT>
bool ValueItemTemplate<DataT>::removeValue(std::size_t i)
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  // If i < the required number of values this is the same as unset - else if
  // its extensible remove it completely
  if (i < def->numberOfRequiredValues())
  {
    this->unset(i);
    return true;
  }
  if (i >= this->numberOfValues())
  {
    return false; // i can't be greater than the number of values
  }
  m_values.erase(m_values.begin() + i);
  m_isSet.erase(m_isSet.begin() + i);
  if (def->isDiscrete())
  {
    m_discreteIndices.erase(m_discreteIndices.begin() + i);
  }
  return true;
}

template <typename DataT>
bool ValueItemTemplate<DataT>::setToDefault(std::size_t element)
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  if (!def->hasDefault())
  {
    return false; // Doesn't have a default value
  }

  if (def->isDiscrete())
  {
    this->setDiscreteIndex(element, def->defaultDiscreteIndex());
  }
  else
  {
    assert(def->defaultValues().size() > element);
    this->setValue(element,
      def->defaultValues().size() > 1 ? def->defaultValues()[element] : def->defaultValue());
  }
  return true;
}

template <typename DataT>
bool ValueItemTemplate<DataT>::isUsingDefault() const
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  if (!def->hasDefault())
  {
    return false; // Doesn't have a default value
  }

  std::size_t i, n = this->numberOfValues();
  DataT dval = def->defaultValue();
  const std::vector<DataT>& dvals = def->defaultValues();
  bool vectorDefault = (dvals.size() == n);
  for (i = 0; i < n; i++)
  {
    assert(m_isSet.size() > i);
    assert(m_values.size() > i);
    if (!(m_isSet[i] && (vectorDefault ? m_values[i] == dvals[i] : m_values[i] == dval)))
    {
      return false;
    }
  }
  return true;
}

template <typename DataT>
bool ValueItemTemplate<DataT>::isUsingDefault(std::size_t element) const
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  assert(m_isSet.size() > element);
  if (!(def->hasDefault() && m_isSet[element]))
  {
    return false; // Doesn't have a default value
  }

  DataT dval = def->defaultValue();
  const std::vector<DataT>& dvals = def->defaultValues();
  bool vectorDefault = (dvals.size() == def->numberOfRequiredValues());
  assert(m_values.size() > element);
  return (vectorDefault ? m_values[element] == dvals[element] : m_values[element] == dval);
}

template <typename DataT>
DataT ValueItemTemplate<DataT>::defaultValue() const
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  if (!def)
    return m_dummy[0];

  return def->defaultValue();
}

template <typename DataT>
const std::vector<DataT>& ValueItemTemplate<DataT>::defaultValues() const
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  if (!def)
    return m_dummy;

  return def->defaultValues();
}

template <typename DataT>
void ValueItemTemplate<DataT>::reset()
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  // If we can have an expression then clear it
  if (def->allowsExpressions())
  {
    m_expression->unset();
  }

  // Was the initial size 0?
  std::size_t i, n = this->numberOfRequiredValues();
  if (this->numberOfValues() != n)
  {
    this->setNumberOfValues(n);
  }
  if (!n)
  {
    m_values.clear();
    m_isSet.clear();
    m_discreteIndices.clear();
    ValueItem::reset();
    return;
  }

  if (!def->hasDefault()) // Do we have default values
  {
    for (i = 0; i < n; i++)
    {
      this->unset(i);
    }
    ValueItem::reset();
    return;
  }

  if (def->isDiscrete())
  {
    int index = def->defaultDiscreteIndex();
    for (i = 0; i < n; i++)
    {
      this->setDiscreteIndex(i, index);
    }
  }
  else
  {
    DataT dval = def->defaultValue();
    const std::vector<DataT>& dvals = def->defaultValues();
    bool vectorDefault = (dvals.size() == n);
    for (i = 0; i < n; i++)
    {
      this->setValue(i, vectorDefault ? dvals[i] : dval);
    }
  }
  ValueItem::reset();
}

template <typename DataT>
bool ValueItemTemplate<DataT>::rotate(std::size_t fromPosition, std::size_t toPosition)
{
  // Let's first verify that ValueItem was OK with the rotation.
  if (!ValueItem::rotate(fromPosition, toPosition))
  {
    return false;
  }

  // No need to check to see if the rotation is valid since ValueItem already checked it
  this->rotateVector(m_values, fromPosition, toPosition);
  return true;
}

template <typename DataT>
bool ValueItemTemplate<DataT>::assign(
  smtk::attribute::ConstItemPtr& sourceItem, unsigned int options)
{
  if (!ValueItem::assign(sourceItem, options))
  {
    return false;
  }
  // Cast input pointer to ValueItemTemplate
  const ValueItemTemplate<DataT>* sourceValueItemTemplate =
    dynamic_cast<const ValueItemTemplate<DataT>*>(sourceItem.get());

  if (!sourceValueItemTemplate)
  {
    return false; // Source is not the right type of item
  }
  // If the item is discrete or an expression there is nothing to be done
  if (!(sourceValueItemTemplate->isExpression() || sourceValueItemTemplate->isDiscrete()))
  {
    // Update values
    this->setNumberOfValues(sourceValueItemTemplate->numberOfValues());
    for (std::size_t i = 0; i < sourceValueItemTemplate->numberOfValues(); ++i)
    {
      if (sourceValueItemTemplate->isSet(i))
      {
        this->setValue(i, sourceValueItemTemplate->value(i));
      }
    }
  }
  return true;
}

template <typename DataT>
std::string ValueItemTemplate<DataT>::streamValue(const DataT& val) const
{
  std::stringstream buffer;
  buffer.precision(std::numeric_limits<DataT>::max_digits10);
  buffer << val;
  return buffer.str();
}

} // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ValueItemTemplate_h */
