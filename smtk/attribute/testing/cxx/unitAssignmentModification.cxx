//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/CopyAssignmentOptions.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
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
    <AttDef Type="Foo" Version="1">
    </AttDef>
    <AttDef Type="Test" Version="1">
      <AssociationsDef Name="Foos" Version="1" NumberOfRequiredValues="0" Extensible="true">
        <Accepts>
          <Resource Name="smtk::attribute::Resource" Filter="*" />
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <Int    Version="2" Name="ObjectiveIQ"/>
        <String Version="1" Name="SubjectiveIQ" Optional="true" IsEnabledByDefault="false" />
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Type="Foo" Name="foo1"/>
    <Att Type="Foo" Name="foo2"/>
    <Att Type="Test" Name="test1">
      <Items>
        <Int Name="ObjectiveIQ">37</Int>
        <String Name="SubjectiveIQ" IsEnabled="true">Dumb</String>
      </Items>
    </Att>
    <Att Type="Test" Name="test2">
      <Items>
        <Int Name="ObjectiveIQ">100</Int>
        <String Name="SubjectiveIQ" IsEnabled="true">Average</String>
      </Items>
    </Att>
  </Attributes>
</SMTK_AttributeResource>
)";

// This test verifies that Item::assign() returns a status object that
// properly reflects whether the assignment resulted in a modification
// of the target item or not.
int unitAssignmentModification(int /*unused*/, char* /*unused*/[])
{
  using namespace smtk::attribute;

  auto managers = smtk::common::Managers::create();
  auto resourceManager = smtk::resource::Manager::create();
  auto operationManager = smtk::operation::Manager::create();
  managers->insert_or_assign(resourceManager);
  managers->insert_or_assign(operationManager);

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  auto attributeRegistry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(
    managers,
    managers->get<smtk::resource::Manager::Ptr>(),
    managers->get<smtk::operation::Manager::Ptr>());

  auto logger = smtk::io::Logger::instance();
  smtk::io::AttributeReader reader;

  // Load an initial attribute resource
  auto attRsrc = smtk::attribute::Resource::create();
  bool err = reader.readContents(attRsrc, attV1, logger);
  ::test(!err, "Error reading template.");

  auto att1 = attRsrc->findAttribute("test1");
  ::test(!!att1, "test1 attribute not found");
  auto att1StrItem = att1->findString("SubjectiveIQ");
  ::test(!!att1StrItem, "String item not found");
  auto att1IntItem = att1->findInt("ObjectiveIQ");
  ::test(!!att1IntItem, "Int item not found");

  // Associate some items to att1
  auto foo1 = attRsrc->findAttribute("foo1");
  auto foo2 = attRsrc->findAttribute("foo2");
  ::test(att1->associate(foo1), "Could not associate foo1 to test1.");
  ::test(att1->associate(foo2), "Could not associate foo2 to test1.");

  std::cout << "Resource template type \"" << (attRsrc->templateType()).data() << "\" "
            << "v" << attRsrc->templateVersion() << "\n"
            << "  Attribute type \"" << att1->type() << "\" "
            << "v" << att1->definition()->version() << "\n";

  auto att2 = attRsrc->findAttribute("test2");
  ::test(!!att2, "test2 attribute not found");
  auto att2StrItem = att2->findString("SubjectiveIQ");
  ::test(!!att2StrItem, "String item not found");
  auto att2IntItem = att2->findInt("ObjectiveIQ");
  ::test(!!att2IntItem, "Int item not found");

  smtk::attribute::Item::Status status;
  smtk::attribute::CopyAssignmentOptions options;

  status = att2StrItem->assign(att1StrItem, options);
  ::test(status.success(), "String item assignment failed.");
  ::test(status.modified(), "String item not modified on assignment.");

  status = att2IntItem->assign(att1IntItem, options);
  ::test(status.success(), "Integer item assignment failed.");
  ::test(status.modified(), "Integer item not modified on assignment.");

  status = att2->associations()->assign(att1->associations());
  ::test(status.success(), "Reference item assignment failed.");
  ::test(status.modified(), "Reference item not modified on assignment.");

  // Now copy the values back to the first attribute.
  // This should be a no-op.
  status = att1StrItem->assign(att2StrItem, options);
  ::test(status.success(), "String item assignment failed.");
  ::test(!status.modified(), "String item modified on no-op assignment.");

  status = att1IntItem->assign(att2IntItem, options);
  ::test(status.success(), "Integer item assignment failed.");
  ::test(!status.modified(), "Integer item modified on no-op assignment.");

  status = att1->associations()->assign(att2->associations());
  ::test(status.success(), "Reference item assignment failed.");
  ::test(!status.modified(), "Reference item modified on no-op assignment.");

  return 0;
}
