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
#include "smtk/markup/operators/Delete.h"
#include "smtk/markup/testing/cxx/helpers.h"

#include "smtk/plugin/Registry.h"

#include "smtk/operation/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/Managers.h"
#include "smtk/common/UUID.h"
#include "smtk/common/testing/cxx/helpers.h"

#include "nlohmann/json.hpp"

#include <cstdio>
#include <iostream>
#include <set>

using namespace smtk::markup;
// using namespace json;

namespace
{

bool testDelete(const smtk::common::Managers::Ptr& managers)
{
  auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  auto resource = resourceManager->create<smtk::markup::Resource>();
  test(!!resource, "Expected to create a resource.");

  auto group1 = resource->createNode<Group>();
  auto group2 = resource->createNode<Group>();
  auto label1 = resource->createNode<Label>();
  auto label2 = resource->createNode<Label>();
  auto label3 = resource->createNode<Label>();
  label1->setName("foo");
  label2->setName("bar");
  label3->setName("baz");
  group1->setName("group1");
  group2->setName("group2");
  group1->members().connect(group2);
  group1->members().connect(label1);
  group2->members().connect(label3);
  label2->subjects().connect(label1);

  auto deleter = operationManager->create<smtk::markup::Delete>();
  test(
    deleter->parameters()->associations()->setValue(group1),
    "Unable to associate group1 to deleter.");
  auto result = deleter->operate();
  auto outcome =
    static_cast<smtk::operation::Operation::Outcome>(result->findInt("outcome")->value());
  test(
    outcome == smtk::operation::Operation::Outcome::UNABLE_TO_OPERATE,
    "Expected ownership semantics to prevent deletion.");
  std::cout << "testDelete: Prevented deletion without dependents.\n";

  deleter->parameters()->findVoid("delete dependents")->setIsEnabled(true);
  result = deleter->operate();
  outcome = static_cast<smtk::operation::Operation::Outcome>(result->findInt("outcome")->value());
  test(outcome == smtk::operation::Operation::Outcome::SUCCEEDED, "Expected deletion to occur.");
  std::cout << "testDelete: Deletion with dependents succeeded.\n";

  auto expungedItem = result->findComponent("expunged");
  auto expunged = expungedItem->as<std::set<smtk::resource::Component::Ptr>>(
    [](const smtk::resource::PersistentObject::Ptr& obj) {
      return std::dynamic_pointer_cast<smtk::resource::Component>(obj);
    });
  test(expunged.find(group1) != expunged.end(), "Expected to remove group1.");
  test(expunged.find(group2) != expunged.end(), "Expected to remove group2.");
  test(expunged.find(label1) != expunged.end(), "Expected to remove label1.");
  test(
    expunged.find(label3) != expunged.end(),
    "Expected to remove label3 (dependent of a dependent).");
  test(label2->subjects().empty(), "Expected label2's arc to removed label1 to die.");
  std::cout << "testDelete: Deletion with dependents matched expected results.\n";
  return true;
}

} // anonymous namespace

int TestDelete(int, char** const)
{
  auto managers = smtk::markup::createTestManagers();
  auto markupRegistry = smtk::plugin::addToManagers<smtk::markup::Registrar>(
    managers->get<smtk::resource::Manager::Ptr>(), managers->get<smtk::operation::Manager::Ptr>());
  bool ok = testDelete(managers);
  return ok ? 0 : 1;
}
