//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/OperationDecorator.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/xml/xmlConfiguration.h"

#include "smtk/operation/Manager.h"

#include "smtk/attribute/operators/Associate.h"
#include "smtk/attribute/operators/Dissociate.h"
#include "smtk/attribute/operators/Export.h"
#include "smtk/attribute/operators/Import.h"
#include "smtk/model/operators/AddAuxiliaryGeometry.h"
#include "smtk/model/operators/AddImage.h"
#include "smtk/model/operators/CompositeAuxiliaryGeometry.h"
#include "smtk/model/operators/CreateInstances.h"
#include "smtk/model/operators/Delete.h"
#include "smtk/model/operators/DivideInstance.h"
#include "smtk/model/operators/EntityGroupOperation.h"
#include "smtk/model/operators/GroupAuxiliaryGeometry.h"
#include "smtk/model/operators/MergeInstances.h"
#include "smtk/model/operators/SetInstancePrototype.h"
#include "smtk/operation/operators/AssignColors.h"
#include "smtk/operation/operators/SetProperty.h"

#include "smtk/common/testing/cxx/helpers.h"

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

namespace
{

using namespace pugi;

void testInitializerListCtor()
{
  using namespace smtk::view;

  OperationDecorator decorator(
    { wrap<smtk::operation::AssignColors>(),
      wrap<smtk::model::Delete>(),
      wrap<smtk::model::EntityGroupOperation>("edit group", "Edit groups", {}, "edit\ngroup"),
      wrap<smtk::model::GroupAuxiliaryGeometry>(),
      wrap<smtk::model::AddAuxiliaryGeometry>(),
      wrap<smtk::model::AddImage>(),
      wrap<smtk::model::CompositeAuxiliaryGeometry>(),
      wrap<smtk::model::CreateInstances>(),
      wrap<smtk::model::DivideInstance>(),
      wrap<smtk::model::MergeInstances>(),
      wrap<smtk::model::SetInstancePrototype>(),
      wrap<smtk::operation::SetProperty>(),
      wrap<smtk::attribute::Associate>(),
      wrap<smtk::attribute::Dissociate>(),
      wrap<smtk::attribute::Export>(),
      wrap<smtk::attribute::Import>() });

  auto foo = decorator.at<smtk::model::EntityGroupOperation>();
  const auto& bar = foo.second.get();
  std::cout << "label " << bar.m_label.c_str() << "\n";
  std::cout.flush();
  std::cout << "  " << bar.m_toolTip.c_str() << "\n  " << bar.m_buttonLabel.c_str() << "\n";
  test(foo.first, "Expected to find operation.");
  test(bar.m_label == "edit group", "Failed to override name.");
  test(decorator.size() == 16, "Expected to register 16 operations.");
  decorator.dump();

  std::cout << std::type_index(typeid(smtk::operation::Operation)).hash_code()
            << " base operation\n";
  foo = decorator.at<smtk::operation::Operation>();
  test(!foo.first, "Expected to not find an unregistered operation.");
}

void testConfigurationCtor()
{
  // Operations we'll register with an operation manager:
  using OperationList = std::tuple<
    smtk::operation::AssignColors,
    smtk::operation::SetProperty,
    smtk::attribute::Associate,
    smtk::attribute::Dissociate,
    smtk::attribute::Export,
    smtk::attribute::Import>;
  auto manager = smtk::operation::Manager::create();
  manager->registerOperations<OperationList>();

  // Now "decorate" the operations above to override
  // presentation and filter out some.
  std::string content(R"xml(
  <View Name="Test" Type="Model" Autorun="true">
    <OperationDecorator>
      <Operation TypeRegex="smtk::attribute::.*"/>
      <Operation Type="smtk::operation::AssignColors">
        <Label>choose a color</Label>
        <ButtonLabel>choose color</ButtonLabel>
        <Tooltip>Choose color(s) for all selected components.</Tooltip>
      </Operation>
    </OperationDecorator>
  </View>
  )xml");
  pugi::xml_document doc;
  pugi::xml_parse_result presult = doc.load_buffer(content.c_str(), content.size());
  test(presult.status == pugi::status_ok, "Could not parse XML configuration.");
  std::shared_ptr<smtk::view::Configuration> config;
  from_xml(doc.child("View"), config);
  test(!!config, "Could not process XML.");
  smtk::view::OperationDecorator decorator(manager, config->details().child(0));

  // Test that the decorator was properly constructed.
  std::cout << "Configured " << decorator.size() << " operations.\n";
  decorator.dump();
  test(decorator.size() == 5, "Expected 5 whitelisted operations.");

  // Test that our override took effect.
  {
    const auto& entry = decorator.at<smtk::operation::AssignColors>();
    test(entry.first, "Expected to find assign-colors operation.");
    test(entry.second.get().m_label == "choose a color", "Expected an overridden label.");
    test(entry.second.get().m_buttonLabel == "choose color", "Expected an overridden button.");
  }

  // Test that operations present but undecorated are filtered out:
  {
    const auto& entry = decorator.at<smtk::model::AddImage>();
    test(!entry.first, "Found an operation that should be omitted.");
  }
}

} // anonymous namespace

int unitOperationDecorator(int, char*[])
{
  testInitializerListCtor();
  testConfigurationCtor();
  return 0;
}
