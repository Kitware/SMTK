//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/AssignedIds.h"
#include "smtk/markup/Component.h"
#include "smtk/markup/Group.h"
#include "smtk/markup/Label.h"
#include "smtk/markup/Registrar.h"
#include "smtk/markup/Resource.h"
#include "smtk/markup/operators/Read.h"
#include "smtk/markup/operators/Write.h"

#include "smtk/plugin/Registry.h"

#include "smtk/operation/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/Managers.h"
#include "smtk/common/UUID.h"
#include "smtk/common/testing/cxx/helpers.h"

#include "nlohmann/json.hpp"

#include <cstdio>
#include <iostream>
#include <sstream>

using namespace smtk::markup;
// using namespace json;

namespace
{

void dumpComponent(smtk::markup::Component::Ptr component)
{
  std::cout << "  " << component->name() << " " << component->id() << " " << component.get()
            << "\n";

  std::cout << "    Labels:\n";
  try
  {
    component->incoming<arcs::LabelsToSubjects>().visit([](const Label* label) {
      std::cout << "      " << label->name() << " " << label->id() << "\n";
    });
  }
  catch (std::domain_error&)
  {
  }
}

smtk::common::Managers::Ptr testRegistrar()
{
  auto managers = smtk::common::Managers::create();
  auto resourceManager = smtk::resource::Manager::create();
  auto operationManager = smtk::operation::Manager::create();
  managers->insert_or_assign(resourceManager);
  managers->insert_or_assign(operationManager);

  auto markupRegistry =
    smtk::plugin::addToManagers<smtk::markup::Registrar>(resourceManager, operationManager);

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  return managers;
}

std::string testCreateAndWrite()
{
  auto resource = smtk::markup::Resource::create();
  std::string filename = generateFilename("markup.", ".smtk");
  resource->setLocation(filename);

  auto label = resource->createNode<Label>();
  label->setName("foo");
  auto group = std::make_shared<Group>(resource, smtk::common::UUID::random());
  resource->add(group);
  group->setName("barf");
  auto components = resource->filter("*");
  std::size_t nn = components.size();
  std::cout << "Created " << resource << " with " << nn << " components.\n";
  for (const auto& component : components)
  {
    std::cout << "  " << component->typeName() << ": " << component->name() << "\n";
  }
  test(nn == 2, "Expected to create 2 components.");

  components = resource->filter("'smtk::markup::Label'");
  nn = components.size();
  std::cout << "Found " << nn << " labels\n";

  auto anotherLabel = resource->createNode<Label>(
    R"({"name": "This is a really long descriptive label with no purpose."})"_json);
  auto anotherGroup = resource->createNode<Group>(R"({"name": "baz"})"_json);

  for (const auto& arcType : resource->arcs().types())
  {
    std::cout << "Arc type \"" << arcType.data() << "\" (" << arcType.id() << ")\n";
  }
  // Test that we can create both outgoing and incoming arcs:
  label->outgoing<smtk::markup::arcs::LabelsToSubjects>().connect(group);
  anotherGroup->incoming<smtk::markup::arcs::LabelsToSubjects>().connect(anotherLabel);

  // Write our resource out so we can test the read and write operations:
  std::cout << "Writing " << filename << "\n";
  auto write = smtk::markup::Write::create();
  write->parameters()->associations()->appendValue(resource);
  write->operate();
  return filename; // or resource->location();
}

std::string testReadAndWrite(const std::string& filename1)
{
  bool ok = true;
  auto read = smtk::markup::Read::create();
  ok = read->parameters()->findFile("filename")->setValue(filename1);
  test(ok, "Could not set filename.");
  std::cout << "Reading " << read->parameters()->findFile("filename")->value() << "\n";
  auto result = read->operate();
  auto resource = result->findResource("resource")->valueAs<smtk::markup::Resource>();
  std::cout << "Read    " << result->findInt("outcome")->value() << " " << resource << "\n";

  // Rename the file we just read to make room for the second generation copy.
  std::string filename2 = ::generateFilename("markup.", ".smtk");
  std::cout << "Rename  " << filename2 << "\n";
  std::rename(filename1.c_str(), filename2.c_str());

  // See what we got.
  auto components = resource->filter("*");
  auto nn = components.size();
  std::cout << "Found " << nn << " components\n";
  for (const auto& component : components)
  {
    dumpComponent(std::dynamic_pointer_cast<smtk::markup::Component>(component));
  }

  // Write our resource back to filename1.
  auto write = smtk::markup::Write::create();
  write->parameters()->associations()->appendValue(resource);
  result = write->operate();
  std::cout << "Wrote   " << result->findInt("outcome")->value() << " " << resource << "\n";

  return filename2;
}

bool testFileContentsMatch(const std::string& filename1, const std::string& filename2)
{
  (void)filename1;
  (void)filename2;
  // TODO: Implement me.
  return true;
}

bool testQueryFilter()
{
  std::cout << "Test query filters:\n";
  auto resource = smtk::markup::Resource::create();
  std::string filename = generateFilename("markup.", ".smtk");
  resource->setLocation(filename);

  auto label = resource->createNode<Label>();
  label->setName("foo");
  auto group = std::make_shared<Group>(resource, smtk::common::UUID::random());
  resource->add(group);
  group->setName("barf");
  auto components = resource->filter("*");
  std::size_t nn = components.size();
  std::cout << "  Created " << resource << " with " << nn << " components.\n";
  for (const auto& component : components)
  {
    std::cout << "    " << component->typeName() << ": " << component->name() << "\n";
  }
  test(nn == 2, "Expected to create 2 components.");

  components = resource->filter("'smtk::markup::Label'");
  nn = components.size();
  std::cout << "  Found " << nn << " labels.\n";
  test(nn == 1, "Expected a single label.");

  components = resource->filter("/.*Group/");
  nn = components.size();
  std::cout << "  Found " << nn << " groups.\n";
  test(nn == 1, "Expected a single group.");

  components = resource->filter("");
  nn = components.size();
  std::cout << "  Found " << nn << " components with empty string.\n";
  test(nn == 0, "Expected a single group.");

  components = resource->filter("*");
  nn = components.size();
  std::cout << "  Found " << nn << " components with '*'.\n";
  test(nn == 2, "Expected a single group.");

  components = resource->filter("any");
  nn = components.size();
  std::cout << "  Found " << nn << " components with 'any'.\n";
  test(nn == 2, "Expected a single group.");
  return true;
}

} // anonymous namespace

int TestResource(int, char** const)
{
  auto managers = testRegistrar();
  std::string filename1 = testCreateAndWrite();
  std::string filename2 = testReadAndWrite(filename1);
  bool ok = testFileContentsMatch(filename1, filename2);
  ok &= testQueryFilter();
  return ok ? 0 : 1;
}
