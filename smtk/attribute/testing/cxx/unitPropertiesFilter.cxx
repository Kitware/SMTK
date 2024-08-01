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
#include "smtk/attribute/Resource.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

namespace
{
const double double_epsilon = 1.e-10;
}

int unitPropertiesFilter(int /*unused*/, char* /*unused*/[])
{
  // Read in the test template
  std::string attFile;
  attFile = SMTK_DATA_DIR;
  attFile += "/attribute/attribute_collection/propertiesFilterExample.sbt";
  io::AttributeReader reader;
  io::Logger logger;
  auto attRes = attribute::Resource::create();
  reader.read(attRes, attFile, logger);
  if (logger.hasErrors())
  {
    std::cerr << "Errors Generated when reading SBT file :\n" << logger.convertToString();
    return -1;
  }
  std::cerr << std::boolalpha; // To print out booleans

  // Lets find the definitions and attributes

  auto defBase = attRes->findDefinition("Base");
  auto defA = attRes->findDefinition("A");
  auto a = attRes->findAttribute("a");
  auto a1 = attRes->findAttribute("a1");

  smtkTest(defBase != nullptr, "Could not find Definition Base");
  smtkTest(defA != nullptr, "Could not find Definition A");
  smtkTest(a != nullptr, "Could not find Attribute a");
  smtkTest(a1 != nullptr, "Could not find Attribute a1");

  // Supported Component Headers include: *, any, attribute, smtk::attribute::Attribute,
  // and smtk::attribute::Definition
  auto filter = attRes->queryOperation("any[ int { 'alpha' }]");
  // All Definitions and Attributes should be considered having a property call alpha
  smtkTest(filter(*defBase), "Definition Base did not have property alpha");
  smtkTest(filter(*defA), "Definition A did not have property alpha");
  smtkTest(filter(*a), "Attribute a did not have property alpha");
  smtkTest(filter(*a1), "Attribute a1 did not have property alpha");
  std::cerr << "Passed Alpha Test\n";

  filter = attRes->queryOperation("definition[ int { 'alpha' }]");
  // All Definitions and Attributes should be considered having a property call alpha
  smtkTest(filter(*defBase), "Definition Base did not have property alpha");
  smtkTest(filter(*defA), "Definition A did not have property alpha");
  smtkTest(!filter(*a), "Attribute a passed definition only test");
  smtkTest(!filter(*a1), "Attribute a1 passed definition only test");
  std::cerr << "Passed Alpha Test - Definition Only\n";

  filter = attRes->queryOperation("smtk::attribute::Attribute[ int { 'alpha' }]");
  // All Definitions and Attributes should be considered having a property call alpha
  smtkTest(!filter(*defBase), "Definition Base passed attribute only test");
  smtkTest(!filter(*defA), "Definition A passed attribute only test");
  smtkTest(filter(*a), "Attribute a did not have property alpha");
  smtkTest(filter(*a1), "Attribute a1 did not have property alpha");
  std::cerr << "Passed Alpha Test - Attribute Only\n";

  filter = attRes->queryOperation("any[ int { 'alpha' = 100 }]");
  // Only the Base should pass this test since everyone else redefines alpha
  smtkTest(filter(*defBase), "Definition Base did not have property alpha = 100");
  smtkTest(!filter(*defA), "Definition A did  have property alpha = 100 but shouldn't have");
  smtkTest(!filter(*a), "Attribute a did have property alpha = 100 but shouldn't have");
  smtkTest(!filter(*a1), "Attribute a1 did have property alpha = 100 but shouldn't have");
  std::cerr << "Passed Alpha = 100 Test\n";

  filter = attRes->queryOperation("any[ int { 'alpha' = 200 }]");
  // A and a1 should pass
  smtkTest(!filter(*defBase), "Definition Base did have property alpha = 200 but shouldn't have");
  smtkTest(filter(*defA), "Definition A did not have property alpha = 200");
  smtkTest(!filter(*a), "Attribute a did have property alpha = 200 but shouldn't have");
  smtkTest(filter(*a1), "Attribute a1 did not have property alpha = 200");
  std::cerr << "Passed Alpha = 200 Test\n";

  filter = attRes->queryOperation("any[ int { 'alpha' = 500 }]");
  // only Attribute a should pass
  smtkTest(!filter(*defBase), "Definition Base did have property alpha = 500 but shouldn't have");
  smtkTest(!filter(*defA), "Definition A did have property alpha = 500 but shouldn't have");
  smtkTest(filter(*a), "Attribute a did not have property alpha = 500");
  smtkTest(!filter(*a1), "Attribute a1 did have property alpha = 500 but shouldn't have");
  std::cerr << "Passed Alpha = 500 Test\n";

  filter = attRes->queryOperation("any[ string { 'beta' = 'cat' }]");
  // Only the Base should pass this test since everyone else redefines beta
  smtkTest(filter(*defBase), "Definition Base did not have property beta = cat");
  smtkTest(!filter(*defA), "Definition A did have property beta = cat but shouldn't have");
  smtkTest(!filter(*a), "Attribute a did have property beta = cat but shouldn't have");
  smtkTest(!filter(*a1), "Attribute a1 did have property beta = cat but shouldn't have");
  std::cerr << "Passed Beta = cat Test\n";

  filter = attRes->queryOperation("any[ string { 'beta' = 'dog' }]");
  // Everything except Base should pass
  smtkTest(!filter(*defBase), "Definition Base did have property beta = dog but shouldn't have");
  smtkTest(filter(*defA), "Definition A did not have property beta = dog");
  smtkTest(filter(*a), "Attribute a did not have property beta = dog");
  smtkTest(filter(*a1), "Attribute a1 did not have property beta = dog");
  std::cerr << "Passed Beta = dog Test\n";

  filter = attRes->queryOperation("any[ floating-point { 'gamma' = 3.141 }]");
  // Everything should pass since they all get the value from the Base Def
  smtkTest(filter(*defBase), "Definition Base did not have property gamma = 3.141");
  smtkTest(filter(*defA), "Definition A did not have property gamma = 3.141");
  smtkTest(filter(*a), "Attribute a did not have property gamma = 3.141");
  smtkTest(filter(*a1), "Attribute a1 did not have property gamma = 3.141");
  std::cerr << "Passed Gamma = 3.141 Test\n";

  filter = attRes->queryOperation("any[ floating-point { 'delta' }]");
  // Everything should fail since this property does not exist.
  smtkTest(!filter(*defBase), "Definition Base did have property delta but shouldn't have");
  smtkTest(!filter(*defA), "Definition A did have property delta but shouldn't have");
  smtkTest(!filter(*a), "Attribute a did have property delta but shouldn't have");
  smtkTest(!filter(*a1), "Attribute a1 did have property delta but shouldn't have");
  std::cerr << "Passed Delta Test\n";

  return 0;
}
