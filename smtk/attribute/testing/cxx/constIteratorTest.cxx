//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include <iterator>
#include <string>

const std::string attTemplate = R"(
<SMTK_AttributeResource Version="8">
  <Definitions>
    <AttDef Type="Test">
      <ItemDefinitions>
        <Component Name="ComponentItem" NumberOfRequiredValues="0" Extensible="true">
          <Accepts>
            <Resource Name="smtk::resource::Resource"/>
          </Accepts>
        </Component>
        <Resource Name="ResourceItem" NumberOfRequiredValues="0" Extensible="true"/>
        <Reference Name="ReferenceItem" NumberOfRequiredValues="0" Extensible="true">
          <Accepts>
            <Resource Name="smtk::resource::Resource" Filter=""/>
            <Resource Name="smtk::resource::Resource" Filter="*"/>
          </Accepts>
        </Reference>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Empty">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Type="Test" Name="Test1" />
  </Attributes>
</SMTK_AttributeResource>
)";

static void testComponentItem()
{
  auto attResource = smtk::attribute::Resource::create();
  smtk::io::AttributeReader reader;
  auto logger = smtk::io::Logger::instance();
  const bool error = reader.readContents(attResource, attTemplate, logger);
  smtkTest(!error, "Error reading template.");

  // Find the attribute resource.
  auto att = attResource->findAttribute("Test1");
  smtkTest(att != nullptr, "Test1 attribute not found");

  // Component item should be empty.
  auto compItem = att->findComponent("ComponentItem");
  smtkTest(compItem != nullptr, "ComponentItem not found");
  smtkTest(std::distance(compItem->begin(), compItem->end()) == 0, "ComponentItem is not empty");

  smtk::common::UUIDArray uuids;
  compItem->setNumberOfValues(4);
  for (int i = 0; i < 4; ++i)
  {
    auto newAtt = attResource->createAttribute("att" + std::to_string(i), "Empty");
    compItem->setValue(i, newAtt);
    uuids.push_back(newAtt->id());
  }

  // Test that the component item returns the components in the same order as insertion when
  // iterated over.
  for (auto it = compItem->begin(); it != compItem->end(); ++it)
  {
    const auto index = std::distance(compItem->begin(), it);
    const auto comp = *it;
    const auto expectedId = uuids[index];
    const auto actualId = comp->id();
    smtkTest(
      actualId == expectedId,
      std::string("component (") + actualId.toString() + ") at " + std::to_string(index) +
        " does not match " + expectedId.toString())
  }
}

static void testResourceItem()
{
  auto attResource = smtk::attribute::Resource::create();
  smtk::io::AttributeReader reader;
  auto logger = smtk::io::Logger::instance();
  const bool error = reader.readContents(attResource, attTemplate, logger);
  smtkTest(!error, "Error reading template.");

  // Find the attribute resource.
  auto att = attResource->findAttribute("Test1");
  smtkTest(att != nullptr, "Test1 attribute not found");

  // Resource item should be empty.
  auto resourceItem = att->findResource("ResourceItem");
  smtkTest(resourceItem != nullptr, "ResourceItem not found");
  smtkTest(
    std::distance(resourceItem->begin(), resourceItem->end()) == 0, "ResourceItem is not empty");

  smtk::resource::ResourceArray rsrcs;
  smtk::common::UUIDArray uuids;
  resourceItem->setNumberOfValues(4);
  for (int i = 0; i < 4; ++i)
  {
    auto newRsrc = smtk::attribute::Resource::create();
    rsrcs.push_back(newRsrc);
    resourceItem->setValue(i, newRsrc);
    uuids.push_back(newRsrc->id());
  }

  // Test that the resource item returns the resources in the same order as insertion when
  // iterated over.
  for (auto it = resourceItem->begin(); it != resourceItem->end(); ++it)
  {
    const auto index = std::distance(resourceItem->begin(), it);
    const auto resource = *it;
    const auto expectedId = uuids[index];
    const auto actualId = resource->id();
    smtkTest(
      actualId == expectedId,
      std::string("resource (") + actualId.toString() + ") at " + std::to_string(index) +
        " does not match " + expectedId.toString())
  }
}

static void testReferenceItem()
{
  auto attResource = smtk::attribute::Resource::create();
  smtk::io::AttributeReader reader;
  auto logger = smtk::io::Logger::instance();
  const bool error = reader.readContents(attResource, attTemplate, logger);
  smtkTest(!error, "Error reading template.");

  // Find the attribute resource.
  auto att = attResource->findAttribute("Test1");
  smtkTest(att != nullptr, "Test1 attribute not found");

  // Resource item should be empty.
  auto referenceItem = att->findReference("ReferenceItem");
  smtkTest(referenceItem != nullptr, "ReferenceItem not found");
  smtkTest(
    std::distance(referenceItem->begin(), referenceItem->end()) == 0, "ResourceItem is not empty");

  smtk::resource::ResourceArray rsrcs;
  smtk::common::UUIDArray uuids;
  referenceItem->setNumberOfValues(8);
  for (int i = 0; i < 4; ++i)
  {
    auto newRsrc = smtk::attribute::Resource::create();
    rsrcs.push_back(newRsrc);
    referenceItem->setValue(i, newRsrc);
    uuids.push_back(newRsrc->id());
  }

  for (int i = 4; i < 8; ++i)
  {
    auto newAtt = attResource->createAttribute("att" + std::to_string(i), "Empty");
    referenceItem->setValue(i, newAtt);
    uuids.push_back(newAtt->id());
  }

  // Test that the resource item returns the resources in the same order as insertion when
  // iterated over.
  for (auto it = referenceItem->begin(); it != referenceItem->end(); ++it)
  {
    const auto index = std::distance(referenceItem->begin(), it);
    const auto obj = *it;
    const auto expectedId = uuids[index];
    const auto actualId = obj->id();
    smtkTest(
      actualId == expectedId,
      std::string("resource (") + actualId.toString() + ") at " + std::to_string(index) +
        " does not match " + expectedId.toString())
  }
}

int main()
{
  testComponentItem();
  testResourceItem();
  testReferenceItem();
  return 0;
}
