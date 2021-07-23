//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <set>
#include <string>

const std::string attTemplate =
  "<SMTK_AttributeResource Version=\"4\">\n"
  "  <Categories>\n"
  "    <Cat>Flow</Cat>\n"
  "  </Categories>\n"
  "  <Definitions>\n"
  "    <AttDef Type=\"Test\">\n"
  "      <Categories>\n"
  "        <Cat>Flow</Cat>\n"
  "      </Categories>\n"
  "      <ItemDefinitions>\n"
  "        <String Name=\"String Item\" Optional=\"true\" IsEnabledByDefault=\"false\" />\n"
  "      </ItemDefinitions>\n"
  "    </AttDef>\n"
  "  </Definitions>\n"
  "  <Attributes>\n"
  "    <Att Type=\"Test\" Name=\"Test1\" />"
  "  </Attributes>\n"
  "</SMTK_AttributeResource>\n";

/* This test verifies that Attribute::isValid() accounts for categories,
 * enabled state, and ignored state of items.
 */

int unitIsValid(int /*unused*/, char* /*unused*/[])
{
  auto attResource = smtk::attribute::Resource::create();
  smtk::io::AttributeReader reader;
  auto logger = smtk::io::Logger::instance();
  bool err = reader.readContents(attResource, attTemplate, logger);
  smtkTest(!err, "Error reading template.");

  auto att = attResource->findAttribute("Test1");
  smtkTest(att != nullptr, "Test1 attribute not found");
  auto sitem = att->findString("String Item");
  smtkTest(sitem != nullptr, "String Item not found");

  // Test with no categories
  sitem->setIsEnabled(false);
  smtkTest(att->isValid(), "No-categories Test1 should be valid when string item disabled.");

  sitem->setIsEnabled(true);
  smtkTest(!att->isValid(), "No-categories Test1 should be invalid when string item enabled.");

  sitem->setIsIgnored(true);
  smtkTest(att->isValid(), "No-categories Test1 should be valid when string item ignored.");
  sitem->setIsIgnored(false);

  // Test with categories passed in
  std::set<std::string> categories = { "Flow" };

  sitem->setIsEnabled(false);
  smtkTest(
    att->isValid(categories), "With-categories Test1 should be valid when string item disabled.");

  sitem->setIsEnabled(true);
  smtkTest(
    !att->isValid(categories), "With-categories Test1 should be invalid when string item enabled.");

  sitem->setIsIgnored(true);
  smtkTest(
    att->isValid(categories), "With-categories Test1 should be valid when string item ignored.");
  sitem->setIsIgnored(false);

  // Test with categories set on resource
  attResource->setActiveCategoriesEnabled(true);
  attResource->setActiveCategories(categories);

  sitem->setIsEnabled(false);
  smtkTest(att->isValid(), "Active-categories Test1 should be valid when string item disabled.");

  sitem->setIsEnabled(true);
  smtkTest(!att->isValid(), "Active-categories Test1 should be invalid when string item enabled.");

  sitem->setIsIgnored(true);
  smtkTest(att->isValid(), "Active-categories Test1 should be valid when string item ignored.");
  sitem->setIsIgnored(false);

  return 0;
}
