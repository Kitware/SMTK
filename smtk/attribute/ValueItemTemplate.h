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

#ifndef smtk_attribute_ValueItemTemplate_h
#define smtk_attribute_ValueItemTemplate_h

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Evaluator.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinitionTemplate.h"
#include "smtk/io/Logger.h"
#include <cassert>
#include <cstdio>
#include <limits>
#include <sstream>
#include <vector>

namespace smtk
{
namespace attribute
{
template<typename DataT>
class SMTK_ALWAYS_EXPORT ValueItemTemplate : public ValueItem
{
  //template<DataT> friend class ValueItemDefinitionTemplate;
public:
  typedef DataT DataType;
  typedef typename std::vector<DataT> value_type;
  typedef typename value_type::const_iterator const_iterator;
  typedef ValueItemDefinitionTemplate<DataType> DefType;

  ~ValueItemTemplate() override = default;
  const_iterator begin() const { return m_values.begin(); }
  const_iterator end() const { return m_values.end(); }
  bool setNumberOfValues(std::size_t newSize) override;

  ///@{
  ///\brief Return the value for the item's specified element
  ///
  /// If the element is not set or in the case of an expression, an error is encountered during
  /// evaluation, DataT() is returned.
  ///
  /// Note:  When DataT does not have a default constructor, as in the case of double or
  /// int. DataT() results in a zero-initialized value:
  /// https://en.cppreference.com/w/cpp/language/value_initialization
  virtual DataT value(std::size_t element = 0) const;
  virtual DataT value(smtk::io::Logger& log) const { return this->value(0, log); }
  virtual DataT value(std::size_t element, smtk::io::Logger& log) const;
  ///@}

  ///\brief Evaluates an Item's expression for a specified element in the units of its definition
  ///
  /// If the element is not set or an error is encountered during
  /// evaluation, DataT() is returned.
  ///
  /// Note:  When DataT does not have a default constructor, as in the case of double or
  /// int. DataT() results in a zero-initialized value:
  /// https://en.cppreference.com/w/cpp/language/value_initialization
  DataT evaluateExpression(std::size_t element, smtk::io::Logger& log) const;
  using ValueItem::valueAsString;
  std::string valueAsString(std::size_t element) const override;
  virtual bool setValue(const DataT& val) { return this->setValue(0, val); }
  virtual bool setValue(std::size_t element, const DataT& val);

  using ValueItem::setValueFromString;
  bool setValueFromString(std::size_t element, const std::string& val) override;

