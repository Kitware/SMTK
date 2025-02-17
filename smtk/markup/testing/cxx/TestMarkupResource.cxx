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
#include "smtk/markup/operators/CreateArc.h"
#include "smtk/markup/operators/Read.h"
#include "smtk/markup/operators/Write.h"

#include "smtk/graph/Registrar.h"
#include "smtk/graph/operators/CreateArc.h"
#include "smtk/graph/operators/CreateArcType.h"
#include "smtk/graph/operators/DeleteArc.h"

#include "smtk/plugin/Registry.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/groups/ArcDeleter.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

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
  managers->insertOrAssign(resourceManager);
  managers->insertOrAssign(operationManager);

  // Make these static so the registrars' unregister methods are not
  // called until the program exits (as opposed to when this function exits).
  static auto graphRegistry =
    smtk::plugin::addToManagers<smtk::graph::Registrar>(resourceManager, operationManager);
  static auto markupRegistry =
    smtk::plugin::addToManagers<smtk::markup::Registrar>(resourceManager, operationManager);

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  return managers;
}

void testCreateRuntimeArcs(
  const std::shared_ptr<smtk::common::Managers>& managers,
  const std::string& arcTypeName,
  smtk::graph::Directionality dir,
  const std::string& fromNodeType,
  const std::string& toNodeType,
  const std::shared_ptr<smtk::markup::Component>& c1,
  const std::shared_ptr<smtk::markup::Component>& c2)
{
  using smtk::operation::outcome;

  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  auto cat = operationManager->create<smtk::graph::CreateArcType>();
  auto params = cat->parameters();
  params->associations()->setValue(c1->resource());
  params->findString("type name")->setValue(arcTypeName);
  params->findString("directionality")->setValue(dir ? "directed" : "undirected");
  if (dir)
  {
    params->findString("from node types")->appendValue(fromNodeType);
    params->findString("to node types")->appendValue(toNodeType);
  }
  else
  {
    params->findString("end node types")->appendValue(fromNodeType);
  }
  auto result = cat->operate();
  ::test(
    outcome(result) == smtk::operation::Operation::Outcome::SUCCEEDED,
    "Expected operation to succeed.");

  auto car = operationManager->create<smtk::markup::CreateArc>();
  car->parameters()->associate(c1);
  car->parameters()->findReference("to node")->appendValue(c2);
  car->parameters()->findString("arc type")->setValue(arcTypeName);
  result = car->operate();
  ::test(
    outcome(result) == smtk::operation::Operation::Outcome::SUCCEEDED,
    "Expected operation to succeed.");
  ::test(c1->outgoing(arcTypeName).contains(c2), "Expected to connect two components.");
}

void testDeleteRuntimeArcs(
  const std::shared_ptr<smtk::common::Managers>& managers,
  const std::string& arcTypeName,
  const std::shared_ptr<smtk::markup::Component>& c1,
  const std::shared_ptr<smtk::markup::Component>& c2,
  bool expectSuccess)
{
  std::cout << "Deleting \"" << arcTypeName << "\" arc between "
            << "\"" << c1->name() << "\" and \"" << c2->name() << "\""
            << ", expecting " << (expectSuccess ? "success" : "failure") << "\n";
  using smtk::operation::outcome;
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();

  auto deleter = operationManager->create<smtk::graph::DeleteArc>();
  ::test(!!deleter, "Could not create deletion operation");

  auto arcTypeItem = deleter->parameters()->findString("arc type");
  ::test(arcTypeItem->setValue(arcTypeName), "Could not set arc type.");
  ::test(
    smtk::operation::ArcDeleter::appendArc(*deleter, arcTypeName, c1, c2),
    "Could not append arc to deletion operation.");

  if (expectSuccess)
  {
    ::test(c1->outgoing(arcTypeName).contains(c2), "Expected arc to exist before deletion.");
  }
  auto result = deleter->operate();
  ::test(
    outcome(result) ==
      (expectSuccess ? smtk::operation::Operation::Outcome::SUCCEEDED
                     : smtk::operation::Operation::Outcome::FAILED),
    "Expected arc deletion to succeed.");

  ::test(!c1->outgoing(arcTypeName).contains(c2), "Expected arc to be non-existent.");
  std::cout << "  Got expected result!\n";
}

