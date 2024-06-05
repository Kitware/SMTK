//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"

#include "smtk/common/StringUtil.h"
#include "smtk/io/Logger.h"

#include "units/Converter.h"
#include "units/System.h"

using namespace smtk::attribute;

DoubleItemDefinition::DoubleItemDefinition(const std::string& myName)
  : ValueItemDefinitionTemplate<double>(myName)
{
}

DoubleItemDefinition::~DoubleItemDefinition() = default;

Item::Type DoubleItemDefinition::type() const
{
  return Item::DoubleType;
}

smtk::attribute::ItemPtr DoubleItemDefinition::buildItem(
  Attribute* owningAttribute,
  int itemPosition) const
{
  return smtk::attribute::ItemPtr(new DoubleItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr
DoubleItemDefinition::buildItem(Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new DoubleItem(owningItem, itemPosition, subGroupPosition));
}

smtk::attribute::ItemDefinitionPtr smtk::attribute::DoubleItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  smtk::attribute::DoubleItemDefinitionPtr newDef =
    smtk::attribute::DoubleItemDefinition::New(this->name());

  ValueItemDefinitionTemplate<double>::copyTo(newDef, info);
  return newDef;
}

bool DoubleItemDefinition::setDefaultValue(const std::vector<double>& vals)
{
  return this->setDefaultValue(vals, m_units);
}

const std::string DoubleItemDefinition::defaultValueAsString(std::size_t element) const
{
  bool vectorDefault = m_defaultValuesAsStrings.size() == this->numberOfRequiredValues();
  assert(!vectorDefault || m_defaultValuesAsStrings.size() > element);
  return m_defaultValuesAsStrings.empty() ? ""
                                          : m_defaultValuesAsStrings[vectorDefault ? element : 0];
}

const std::vector<std::string> DoubleItemDefinition::defaultValuesAsStrings() const
{
  return m_defaultValuesAsStrings;
}

bool DoubleItemDefinition::setDefaultValue(const double& val, const std::string& units)
{
  std::vector<double> defaultTuple(1, val);
  return this->setDefaultValue(defaultTuple, units);
}

bool DoubleItemDefinition::setUnits(const std::string& newUnits)
{
  if (newUnits == m_units)
  {
    return true;
  }
  std::string origUnits = m_units;
  m_units = newUnits;
  if (!this->reevaluateDefaults())
  {
    // Could not set new units
#ifndef NDEBUG
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Failed to set units of \"" << newUnits << "\" for item definition \"" << this->name()
                                  << "\"");
#endif
    m_units = origUnits;
    return false;
  }
  return true;
}

bool DoubleItemDefinition::setDefaultValue(
  const std::vector<double>& vals,
  const std::string& units)
{
  if (
    vals.empty() ||
    ((vals.size() > 1) &&
     (this->isDiscrete() || this->isExtensible() ||
      (vals.size() != this->numberOfRequiredValues()))))
  {
    return false; // *some* value must be provided or only fixed-size attributes can have vector defaults.
  }

  std::vector<std::string> stringVals(vals.size());
  // Do we need to do unit conversion?
  if (units != m_units)
  {
    // Are either unit-less?
    if (units.empty() || m_units.empty())
    {
      return false;
    }
    // do we have a unit system?
    if (m_unitsSystem)
    {
      // Are the units compatible?
      bool status;
      auto defUnit = m_unitsSystem->unit(m_units, &status);
      if (!status)
      {
        return false; // Could not find the definition's units
      }
      auto valUnit = m_unitsSystem->unit(units, &status);
      if (!status)
      {
        return false; // Could not find the default vals' units
      }
      auto converter = m_unitsSystem->convert(valUnit, defUnit);
      if (!converter)
      {
        return false; // Could not find a conversion between the units
      }
      std::vector<double> convertedVals(vals.size());
      // Convert all of the values
      converter->transform(vals.begin(), vals.end(), convertedVals.begin());
      // Now check to make sure these values are valid w/r to the Definition
      for (std::size_t i = 0; i < convertedVals.size(); i++)
      {
        if (!this->isValueValid(convertedVals[i]))
        {
          return false;
        }
        std::stringstream ss;
        ss.precision(17);
        ss << convertedVals[i] << " " << units;
        stringVals[i] = ss.str();
      }
      m_defaultValue = convertedVals;
      m_defaultValuesAsStrings = stringVals;
      m_hasDefault = true;
      return true;
    }
    // In this case there is no units system but we have been told to use units
    return false;
  }

  // In this case we don't need to do unit conversion at all
  typename std::vector<double>::const_iterator it;
  for (std::size_t i = 0; i < vals.size(); i++)
  {
    if (!this->isValueValid(vals[i]))
    {
      return false; // Is each value valid?
    }
    std::stringstream ss;
    ss.precision(17);
    ss << vals[i];
    if (!units.empty())
    {
      ss << " " << units;
    }
    stringVals[i] = ss.str();
  }
  m_defaultValue = vals;
  m_defaultValuesAsStrings = stringVals;
  m_hasDefault = true;
  return true;
}