  template<typename I>
  bool setValues(I vbegin, I vend)
  {
    bool ok = false;
    std::size_t num = std::distance(vbegin, vend);
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
  virtual bool appendValue(const DataT& val);
  virtual bool removeValue(std::size_t element);
  void reset() override;
  bool rotate(std::size_t fromPosition, std::size_t toPosition) override;
  bool setToDefault(std::size_t element = 0) override;
  bool isUsingDefault(std::size_t element) const override;
  bool isUsingDefault() const override;
  DataT defaultValue() const;
  const std::vector<DataT>& defaultValues() const;

  using Item::assign;
  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occurred.  By default, an attribute being used by this
  // to represent an expression will be copied if needed.  Use itemOptions.setIgnoreExpressions option to prevent this
  // When an expression attribute is copied, its  associations are by default not.
  // Use attributeOptions.setCopyAssociations option if you want them copied as well.These options are defined in CopyAssigmentOptions.h .
  Item::Status assign(
    const smtk::attribute::ConstItemPtr& sourceItem,
    const CopyAssignmentOptions& options,
    smtk::io::Logger& logger) override;

  shared_ptr<const DefType> concreteDefinition() const
  {
    return dynamic_pointer_cast<const DefType>(this->definition());
  }

protected:
  ValueItemTemplate(Attribute* owningAttribute, int itemPosition);
  ValueItemTemplate(Item* owningItem, int myPosition, int mySubGroupPosition);
  void updateDiscreteValue(std::size_t element) override;
  bool initializeValues() override;

  std::vector<DataT> m_values;
  const std::vector<DataT> m_dummy; //(1, DataT());

  std::string streamValue(const DataT& val) const;
};

template<typename DataT>
ValueItemTemplate<DataT>::ValueItemTemplate(Attribute* owningAttribute, int itemPosition)
  : ValueItem(owningAttribute, itemPosition)
  , m_dummy(1, DataT())
{
}

template<typename DataT>
ValueItemTemplate<DataT>::ValueItemTemplate(
  Item* inOwningItem,
  int itemPosition,
  int mySubGroupPosition)
  : ValueItem(inOwningItem, itemPosition, mySubGroupPosition)
  , m_dummy(1, DataT())
{
}

template<typename DataT>
DataT ValueItemTemplate<DataT>::value(std::size_t element) const
{
  return this->value(element, smtk::io::Logger::instance());
}

template<typename DataT>
DataT ValueItemTemplate<DataT>::evaluateExpression(std::size_t element, smtk::io::Logger& log) const
{
  if (!(this->isSet(element) && this->isExpression()))
  {
    smtkErrorMacro(
      log,
      "Item \"" << this->name() << "\" element " << element << " is either not set "
                << " or is not an expression (attribute \"" << this->attribute()->name() << "\").");
    return DataT();
  }

  smtk::attribute::AttributePtr expAtt = this->expression();
  if (!expAtt)
  {
    smtkErrorMacro(
      log,
      "Item \"" << this->name() << "\" has no reference expression (attribute \""
                << this->attribute()->name() << "\").");
    return DataT();
  }

  std::unique_ptr<smtk::attribute::Evaluator> evaluator = expAtt->createEvaluator();
  if (!evaluator)
  {
    smtkErrorMacro(
      log,
      "Item \"" << this->name() << "\" expression is not evaluate (attribute \""
                << this->attribute()->name() << "\").");
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
    smtkErrorMacro(
      log,
      "Item \"" << this->name() << "\" evaluation result was not compatible (attribute \""
                << this->attribute()->name() << "\").");
    return DataT();
  }

  return resultAsDataT;
}

template<typename DataT>
DataT ValueItemTemplate<DataT>::value(std::size_t element, smtk::io::Logger& log) const
{
  if (!this->isSet(element))
  {
    smtkErrorMacro(
      log,
      "Item \"" << this->name() << "\" element " << element << " is not set (attribute \""
                << this->attribute()->name() << "\").");
    return DataT();
  }

  if (isExpression())
  {
    return this->evaluateExpression(element, log);
  }
  else
  {
    return m_values[element];
  }
}

template<typename DataT>
bool ValueItemTemplate<DataT>::initializeValues()
{
  const DefType* def = dynamic_cast<const DefType*>(this->definition().get());
  if (def == nullptr)
  {
    return false; // Can't initialize values without a definition
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

template<typename DataT>
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
  return !iss.fail() && this->setValue(element, val);
}

template<typename DataT>
bool ValueItemTemplate<DataT>::setValue(std::size_t element, const DataT& val)
{
  // Simple Fail check
  if (m_values.size() <= element)
  {
    return false;
  }
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

template<typename DataT>
void ValueItemTemplate<DataT>::updateDiscreteValue(std::size_t element)
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  m_values[element] = def->discreteValue(static_cast<size_t>(m_discreteIndices[element]));
}

template<typename DataT>
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

template<typename DataT>
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

template<typename DataT>
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

template<typename DataT>
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

template<typename DataT>
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
    this->setValue(
      element,
      def->defaultValues().size() > 1 ? def->defaultValues()[element] : def->defaultValue());
  }
  return true;
}

template<typename DataT>
bool ValueItemTemplate<DataT>::isUsingDefault() const
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  if (!def->hasDefault())
  {
    return false; // Doesn't have a default value
  }

