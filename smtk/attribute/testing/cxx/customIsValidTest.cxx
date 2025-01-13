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
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <set>
#include <string>

const std::string attTemplate = R"(
<SMTK_AttributeResource Version="4">
  <Categories>
    <Cat>Flow</Cat>
  </Categories>
  <Definitions>
    <AttDef Type="Test">
      <Categories>
        <Cat>Flow</Cat>
      </Categories>
      <ItemDefinitions>
        <String Name="String Item" Optional="true" IsEnabledByDefault="false" />
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Type="Test" Name="Test1" />
  </Attributes>
</SMTK_AttributeResource>
)";

using namespace smtk::attribute;

bool isValidDefault(const Item* item, const std::set<std::string>& categories)
{
  // This function determines the item validity based on the defaultIsValid implementation.
  return item->defaultIsValid(categories);
}

bool isValidCustom(const Item* item, const std::set<std::string>& categories)
{
  // This function determines the item validity based on its string item value.

  // isValid should get a set of active categories is useActiveCategories is true.
  smtkTest(
    categories.find("Flow") != categories.end(), "isValidCustom should get active categories.");

  const auto att = item->attribute();
  if (att)
  {
    const auto stringItem = att->findAs<StringItem>("String Item");
    if (stringItem)
    {
      if (!stringItem->isSet())
      {
        // The string item was never set.
        return false;
      }

      // Return true if the string item is "valid".
      return stringItem->value() == "valid";
    }
  }

  // Do default behavior if we don't have an attribute.
  return item->defaultIsValid(categories);
}

bool alwaysValid(const Item*, const std::set<std::string>&)
{
  // Reports the attribute item as always valid.
  // Bypasses disabled and ignored checks.
  return true;
}

bool neverValid(const Item*, const std::set<std::string>&)
{
  // Reports the attribute item as never valid.
  return false;
}

int main()
{
  auto attResource = smtk::attribute::Resource::create();
  smtk::io::AttributeReader reader;
  auto logger = smtk::io::Logger::instance();
  bool error = reader.readContents(attResource, attTemplate, logger);
  smtkTest(!error, "Error reading template.");

  auto att = attResource->findAttribute("Test1");
  smtkTest(att != nullptr, "Test1 attribute not found");
  auto stringItem = att->findString("String Item");
  smtkTest(stringItem != nullptr, "String Item not found");

  // Test with no categories.
  stringItem->setIsEnabled(false);
  smtkTest(
    att->isValid(), "Without custom validity, Test1 should be valid when string item is disabled.");

  stringItem->setIsEnabled(true);
  stringItem->setCustomIsValid(&alwaysValid);
  smtkTest(
    att->isValid(),
    "With \"alwaysValid\" validity, Test1 should be valid even when string item is enabled.");

  stringItem->setIsIgnored(true);
  stringItem->setIsEnabled(false);
  stringItem->setCustomIsValid(&neverValid);
  smtkTest(
    !att->isValid(),
    "With \"neverValid\" validity, Test1 should be invalid even when string item is ignored or "
    "disabled.");
  stringItem->setIsIgnored(false);
  stringItem->setIsEnabled(true);

  // Test custom logic with categories passed in.
  stringItem->setCustomIsValid(&isValidCustom);
  std::set<std::string> categories = { "Flow" };

  smtkTest(
    !att->isValid(categories),
    "With categories passed in, Test1 should be invalid when string item is unset.");

  stringItem->setValue("valid");
  smtkTest(
    att->isValid(categories),
    "With categories passed in, Test1 should be valid when string item is \"valid\".");

  stringItem->setValue("invalid");
  smtkTest(
    !att->isValid(categories),
    "With categories passed in, Test1 should be invalid when string item is \"invalid\".");
  stringItem->unset();

  // Test the defaultIsValid implementation.
  stringItem->setCustomIsValid(&isValidDefault);

  stringItem->setIsEnabled(false);
  smtkTest(
    att->isValid(categories),
    "With default validity, Test1 should be valid when string item disabled.");

  stringItem->setIsEnabled(true);
  smtkTest(
    !att->isValid(categories),
    "With default validity, Test1 should be invalid when string item enabled.");

  stringItem->setIsIgnored(true);
  smtkTest(
    att->isValid(categories),
    "With default validity, Test1 should be valid when string item ignored.");
  stringItem->setIsIgnored(false);

  // Test with categories set on resource.
  attResource->setActiveCategoriesEnabled(true);
  attResource->setActiveCategories(categories);

  // Test custom logic with active categories.
  stringItem->setCustomIsValid(&isValidCustom);

  smtkTest(
    !att->isValid(), "With active categories, Test1 should be invalid when string item is unset.");

  stringItem->setValue("valid");
  smtkTest(
    att->isValid(), "With active categories, Test1 should be valid when string item is \"valid\".");

  stringItem->setValue("invalid");
  smtkTest(
    !att->isValid(),
    "With active categories, Test1 should be invalid when string item is \"invalid\".");

  return 0;
}
