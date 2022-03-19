//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItemDefinition.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

namespace
{
bool testItem(StringItemPtr& item, int index, const std::vector<std::string>& vals);
bool testItem(GroupItemPtr& groupItem, int index, const std::vector<std::string>& vals);
bool testConfigiration(
  ResourcePtr& attRes,
  const std::string& configName,
  const std::vector<std::string>& vals)
{
  std::cerr << "Testing configuration: " << configName;
  auto config = attRes->findAttribute(configName);
  if (!config)
  {
    std::cerr << " - Can not find configuration - FAILED\n";
    return false;
  }
  // If the configuration is Test A - it should have advance read level 5
  // If the configuration is Test B - it should have advance write level 10
  // All others access should be 0
  if (configName == "Test A")
  {
    if (config->advanceLevel(0) != 5)
    {
      std::cerr << " - Has incorrect read access: " << config->advanceLevel(0)
                << " should be 5 - FAILED\n";
      return false;
    }
  }
  else if (config->advanceLevel(0))
  {
    std::cerr << " - Has incorrect read access: " << config->advanceLevel(0)
              << " should be 5 - FAILED\n";
    return false;
  }
  if (configName == "Test B")
  {
    if (config->advanceLevel(1) != 10)
    {
      std::cerr << " - Has incorrect write access: " << config->advanceLevel(1)
                << " should be 10 - FAILED\n";
      return false;
    }
  }
  else if (config->advanceLevel(1))
  {
    std::cerr << " - Has incorrect write access: " << config->advanceLevel(1)
              << " should be 5 - FAILED\n";
    return false;
  }
  //top level is exclusive so get the string item and process it
  auto s = std::dynamic_pointer_cast<StringItem>(config->item(0));
  if (testItem(s, 0, vals))
  {
    std::cerr << " - PASSED\n";
    return true;
  }
  return false;
}

bool testItem(GroupItemPtr& groupItem, int index, const std::vector<std::string>& vals)
{
  //Find the item in the group
  ItemPtr child = groupItem->find(0, vals[index]);
  if (!child)
  {
    std::cerr << "\tLevel: " << index << " could not find Analysis  " << vals[index]
              << " - FAILED\n";
    return false;
  }
  if (!child->isEnabled())
  {
    std::cerr << "\tLevel: " << index << " Analysis  " << vals[index]
              << " is not enabled - FAILED\n";
    return false;
  }
  int nextLevel = index + 1;
  // Are we at the end?
  if (nextLevel == static_cast<int>(vals.size()))
  {
    return true;
  }
  std::size_t n = groupItem->numberOfItemsPerGroup();
  if (n == 0)
  {
    std::cerr << "\tLevel: " << index << " has no children. - FAILED\n";
    return false;
  }
  for (std::size_t i = 0; i < n; i++)
  {
    auto sitem = std::dynamic_pointer_cast<StringItem>(groupItem->item(i));
    if (sitem)
    {
      if (!testItem(sitem, nextLevel, vals))
      {
        return false;
      }
    }
    else
    {
      auto gitem = std::dynamic_pointer_cast<GroupItem>(groupItem->item(i));
      if (gitem)
      {
        if (!testItem(gitem, nextLevel, vals))
        {
          return false;
        }
      }
      else
      {
        std::cerr << "\tLevel: " << index << " unexpected child item. - FAILED\n";
        return false;
      }
    }
  }
  return true;
}

bool testItem(StringItemPtr& item, int index, const std::vector<std::string>& vals)
{
  //Does the item point to the proper value?
  if (item->value() != vals[index])
  {
    std::cerr << "\tLevel: " << index << " was " << item->value() << " should be " << vals[index]
              << " - FAILED\n";
    return false;
  }
  int nextLevel = index + 1;
  // Are we at the end?
  if (nextLevel == static_cast<int>(vals.size()))
  {
    return true;
  }
  if (item->numberOfActiveChildrenItems() == 0)
  {
    std::cerr << "\tLevel: " << index << " has no children. - FAILED\n";
    return false;
  }
  auto sitem = std::dynamic_pointer_cast<StringItem>(item->activeChildItem(0));
  if (sitem)
  {
    return testItem(sitem, nextLevel, vals);
  }
  auto gitem = std::dynamic_pointer_cast<GroupItem>(item->activeChildItem(0));
  if (!gitem)
  {
    std::cerr << "\tLevel: " << index << " unexpected child item. - FAILED\n";
    return false;
  }
  return testItem(gitem, nextLevel, vals);
}
} // namespace

int unitAnalysisConfigurations(int /*unused*/, char* /*unused*/[])
{
  // Read in the test configurations files
  bool status = true;
  std::string attFile;
  attFile = SMTK_DATA_DIR;
  attFile += "/attribute/attribute_collection/analysisConfigTest.sbt";
  io::AttributeReader reader;
  io::Logger logger;
  auto attRes = attribute::Resource::create();
  reader.read(attRes, attFile, logger);
  std::cerr
    << "The log for reading in the template should complain about not being able to build:\n"
    << "Test C and Test C-E.  These configurations have intentional mistakes\n\n";
  if (logger.hasErrors())
  {
    std::cerr << "Errors Generated when reading SBT file :\n" << logger.convertToString();
  }
  std::vector<std::vector<std::string>> configs = {
    { "A" }, { "B" }, { "B", "B-D" }, { "C", "C-D" }, { "C", "C-E", "C-E-F" }
  };
  // The above test template should have contained the following Test Configurations:
  /*Test A
    - Set top level to A
  Test B
    - Set top level to B
  Test B-D
    - Set top level to B and turn on D
  Test C
    - Set top level to C - should not create a configuration
  Test C-D
    - Set top level to C and turn D on
  Test C-E
    - Set top level to C and turn E on - should not create a configuration
  Test C-E-F
    - Set top level to C and turn E on and select F
*/

  // Ok there should be 5 valid configuration attributes
  std::vector<smtk::attribute::AttributePtr> configurationAtts;
  attRes->findAttributes("Analysis", configurationAtts);
  std::cerr << "Number of Configurations Created: " << configurationAtts.size() << std::endl;
  for (auto& c : configurationAtts)
  {
    std::cerr << "\t" << c->name() << std::endl;
  }
  if (configurationAtts.size() != 5)
  {
    std::cerr << "Error: Incorrect number of configurations!\n";
    status = false;
  }
  // Lets test the configurations
  status &= testConfigiration(attRes, "Test A", configs[0]);
  status &= testConfigiration(attRes, "Test B", configs[1]);
  status &= testConfigiration(attRes, "Test B-D", configs[2]);
  status &= testConfigiration(attRes, "Test C-D", configs[3]);
  status &= testConfigiration(attRes, "Test C-E-F", configs[4]);

  io::AttributeWriter writer;
  std::string writeRroot(SMTK_SCRATCH_DIR);
  std::string fname = writeRroot + "/unitAnalysisConfigurationTest.sbi";
  writer.write(attRes, fname, logger);
  if (status)
  {
    std::cerr << "All Tests Passed!\n";
    return 0;
  }
  return -1;
}
