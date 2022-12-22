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
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

// Test the Definition to see that it has the proper structure that was generated using ItemBlocks.  One of the the ItemDefinitions (indicated by testItemPos), will have
// its categories tested which should also be the same as the Attribute Definition itself
bool testDef(
  attribute::ResourcePtr& attRes,
  const std::string& attName,
  const std::vector<std::string>& itemNames,
  const std::vector<std::string>& passCategories,
  const std::string& failedCategory,
  int testItemPos)
{
  bool status = true;
  DefinitionPtr attDef = attRes->findDefinition(attName);
  if (attDef == nullptr)
  {
    std::cerr << "Could not find Definition: " << attName << "\n";
    return false;
  }
  // attDef check to see if the attribute passed its categories
  for (const auto& cat : passCategories)
  {
    if (!attDef->categories().passes(cat))
    {
      std::cerr << attName << " does not pass " << cat << "\n";
      status = false;
    }
  }
  if (attDef->categories().passes(failedCategory))
  {
    std::cerr << attName << " should not have passed " << failedCategory << "\n";
    status = false;
  }

  if (attDef->numberOfItemDefinitions() != itemNames.size())
  {
    std::cerr << attName << " does not contain the correct items"
              << " found: ";
    int numIDefs = (int)attDef->numberOfItemDefinitions();
    for (int i = 0; i < numIDefs; ++i)
    {
      std::cerr << "\"" << attDef->itemDefinition(i)->name() << "\" ";
    }
    return false;
  }

  int numItems = (int)itemNames.size();
  for (int i = 0; i < numItems; i++)
  {
    auto item = attDef->itemDefinition(i);
    if (item->name() != itemNames[i])
    {
      std::cerr << attName << "[" << i << "] should be " << itemNames[i]
                << " - found: " << item->name() << std::endl;
      status = false;
    }
    if (i == testItemPos)
    {
      for (const auto& cat : passCategories)
      {
        if (!item->categories().passes(cat))
        {
          std::cerr << item->name() << " does not pass " << cat << "\n";
          status = false;
        }
      }
      if (item->categories().passes(failedCategory))
      {
        std::cerr << item->name() << " should not have passed " << failedCategory << "\n";
        status = false;
      }
    }
  }
  return status;
}

int unitItemBlocks(int /*unused*/, char* /*unused*/[])
{
  // Read in the test configurations files
  bool status = true;
  std::string attFile;
  attFile = SMTK_DATA_DIR;
  attFile += "/attribute/attribute_collection/ItemBlockTest.sbt";
  io::AttributeReader reader;
  io::Logger logger;
  auto attRes = attribute::Resource::create();
  reader.read(attRes, attFile, logger);
  if (logger.hasErrors())
  {
    std::cerr << "Errors Generated when reading SBT file :\n" << logger.convertToString();
  }

  std::vector<std::vector<std::string>> itemNames = { { "foo", "s1", "i1", "bar" },
                                                      { "s1", "i1", "str2" },
                                                      { "s1-globals2", "i1-globals2", "str2" },
                                                      { "s3-globals2", "i3-globals2", "str2" },
                                                      { "s2-globals1", "i2-globals1", "str2" },
                                                      { "s4-globals2", "i4-globals2", "str2" } };
  std::vector<std::vector<std::string>> passCats = {
    { "Solid Mechanics", "Fluid Flow" },    { "Solid Mechanics", "Heat Transfer" },
    { "Solid Mechanics", "Heat Transfer" }, { "Solid Mechanics", "Heat Transfer" },
    { "Solid Mechanics", "Heat Transfer" }, { "Solid Mechanics", "Heat Transfer" }
  };

  status = status && testDef(attRes, "Type0", itemNames[0], passCats[0], "Heat Transfer", 1);
  status = status && testDef(attRes, "Type1", itemNames[1], passCats[1], "Fluid Flow", 0);
  status = status && testDef(attRes, "Type2", itemNames[2], passCats[2], "Fluid Flow", 0);
  status = status && testDef(attRes, "Type3", itemNames[3], passCats[3], "Fluid Flow", 0);
  status = status && testDef(attRes, "Type4", itemNames[4], passCats[4], "Fluid Flow", 0);
  status = status && testDef(attRes, "Type5", itemNames[5], passCats[5], "Fluid Flow", 0);

  if (status)
  {
    std::cerr << "All Tests Passed!\n";
    return 0;
  }
  return -1;
}
