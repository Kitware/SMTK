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
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

// Test the Definition to see that it has the proper structure that was generated using Templates.
// It should consist of 3 string Items:
// s1 - default set to cat
// s2 - default set to dog (the Template default)
// s3 - discrete with an default index of 1

int unitTemplates(int /*unused*/, char* /*unused*/[])
{
  // Read in the test configurations files
  std::string attFile;
  attFile = SMTK_DATA_DIR;
  attFile += "/attribute/attribute_collection/TemplateTest.sbt";
  io::AttributeReader reader;
  io::Logger logger;
  auto attRes = attribute::Resource::create();
  reader.read(attRes, attFile, logger);
  if (logger.hasErrors())
  {
    std::cerr << "Errors Generated when reading SBT file :\n" << logger.convertToString();
  }

  DefinitionPtr attDef = attRes->findDefinition("A");
  if (attDef == nullptr)
  {
    std::cerr << "Error - Could not find Definition: A\n";
    return -1;
  }

  // Test item s1
  auto sitem = smtk::dynamic_pointer_cast<StringItemDefinition>(attDef->itemDefinition(0));
  if (!sitem)
  {
    std::cerr << "Error - Could not find StringItemDefinition[0]\n";
    return -1;
  }

  if (sitem->name() != "s1")
  {
    std::cerr << "Error - StringItemDefinition[0] was " << sitem->name()
              << " should have been s1\n";
    return -1;
  }

  if (sitem->defaultValue() != "cat")
  {
    std::cerr << "Error - StringItemDefinition: s1's default was " << sitem->defaultValue()
              << " should have been cat\n";
    return -1;
  }

  // Test item s2
  sitem = smtk::dynamic_pointer_cast<StringItemDefinition>(attDef->itemDefinition(1));
  if (!sitem)
  {
    std::cerr << "Error - Could not find StringItemDefinition[1]\n";
    return -1;
  }

  if (sitem->name() != "s2")
  {
    std::cerr << "Error - StringItemDefinition[1] was " << sitem->name()
              << " should have been s2\n";
    return -1;
  }

  if (sitem->defaultValue() != "dog")
  {
    std::cerr << "Error - StringItemDefinition: s2's default was " << sitem->defaultValue()
              << " should have been dog\n";
    return -1;
  }

  // Test item s3
  sitem = smtk::dynamic_pointer_cast<StringItemDefinition>(attDef->itemDefinition(2));
  if (!sitem)
  {
    std::cerr << "Error - Could not find StringItemDefinition[2]\n";
    return -1;
  }

  if (sitem->name() != "s3")
  {
    std::cerr << "Error - StringItemDefinition[2] was " << sitem->name()
              << " should have been s3\n";
    return -1;
  }

  if (!sitem->isDiscrete())
  {
    std::cerr << "Error - StringItemDefinition: s3 is not discrete\n";
    return -1;
  }
  if (sitem->numberOfChildrenItemDefinitions() != 2)
  {
    std::cerr << "Error - StringItemDefinition: s3 has " << sitem->numberOfChildrenItemDefinitions()
              << " children definitions, should have been 2\n";
    return -1;
  }
  if (sitem->numberOfDiscreteValues() != 2)
  {
    std::cerr << "Error - StringItemDefinition: s3 has " << sitem->numberOfDiscreteValues()
              << " discrete values, should have been 2\n";
    return -1;
  }
  if (sitem->defaultDiscreteIndex() != 1)
  {
    std::cerr << "Error - StringItemDefinition: s3's default index is "
              << sitem->defaultDiscreteIndex() << ", should have been 1\n";
    return -1;
  }
  std::cerr << "All Tests Passed!\n";
  return 0;
}
