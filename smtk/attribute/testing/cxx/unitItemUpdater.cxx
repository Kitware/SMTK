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
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/UpdateManager.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/operation/Manager.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <set>
#include <string>

const std::string attV1 = R"(
<SMTK_AttributeResource Version="6" TemplateType="Example" TemplateVersion="1">
  <Definitions>
    <AttDef Type="Test" Version="1">
      <ItemDefinitions>
        <Int    Version="2" Name="ObjectiveIQ"/>
        <String Version="1" Name="SubjectiveIQ" Optional="true" IsEnabledByDefault="false" />
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Type="Test" Name="Test1">
      <Items>
        <Int Name="ObjectiveIQ">37</Int>
        <String Name="SubjectiveIQ" IsEnabled="true">Dumb</String>
      </Items>
    </Att>
  </Attributes>
</SMTK_AttributeResource>
)";

const std::string attV2 = R"(
<SMTK_AttributeResource Version="6" TemplateType="Example" TemplateVersion="5">
  <Definitions>
    <AttDef Type="Test" Version="2">
      <ItemDefinitions>
        <String Version="10" Name="SubjectiveIQ" Optional="true" IsEnabledByDefault="true" />
        <Int    Version="10" Name="ObjectiveIQ"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
)";

int unitItemUpdater(int /*unused*/, char* /*unused*/[])
{
  using namespace smtk::attribute;

  auto managers = smtk::common::Managers::create();
  auto resourceManager = smtk::resource::Manager::create();
  auto operationManager = smtk::operation::Manager::create();
  managers->insertOrAssign(resourceManager);
  managers->insertOrAssign(operationManager);

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  auto attributeRegistry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(
    managers,
    managers->get<smtk::resource::Manager::Ptr>(),
    managers->get<smtk::operation::Manager::Ptr>());

  // Register item updaters
  // Note that this can happen before the items are defined because of the
  // new TemplateType member of attribute::Resource.
  auto updateManager = managers->get<smtk::attribute::UpdateManager::Ptr>();
  ::test(!!updateManager, "Expected the attribute registrar would create an attribute manager.");
  updateManager->itemUpdaters("Example", "Test")
    .registerUpdater(
      "SubjectiveIQ", 1, 9, 10, [](const Item& inRaw, Item& outRaw, smtk::io::Logger& log) {
        const auto* in = dynamic_cast<const StringItem*>(&inRaw);
        auto* out = dynamic_cast<StringItem*>(&outRaw);
        if (in && out)
        {
          smtkInfoMacro(log, "Converting SubjectiveIQ");
          out->setValue(in->value() + "er"); // Turn "Dumb" into "Dumber", "Smart" into "Smarter", …
          return true;
        }
        return false;
      });

  updateManager->itemUpdaters("Example", "Test")
    .registerUpdater(
      "ObjectiveIQ", 1, 9, 10, [](const Item& inRaw, Item& outRaw, smtk::io::Logger& log) {
        const auto* in = dynamic_cast<const IntItem*>(&inRaw);
        auto* out = dynamic_cast<IntItem*>(&outRaw);
        if (in && out)
        {
          smtkInfoMacro(log, "Converting ObjectiveIQ");
          out->setValue(in->value() + 5); // New "IQ" is 5 above old "IQ."
          return true;
        }
        return false;
      });

  auto logger = smtk::io::Logger::instance();
  smtk::io::AttributeReader reader;

  // Load an "old" attribute resource
  auto attRsrc1 = smtk::attribute::Resource::create();
  bool err = reader.readContents(attRsrc1, attV1, logger);
  ::test(!err, "Error reading template 1.");

  auto att1 = attRsrc1->findAttribute("Test1");
  ::test(!!att1, "Test1 attribute not found");
  auto att1StrItem = att1->findString("SubjectiveIQ");
  ::test(!!att1StrItem, "String item not found");
  auto att1IntItem = att1->findInt("ObjectiveIQ");
  ::test(!!att1IntItem, "Int item not found");

  std::cout << "Resource 1 template type \"" << (attRsrc1->templateType()).data() << "\" "
            << "v" << attRsrc1->templateVersion() << "\n"
            << "  Attribute 1 type \"" << att1->type() << "\" "
            << "v" << att1->definition()->version() << "\n";

  // Load a "new" version of the attribute (definitions only).
  auto attRsrc2 = smtk::attribute::Resource::create();
  err = reader.readContents(attRsrc2, attV2, logger);
  ::test(!err, "Error reading template 2.");

  // Create a default "Test1" attribute to hold the output of our item migration.
  auto att2 = attRsrc2->createAttribute("Test");
  ::test(!!att2, "Test1 attribute not found");
  auto att2StrItem = att2->findString("SubjectiveIQ");
  ::test(!!att2StrItem, "String item not found");
  auto att2IntItem = att2->findInt("ObjectiveIQ");
  ::test(!!att2IntItem, "Int item not found");

  // Now attempt to migrate items from att1 to att2.
  std::cout << "    Can we update \"" << att1->itemPath(att1StrItem) << "\"?\n";
  if (
    auto updater = updateManager->itemUpdaters(attRsrc1->templateType(), att1->type())
                     .find(
                       att1->itemPath(att1StrItem),
                       att2StrItem->definition()->version(),
                       att1StrItem->definition()->version()))
  {
    std::cout << "        Source item value:   " << att1StrItem->value() << "\n";
    std::cout << "        Previous item value: " << att2StrItem->value() << "\n";
    auto didUpdate = updater.Update(*att1StrItem, *att2StrItem, logger);
    std::cout << "        Updated item value:  " << att2StrItem->value() << "\n";
    ::test(didUpdate, "Expected item to update.");
    ::test(att2StrItem->value() == "Dumber", "Expected Dumb → Dumber.");
  }
  else
  {
    ::test(false, "Did not find updater for SubjectiveIQ.");
  }

  std::cout << "    Can we update \"" << att1->itemPath(att1IntItem) << "\"?\n";
  if (
    auto updater = updateManager->itemUpdaters(attRsrc1->templateType(), att1->type())
                     .find(
                       att1->itemPath(att1IntItem),
                       att2IntItem->definition()->version(),
                       att1IntItem->definition()->version()))
  {
    std::cout << "        Source item value:   " << att1IntItem->value() << "\n";
    std::cout << "        Previous item value: " << att2IntItem->value() << "\n";
    auto didUpdate = updater.Update(*att1IntItem, *att2IntItem, logger);
    std::cout << "        Updated item value:  " << att2IntItem->value() << "\n";
    ::test(didUpdate, "Expected item to update.");
    ::test(att2IntItem->value() == 42, "Expected 37 → 42.");
  }
  else
  {
    ::test(false, "Did not find updater for ObjectiveIQ.");
  }

  return 0;
}