std::string testCreateAndWrite(const std::shared_ptr<smtk::common::Managers>& managers)
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

  // Test filtering by node type.
  components = resource->filter("'smtk::markup::Label'");
  nn = components.size();
  std::cout << "Found " << nn << " labels\n";

  // Test finding by node name (and type).
  auto labelsNamedFoo = resource->findByName<std::set<Label*>>("foo");
  std::cout << "Found " << labelsNamedFoo.size() << " labels named \"foo\".\n";
  auto groupsNamedFoo = resource->findByName<std::set<Group*>>("foo");
  std::cout << "Found " << groupsNamedFoo.size() << " groups named \"foo\".\n";
  auto labelsNamedBarf = resource->findByName<std::vector<Label*>>("barf");
  std::cout << "Found " << labelsNamedBarf.size() << " labels named \"barf\".\n";
  auto groupsNamedBarf = resource->findByName<std::vector<Group*>>("barf");
  std::cout << "Found " << groupsNamedBarf.size() << " groups named \"barf\".\n";
  test(labelsNamedFoo.size() == 1, "Expected 1 label named \"foo\".");
  test(groupsNamedFoo.empty(), "Expected no group named \"foo\".");
  test(labelsNamedBarf.empty(), "Expected no label named \"barf\".");
  test(groupsNamedBarf.size() == 1, "Expected 1 group named \"barf\".");

  auto anotherLabel = resource->createNode<Label>();
  anotherLabel->setName("useless");
  auto anotherGroup = resource->createNode<Group>();
  anotherGroup->setName("baz");

  for (const auto& arcType : resource->arcs().types())
  {
    std::cout << "Arc type \"" << arcType.data() << "\" (" << arcType.id() << ")\n";
  }
  // Test that we can create both outgoing and incoming arcs:
  label->outgoing<smtk::markup::arcs::LabelsToSubjects>().connect(group);
  anotherGroup->incoming<smtk::markup::arcs::LabelsToSubjects>().connect(anotherLabel);

  // Test creation and use of run-time arc types.
  testCreateRuntimeArcs(
    managers,
    "label to label",
    smtk::graph::IsUndirected,
    "smtk::markup::Label",
    "smtk::markup::Label",
    label,
    anotherLabel);
  testCreateRuntimeArcs(
    managers,
    "group to anything",
    smtk::graph::IsDirected,
    "smtk::markup::Group",
    "smtk::markup::Component",
    group,
    anotherLabel);

  // Write our resource out so we can test the read and write operations:
  std::cout << "Writing " << filename << "\n";
  auto write = smtk::markup::Write::create();
  write->parameters()->associations()->appendValue(resource);
  write->operate();
  return filename; // or resource->location();
}

std::string testReadAndWrite(
  const std::shared_ptr<smtk::common::Managers>& managers,
  const std::string& filename1)
{
  using namespace smtk::string::literals;
  bool ok = true;
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  auto read = operationManager->create<smtk::markup::Read>();
  ok = read->parameters()->findFile("filename")->setValue(filename1);
  test(ok, "Could not set filename.");
  std::cout << "Reading " << read->parameters()->findFile("filename")->value() << "\n";
  auto result = read->operate();
  auto resource = result->findResource("resourcesCreated")->valueAs<smtk::markup::Resource>();
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
  auto write = operationManager->create<smtk::markup::Write>();
  write->parameters()->associations()->appendValue(resource);
  result = write->operate();
  std::cout << "Wrote   " << result->findInt("outcome")->value() << " " << resource << "\n";

  // Now that we have written what we read in,
  // test deletion of runtime arcs.
  // Do not move this above the write operation
  // or files will not match.
  // xxx
  auto labelsNamedFoo = resource->findByName<std::set<Label*>>("foo");
  std::cout << "Found " << labelsNamedFoo.size() << " labels named \"foo\".\n";

  smtk::markup::Label::Ptr label;
  smtk::markup::Label::Ptr anotherLabel;
  smtk::markup::Group::Ptr group;
  smtk::markup::Group::Ptr anotherGroup;
  std::function<void(const smtk::resource::Component::Ptr&)> visitor =
    [&](const smtk::resource::Component::Ptr& component) {
      switch (component->typeToken().id())
      {
        case "smtk::markup::Label"_hash:
          if (component->name() == "foo")
          {
            label = std::dynamic_pointer_cast<smtk::markup::Label>(component);
          }
          else
          {
            anotherLabel = std::dynamic_pointer_cast<smtk::markup::Label>(component);
          }
          break;
        case "smtk::markup::Group"_hash:
          if (component->name() == "barf")
          {
            group = std::dynamic_pointer_cast<smtk::markup::Group>(component);
          }
          else
          {
            anotherGroup = std::dynamic_pointer_cast<smtk::markup::Group>(component);
          }
          break;
        default:
          break;
      }
    };

  resource->visit(visitor);
  testDeleteRuntimeArcs(managers, "label to label", label, anotherLabel, true);
  testDeleteRuntimeArcs(managers, "group to anything", group, anotherLabel, true);
  testDeleteRuntimeArcs(managers, "group to anything", group, label, false);

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

int TestMarkupResource(int, char** const)
{
  auto managers = testRegistrar();
  std::string filename1 = testCreateAndWrite(managers);
  std::string filename2 = testReadAndWrite(managers, filename1);
  bool ok = testFileContentsMatch(filename1, filename2);
  ok &= testQueryFilter();
  return ok ? 0 : 1;
}
