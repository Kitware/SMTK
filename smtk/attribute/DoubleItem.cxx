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

Item::Status DoubleItem::assign(
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

const std::string& DoubleItem::units() const
{
  // Check to see if we have explicit units
  if (!m_units.empty())
  {
    return m_units;
  }
  return ValueItem::units();
}

bool DoubleItem::setUnits(const std::string& newUnits)
{
  // make sure that the definition does not already specify units
  const DoubleItemDefinition* def =
    dynamic_cast<const DoubleItemDefinition*>(this->definition().get());
  const std::string& defUnits = def->units();
  if (!defUnits.empty())
  {
    return false; // Units are coming from the Definition
  }

  m_units = newUnits;

  // Now we need to see if the current values are compatible with the new units
  std::string valStr, valUnitsStr;
  bool foundFirstSetValue = false;
  bool canTransferValues = false;
  for (std::size_t i = 0; i < this->numberOfValues(); i++)
  {
    if (!this->isSet(i))
    {
      continue; // skip unset values - nothing needs to be processed
    }
    // if this is the first set value we need to see if it (and all set values)'s units
    // are compatible with the new units.  NOTE - the assumption here is that the units
    // system is consistent - meaning that if all of the existing values were convertible to
    // the original units, and if one value can be converted to the new units, then all of the
    // remaining values must also be convertible
    if (!foundFirstSetValue)
    {
      // Lets see if we can use the  units associate with the values
      canTransferValues = this->setValueFromString(i, m_valuesAsString[i]);
      if (!canTransferValues)
      {
        // So we can't use the value with its associated units so we will
        // split the string and only keep the value
        if (DoubleItemDefinition::splitStringStartingDouble(
              m_valuesAsString[i], valStr, valUnitsStr))
        {
          this->setValueFromString(i, valStr);
        }
        else
        {
          // There was a problem splitting the string (which should never happen) and the value should be unset
          this->unset(i);
        }
      }
      foundFirstSetValue = true;
    }
    // we have determine that the units associated with the values are compatible with the new units
    else if (canTransferValues)
    {
      this->setValueFromString(i, m_valuesAsString[i]);
    }
    // We are only transferring the values and removing the units
    else if (DoubleItemDefinition::splitStringStartingDouble(
               m_valuesAsString[i], valStr, valUnitsStr))
    {
      this->setValueFromString(i, valStr);
    }
    else
    {
      // There was a problem trying to split the string (which should never happen) and the value should be unset
      this->unset(i);
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
  double origVal;
  bool wasSet = false;

  // Was the current value set?
  if (m_isSet[element])
  {
    origVal = m_values[element];
    wasSet = true;
  }

  if (!ValueItemTemplate<double>::setValue(element, val))
  {
    return false;
  }

  // Did it actually change the value?
  if (wasSet && (m_values[element] == origVal))
  {
    return true; // nothing to be done
  }

  m_valuesAsString[element] = this->streamValue(m_values[element]);
  // See if we need to append units - this only needs to be done if the
  // item's units are supported
  auto myUnitStr = this->supportedUnits();
  if (!myUnitStr.empty())
  {
    m_valuesAsString[element].append(" ").append(myUnitStr);
  }
  return true;
}

bool DoubleItem::setValue(std::size_t element, const double& val, const std::string& valUnitStr)
{
  if (element >= m_values.size())
  {
    return false;
  }

  double origVal = m_values[element];
  double convertedVal = val;

  // Do we need to convert?
  const std::string& myUnitStr = this->supportedUnits();
  if (myUnitStr != valUnitStr)
  {
    auto unitsSystem = this->definition()->unitsSystem();
    // Is there a units system specified?
    if (!unitsSystem)
    {
      return false; // we can not convert units
    }
    bool status;
    auto myUnits = unitsSystem->unit(myUnitStr, &status);
    if (!status)
    {
      return false; // Could not find the base's units
    }
    auto valUnits = unitsSystem->unit(valUnitStr, &status);
    if (!status)
    {
      return false; // Could not find vals' units
    }

    units::Measurement m(val, valUnits);
    auto newM = unitsSystem->convert(m, myUnits, &status);
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
  if (!myUnitStr.empty())
  {
    m_valuesAsString[element].append(" ").append(valUnitStr);
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

  std::string valStr, valUnitsStr;
  if (!DoubleItemDefinition::splitStringStartingDouble(val, valStr, valUnitsStr))
  {
    return false; // badly formatted string
  }

  auto unitsSystem = this->definition()->unitsSystem();
  const std::string& myUnitStr = this->supportedUnits();

  units::Unit myUnit;
  // We can only do conversion if we have a units system and the
  // item has known units. Note that we don't need to do conversion
  // if the value does not have units specified
  bool convert = false;
  if (unitsSystem && (!(myUnitStr.empty() || valUnitsStr.empty())))
  {
    // If we have a units System, let's see if the base units
    // are valid?
    myUnit = unitsSystem->unit(myUnitStr, &convert);
  }

  double convertedVal;
  // Is conversion not possible or required
  if (!convert)
  {
    if (!(valUnitsStr.empty() || (valUnitsStr == myUnitStr)))
    {
      return false; // Units were specified that did not match the definition's
    }

    std::istringstream iss(valStr);
    iss >> convertedVal;
    if (iss.fail())
    {
      return false; // Could not read double
    }
  }
  else
  {
    // We can convert units
    bool status;

    auto valMeasure = unitsSystem->measurement(val, &status);
    if (!status)
    {
      // Could not parse the value
      return false;
    }
    if (!valMeasure.m_units.dimensionless())
    {
      auto convertedMeasure = unitsSystem->convert(valMeasure, myUnit, &status);
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

  // if the value didn't have units but the item has units defined  and the units are supported
  // by the units system, append the  units to the value
  if (valUnitsStr.empty() && !myUnitStr.empty())
  {
    m_valuesAsString[element].append(" ").append(myUnitStr);
  }
  // Else if the item's units are not supported then drop the units from the string
  else if (myUnitStr.empty())
  {
    m_valuesAsString[element] = valStr;
  }
  return true;
}

bool DoubleItem::appendValue(const double& val)
{
  if (!ValueItemTemplate<double>::appendValue(val))
  {
    return false;
  }
  std::string sval = this->streamValue(val);
  std::string myUnitStr = this->supportedUnits();
  if (!myUnitStr.empty())
  {
    sval.append(" ").append(myUnitStr);
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
  std::string myUnitStr = this->supportedUnits();
  m_valuesAsString[element] = this->streamValue(m_values[element]);
  if (!myUnitStr.empty())
  {
    m_valuesAsString[element].append(" ").append(myUnitStr);
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

bool DoubleItem::setToDefault(std::size_t element)
{
  const DoubleItemDefinition* def =
    static_cast<const DoubleItemDefinition*>(this->definition().get());
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
    this->setValueFromString(
      element,
      def->defaultValues().size() > 1 ? def->defaultValueAsString(element)
                                      : def->defaultValueAsString(0));
  }
  return true;
}
