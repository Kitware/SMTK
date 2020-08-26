//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/attributeUtils.h"
#include "smtk/io/Logger.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/common/StringUtil.h"

#include <fstream>
#include <vector>

using namespace smtk::attribute;

namespace smtk
{
namespace io
{
bool importFromCSV(smtk::attribute::GroupItem& item, const std::string& filename, Logger& logger,
  bool appendToGroup, const std::string& sep, const std::string& comment)
{
  // First lets determine the number of values each group as well as determining if the
  // group is acceptable for import - this means the following:
  //   1. Group is extensible
  //   2. Group's children are ValueItems and are not optional and not extensible
  //
  // This will ignore ValueItem children.

  auto def = item.definitionAs<GroupItemDefinition>();
  if (!def->isExtensible())
  {
    smtkErrorMacro(logger,
      "GroupItem: " << item.name() << " is not extensible and is inappropriate for CSV Import");
    return false;
  }
  std::size_t numValues = 0, numItemsPerGroup = def->numberOfItemDefinitions();
  for (std::size_t i = 0; i < numItemsPerGroup; i++)
  {
    auto vdef = std::dynamic_pointer_cast<ValueItemDefinition>(def->itemDefinition(i));
    if ((vdef == nullptr) || vdef->isOptional() || vdef->isExtensible())
    {
      smtkErrorMacro(logger,
        "GroupItem: " << item.name() << " is not appropriate for CSV Import due to child item "
                      << vdef->name() << " either not being a ValueItem or it is either extensible "
                                         "or optional which are not supported.");
      return false;
    }
    numValues += vdef->numberOfRequiredValues();
  }

  // OK now lets open the file
  std::ifstream csv(filename);
  if (!csv.is_open())
  {
    smtkErrorMacro(logger, "Could not open CSV File: " << filename);
    return false;
  }
  //Process the file - we will skip any lines that do not contain numValues;
  std::string line;
  int lineCount = 0;
  std::size_t groupIndex = 0;
  std::size_t numGroups = item.numberOfGroups();
  if (appendToGroup)
  {
    groupIndex = numGroups;
  }

  while (std::getline(csv, line))
  {
    // See if the line is empty or first character matches comment -  skip it
    if (line.empty() || ((!comment.empty()) && (line[0] == comment[0])))
    {
      lineCount++;
      continue;
    }
    // First split the line into components using ","
    std::vector<std::string> svals = smtk::common::StringUtil::split(line, sep, false, true);
    if (svals.size() != numValues)
    {
      smtkWarningMacro(logger, "Skipping line "
          << lineCount << ": \"" << line << "\" - incorrect number of values. Found "
          << svals.size() << " should have been " << numValues);
      lineCount++;
      continue;
    }
    std::size_t currentVal = 0;
    // Are we appending a new group?
    if (groupIndex >= numGroups)
    {
      item.appendGroup();
    }
    // Lets go through each item in that group and set it
    for (std::size_t i = 0; i < numItemsPerGroup; i++)
    {
      auto vitem = std::dynamic_pointer_cast<ValueItem>(item.item(groupIndex, i));
      std::size_t numValuesForItem = vitem->numberOfValues();
      for (std::size_t j = 0; j < numValuesForItem; j++)
      {
        if (!vitem->setValueFromString(j, svals.at(currentVal++)))
        {
          smtkErrorMacro(logger, "Could not set Group[" << groupIndex << "]'s Item "
                                                        << vitem->name() << "[" << j << "] to "
                                                        << svals.at(currentVal - 1));
        }
      }
    }
    // Move to next group
    groupIndex++;
    lineCount++;
  }
  return true;
}

bool importFromDoubleFile(smtk::attribute::GroupItem& item, const std::string& filename,
  Logger& logger, bool appendToGroup, const std::string& optionalSep, const std::string& comment)
{
  // First lets determine the number of values each group as well as determining if the
  // group is acceptable for import - this means the following:
  //   1. Group is extensible
  //   2. Group's children are DoubleItems and are not optional and not extensible
  //
  // This will ignore ValueItem children.

  auto def = item.definitionAs<GroupItemDefinition>();
  if (!def->isExtensible())
  {
    smtkErrorMacro(logger,
      "GroupItem: " << item.name() << " is not extensible and is inappropriate for CSV Import");
    return false;
  }
  std::size_t numValues = 0, numItemsPerGroup = def->numberOfItemDefinitions();
  for (std::size_t i = 0; i < numItemsPerGroup; i++)
  {
    auto ddef = std::dynamic_pointer_cast<DoubleItemDefinition>(def->itemDefinition(i));
    if ((ddef == nullptr) || ddef->isOptional() || ddef->isExtensible())
    {
      smtkErrorMacro(logger, "GroupItem: "
          << item.name() << " is not appropriate for Double File  Import due to child item "
          << ddef->name() << " either not being a DoubleItem or it is either extensible "
                             "or optional which are not supported.");
      return false;
    }
    numValues += ddef->numberOfRequiredValues();
  }

  // OK now lets open the file
  std::ifstream doublefile(filename);
  if (!doublefile.is_open())
  {
    smtkErrorMacro(logger, "Could not open Double File: " << filename);
    return false;
  }
  //Process the file - we will skip any lines that do not contain numValues;
  std::string line;
  int lineCount = 0;
  std::size_t groupIndex = 0;
  std::size_t numGroups = item.numberOfGroups();
  if (appendToGroup)
  {
    groupIndex = numGroups;
  }

  std::vector<double> vals(numValues);
  while (std::getline(doublefile, line))
  {
    // See if the line is empty or first character matches comment -  skip it
    if (line.empty() || ((!comment.empty()) && (line[0] == comment[0])))
    {
      lineCount++;
      continue;
    }
    // Since the file may have text between the values in the line we want do the following:
    //  1. Skip white space
    //  2. read in string and test for double - if not a valid double skip it

    std::istringstream iss(line);
    // set stream to skip white space
    iss >> std::skipws;
    int numFound = 0;
    while (!((static_cast<std::size_t>(numFound) == numValues) || iss.eof()))
    {
      // lets try to read the next double
      iss >> vals[numFound];
      if (iss.fail())
      {
        // Ok did we encounter the optional separator?
        iss.clear();
        if (optionalSep.empty() || (iss.peek() != optionalSep[0]))
        {
          break; // encountered a problem
        }
        else
        {
          iss.ignore(); // skip the separator
        }
      }
      else
      {
        ++numFound; // we found a double
      }
    }
    if (static_cast<std::size_t>(numFound) != numValues)
    {
      // Did we encounter the end of line?
      if (iss.eof())
      {
        smtkWarningMacro(logger, "Skipping line "
            << lineCount << ": \"" << line << "\" - incorrect number of values. Found " << numFound
            << " should have been " << numValues);
      }
      else
      {
        std::string probString;
        std::getline(iss, probString);
        smtkWarningMacro(logger, "Skipping line " << lineCount << ": \"" << line
                                                  << "\" - encountered a format issue near \""
                                                  << probString << "\"");
      }
      lineCount++;
      continue;
    }
    // OK lets process the values we read in
    std::size_t currentVal = 0;

    // Are we appending a new group?
    if (groupIndex >= numGroups)
    {
      item.appendGroup();
    }

    // Lets go through each item in that group and set it
    for (std::size_t i = 0; i < numItemsPerGroup; i++)
    {
      auto ditem = std::dynamic_pointer_cast<DoubleItem>(item.item(groupIndex, i));
      std::size_t numValuesForItem = ditem->numberOfValues();
      for (std::size_t j = 0; j < numValuesForItem; j++)
      {
        if (!ditem->setValue(j, vals.at(currentVal++)))
        {
          smtkErrorMacro(logger, "Could not set Group[" << groupIndex << "]'s Item "
                                                        << ditem->name() << "[" << j << "] to "
                                                        << vals.at(currentVal - 1));
        }
      }
    }
    // Move to next group
    groupIndex++;
    lineCount++;
  }
  return true;
}
}
}
