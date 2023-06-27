//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"

#include "units/System.h"

using namespace smtk::attribute;

DoubleItem::DoubleItem(Attribute* owningAttribute, int itemPosition)
  : ValueItemTemplate<double>(owningAttribute, itemPosition)
{
}

DoubleItem::DoubleItem(Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : ValueItemTemplate<double>(inOwningItem, itemPosition, mySubGroupPosition)
{
}

DoubleItem::~DoubleItem() = default;

Item::Type DoubleItem::type() const
{
  return DoubleType;
}

bool DoubleItem::assign(
  const smtk::attribute::ConstItemPtr& sourceItem,
  const CopyAssignmentOptions& options,
  smtk::io::Logger& logger)
{
  // Assigns my contents to be same as sourceItem
  return ValueItemTemplate<double>::assign(sourceItem, options, logger);
}

bool DoubleItem::initializeValues()
{
  const DoubleItemDefinition* def =
    dynamic_cast<const DoubleItemDefinition*>(this->definition().get());
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
        m_valuesAsString = def->defaultValuesAsStrings();
      }
      else
      {
        m_values.resize(n, def->defaultValue());
        m_valuesAsString.resize(n, def->defaultValueAsString());
      }
    }
    else
    {
      m_values.resize(n);
      m_valuesAsString.resize(n);
    }
  }
  return true;
}

bool DoubleItem::setValue(std::size_t element, const double& val)
{
  if (element >= m_values.size())
  {
    return false;
  }
  double origVal = m_values[element];
  if (!ValueItemTemplate<double>::setValue(element, val))
  {
    return false;
  }

  // Did it actually change the value?
  if (m_values[element] == origVal)
  {
    return true; // nothing to be done
  }

  m_valuesAsString[element] = this->streamValue(m_values[element]);
  const DoubleItemDefinition* def =
    dynamic_cast<const DoubleItemDefinition*>(this->definition().get());
  const std::string& units = def->units();
  if (!units.empty())
  {
    m_valuesAsString[element].append(" ").append(units);
  }
  return true;
}

bool DoubleItem::setValue(std::size_t element, const double& val, const std::string& units)
{
  if (element >= m_values.size())
  {
    return false;
  }

  double origVal = m_values[element];
  double convertedVal = val;

  // Do we need to convert?
  const DoubleItemDefinition* def =
    dynamic_cast<const DoubleItemDefinition*>(this->definition().get());
  const std::string& dunits = def->units();
  if (dunits != units)
  {
    // Is there a units system specified?
    if (!def->unitsSystem())
    {
      return false; // we can not convert units
    }
    bool status;
    auto defUnits = def->unitsSystem()->unit(dunits, &status);
    if (!status)
    {
      return false; // Could not find the definition's units
    }
    auto valUnits = def->unitsSystem()->unit(units, &status);
    if (!status)
    {
      return false; // Could not find vals' units
    }

    units::Measurement m(val, valUnits);
    auto newM = def->unitsSystem()->convert(m, defUnits, &status);
    if (!status)
    {
      return false; // could not convert
    }
    convertedVal = newM.m_value;
  }

  // let's try to set the converted value
  if (!ValueItemTemplate<double>::setValue(element, convertedVal))
  {
    return false;
  }

  // Did it actually change the value?
  if (m_values[element] == origVal)
  {
    return true; // nothing to be done
  }
  m_valuesAsString[element] = this->streamValue(m_values[element]);
  if (!units.empty())
  {
    m_valuesAsString[element].append(" ").append(units);
  }
  return true;
}