bool DoubleItemDefinition::setDefaultValueAsString(const std::string& val)
{
  std::vector<std::string> defaultTuple(1, val);
  return this->setDefaultValueAsString(defaultTuple);
}

bool DoubleItemDefinition::setDefaultValueAsString(const std::vector<std::string>& vals)
{
  if (
    vals.empty() ||
    ((vals.size() > 1) &&
     (this->isDiscrete() || this->isExtensible() ||
      (vals.size() != this->numberOfRequiredValues()))))
  {
    return false; // *some* value must be provided or only fixed-size attributes can have vector defaults.
  }

  std::vector<std::string> origDefaults = m_defaultValuesAsStrings;
  m_defaultValuesAsStrings = vals;
  if (!this->reevaluateDefaults())
  {
    m_defaultValuesAsStrings = origDefaults;
    return false;
  }
  m_hasDefault = true;
  return true;
}

bool DoubleItemDefinition::splitStringStartingDouble(
  const std::string& input,
  std::string& valueString,
  std::string& unitsString)
{
  valueString.clear();
  unitsString.clear();

  // Try streaming double value
  std::istringstream iss(input);
  double value;
  iss >> value;
  if ((iss.bad()) || iss.fail())
  {
    return false;
  }

  if (iss.eof())
  {
    valueString = input;
    return true;
  }

  std::size_t pos = iss.tellg();
  valueString = input.substr(0, pos);
  unitsString = input.substr(pos);

  // Trim away any white space
  smtk::common::StringUtil::trim(valueString);
  smtk::common::StringUtil::trim(unitsString);
  return true;
}

bool DoubleItemDefinition::reevaluateDefaults()
{
  std::vector<double> convertedVals(m_defaultValuesAsStrings.size());
  std::string units;
  units::Unit defUnit;
  bool convert = false;
  if (m_unitsSystem)
  {
    // If we have an units System, lets see if the definition's units
    // are valid?
    defUnit = m_unitsSystem->unit(m_units, &convert);
  }

  // Can we not do conversion?
  if (!convert)
  {
    // In this case the defaults should either not have units or have units that match the definition
    for (std::size_t i = 0; i < m_defaultValuesAsStrings.size(); i++)
    {
      std::string valStr, unitsStr;
      if (!DoubleItemDefinition::splitStringStartingDouble(
            m_defaultValuesAsStrings[i], valStr, unitsStr))
      {
        return false; // String is badly formatted
      }
      // If we have units, do they match?
      if (!(unitsStr.empty() || (unitsStr == m_units)))
      {
        return false;
      }
      // Is the value valid w/r to the Definition's constraints?
      std::stringstream convertToDouble(valStr);
      convertToDouble.precision(17);
      if (!((convertToDouble >> convertedVals[i]) && this->isValueValid(convertedVals[i])))
      {
        return false; // failed to get the double or it was not considered valid
      }
      // If there were units specified, drop them since they must match the Definition's and are not needed
      if (!unitsStr.empty())
      {
        m_defaultValuesAsStrings[i] = valStr;
      }
    }
  }
  else
  {
    // OK we can do units conversion
    bool status;
    double convertedVal;
    for (std::size_t i = 0; i < m_defaultValuesAsStrings.size(); i++)
    {
      auto valMeasure = m_unitsSystem->measurement(m_defaultValuesAsStrings[i], &status);
      if (!status)
      {
        // Could not parse the value
        return false;
      }
      if (!valMeasure.m_units.dimensionless())
      {
        auto convertedMeasure = m_unitsSystem->convert(valMeasure, defUnit, &status);
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
        // We should add the units to the string if not empty
        if (!m_units.empty())
        {
          m_defaultValuesAsStrings[i].append(" ").append(m_units);
        }
      }
      if (!this->isValueValid(convertedVal))
      {
        return false;
      }
      convertedVals[i] = convertedVal;
    }
  }
  m_defaultValue = convertedVals;
  return true;
}