  std::size_t i, n = this->numberOfValues();
  const DataT& dval = def->defaultValue();
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

template<typename DataT>
bool ValueItemTemplate<DataT>::isUsingDefault(std::size_t element) const
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  assert(m_isSet.size() > element);
  if (!(def->hasDefault() && m_isSet[element]))
  {
    return false; // Doesn't have a default value
  }

  const DataT& dval = def->defaultValue();
  const std::vector<DataT>& dvals = def->defaultValues();
  bool vectorDefault = (dvals.size() == def->numberOfRequiredValues());
  assert(m_values.size() > element);
  return (vectorDefault ? m_values[element] == dvals[element] : m_values[element] == dval);
}

template<typename DataT>
DataT ValueItemTemplate<DataT>::defaultValue() const
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  if (!def)
    return m_dummy[0];

  return def->defaultValue();
}

template<typename DataT>
const std::vector<DataT>& ValueItemTemplate<DataT>::defaultValues() const
{
  const DefType* def = static_cast<const DefType*>(this->definition().get());
  if (!def)
    return m_dummy;

  return def->defaultValues();
}

template<typename DataT>
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

template<typename DataT>
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

template<typename DataT>
Item::Status ValueItemTemplate<DataT>::assign(
  const smtk::attribute::ConstItemPtr& sourceItem,
  const CopyAssignmentOptions& options,
  smtk::io::Logger& logger)
{
  Item::Status result = ValueItem::assign(sourceItem, options, logger);
  if (!result.success())
  {
    return result;
  }
  // Cast input pointer to ValueItemTemplate
  const ValueItemTemplate<DataT>* sourceValueItemTemplate =
    dynamic_cast<const ValueItemTemplate<DataT>*>(sourceItem.get());

  if (!sourceValueItemTemplate)
  {
    result.markFailed();
    smtkErrorMacro(logger, "Source Item: " << name() << " is not a ValueItemTemplate");
    return result; // Source is not the right type of item
  }
  // If the item is discrete or an expression there is nothing to be done
  if (sourceValueItemTemplate->isExpression() || sourceValueItemTemplate->isDiscrete())
  {
    return result;
  }

  // Update values
  bool status;
  if (this->numberOfValues() != sourceValueItemTemplate->numberOfValues())
  {
    status = this->setNumberOfValues(sourceValueItemTemplate->numberOfValues());
    if (status)
    {
      result.markModified();
    }
  }

  // Were we able to allocate enough space to fit all of the source's values?
  std::size_t myNumVals, sourceNumVals, numVals;
  myNumVals = this->numberOfValues();
  sourceNumVals = sourceValueItemTemplate->numberOfValues();
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

  for (std::size_t i = 0; i < numVals; ++i)
  {
    if (sourceValueItemTemplate->isSet(i))
    {
      if (this->valueAsString(i) == sourceValueItemTemplate->valueAsString(i))
      {
        continue;
      }
      status = this->setValueFromString(i, sourceValueItemTemplate->valueAsString(i));
      if (!status)
      {
        if (options.itemOptions.allowPartialValues())
        {
          smtkInfoMacro(
            logger,
            "Could not assign Value:" << sourceValueItemTemplate->value(i)
                                      << " to ValueItem: " << sourceItem->name());
          this->unset(i);
          result.markModified();
        }
        else
        {
          result.markFailed();
          smtkErrorMacro(
            logger,
            "Could not assign Value:" << sourceValueItemTemplate->value(i)
                                      << " to ValueItem: " << sourceItem->name()
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
  return result;
}

template<typename DataT>
std::string ValueItemTemplate<DataT>::streamValue(const DataT& val) const
{
  std::stringstream buffer;
  buffer.precision(std::numeric_limits<DataT>::max_digits10);
  buffer << val;
  return buffer.str();
}

} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_ValueItemTemplate_h */