bool DoubleItem::setValueFromString(std::size_t element, const std::string& val)
{
  // If the string is empty then unset the value.
  if (val.empty())
  {
    this->unset(element);
    return true;
  }

  const DoubleItemDefinition* def =
    dynamic_cast<const DoubleItemDefinition*>(this->definition().get());
  const std::string& dunits = def->units();

  units::Unit defUnit;
  // We can only do conversion if we have a units system and the
  // definition has known units.
  bool convert = false;
  if (def->unitsSystem() && (!dunits.empty()))
  {
    // If we have a units System, let's see if the definition's units
    // are valid?
    defUnit = def->unitsSystem()->unit(def->units(), &convert);
  }

  double convertedVal;
  // Is conversion not possible?
  if (!convert)
  {
    std::istringstream iss(val);
    iss >> convertedVal;
    if (iss.fail())
    {
      return false; // Could not read double
    }
    // Let's see if there are units in the string
    std::string valUnits;
    iss >> valUnits;
    if ((!valUnits.empty()) && (valUnits != dunits))
    {
      return false; // Units were specified that did not match the definition's
    }
  }
  else
  {
    // We can convert units
    bool status;

    auto valMeasure = def->unitsSystem()->measurement(val, &status);
    if (!status)
    {
      // Could not parse the value
      return false;
    }
    if (!valMeasure.m_units.dimensionless())
    {
      auto convertedMeasure = def->unitsSystem()->convert(valMeasure, defUnit, &status);
      if (!status)
      {
        return false;
      }
      convertedVal = convertedMeasure.m_value;
    }
    else
    {
      // No conversion needed there were no units specified
      convertedVal = valMeasure.m_value;
    }
  }
  // See if the converted value can be set
  if (!ValueItemTemplate<double>::setValue(element, convertedVal))
  {
    return false;
  }
  m_valuesAsString[element] = val;
  return true;
}

bool DoubleItem::appendValue(const double& val)
{
  if (!ValueItemTemplate<double>::appendValue(val))
  {
    return false;
  }
  std::string sval = this->streamValue(val);
  const DoubleItemDefinition* def =
    dynamic_cast<const DoubleItemDefinition*>(this->definition().get());
  if (!def->units().empty())
  {
    sval.append(" ").append(def->units());
  }
  m_valuesAsString.push_back(sval);
  return true;
}

bool DoubleItem::removeValue(std::size_t element)
{
  std::size_t origSize = m_values.size();
  if (!ValueItemTemplate<double>::removeValue(element))
  {
    return false;
  }
  // Did we actually remove a value?
  if (m_values.size() == origSize)
  {
    return true; // nothing to do, we just unset the value
  }
  m_valuesAsString.erase(m_valuesAsString.begin() + element);
  return true;
}

void DoubleItem::updateDiscreteValue(std::size_t element)
{
  ValueItemTemplate<double>::updateDiscreteValue(element);
  const DoubleItemDefinition* def =
    dynamic_cast<const DoubleItemDefinition*>(this->definition().get());
  m_valuesAsString[element] = this->streamValue(m_values[element]);
  if (!def->units().empty())
  {
    m_valuesAsString[element].append(" ").append(def->units());
  }
}

bool DoubleItem::rotate(std::size_t fromPosition, std::size_t toPosition)
{
  // Let's first verify that ValueItemTemplate was OK with the rotation.
  if (!ValueItemTemplate<double>::rotate(fromPosition, toPosition))
  {
    return false;
  }

  // No need to check to see if the rotation is valid since ValueItemTemplate already checked it
  this->rotateVector(m_valuesAsString, fromPosition, toPosition);
  return true;
}

bool DoubleItem::setNumberOfValues(std::size_t newSize)
{
  // If the current size is the same just return
  if (this->numberOfValues() == newSize)
  {
    return true;
  }

  // Let's keep track of the original size
  std::size_t origSize = this->maxNumberOfValues();

  if (!ValueItemTemplate<double>::setNumberOfValues(newSize))
  {
    return false;
  }

  // Did we reduce the number of values?
  if (newSize < origSize)
  {
    // Just reduce the size of the string value array
    m_valuesAsString.resize(newSize);
    return true;
  }

  // Do we have defaults?
  const DoubleItemDefinition* def =
    static_cast<const DoubleItemDefinition*>(this->definition().get());
  if (def->hasDefault())
  {
    if (def->defaultValues().size() == newSize)
    {
      m_valuesAsString = def->defaultValuesAsStrings();
    }
    else
    {
      m_valuesAsString.resize(newSize, def->defaultValueAsString());
    }
  }
  else
  {
    m_valuesAsString.resize(newSize);
  }
  return true;
}

std::string DoubleItem::valueAsString(std::size_t element) const
{
  if (this->isExpression())
  {
    // Can the expression be evaluated by value()? |log| will have errors when
    // evaluation is not possible or failed.
    smtk::io::Logger log;
    double val = value(element, log);
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
      return m_valuesAsString[element];
    }

    return "VALUE_IS_NOT_SET";
  }
}
