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
#include "smtk/markup/ontology/OwlRdfSource.h"
#include "smtk/markup/operators/TagIndividual.h"
#include "smtk/markup/testing/cxx/helpers.h"

#include "smtk/plugin/Registry.h"

#include "smtk/operation/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
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
#include <fstream>
#include <iostream>
#include <set>

using namespace smtk::markup;
// using namespace json;

namespace
{

std::string dataRoot = SMTK_DATA_DIR;
std::string writeRoot = SMTK_SCRATCH_DIR;

std::string exampleXml()
{
  std::string xml;
  std::ifstream file((dataRoot + "/model/conceptual/example.owl").c_str());
  if (file.good())
  {
    xml = std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
  }
  return xml;
}

bool testTagIndividual(const smtk::common::Managers::Ptr& managers)
{
  // Register a test ontology
  std::string ontologyName = "engineered";
  smtk::markup::ontology::Source::registerSource(smtk::markup::ontology::OwlRdfSource(
    exampleXml(), "http://dummy.com/engineered.owl", ontologyName));

  // Create a resource
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

  // Add a tag
  auto tagger = operationManager->create<smtk::markup::TagIndividual>();
  test(
    tagger->parameters()->associations()->setValue(group1),
    "Unable to associate group1 to tagger.");
  const auto& source = smtk::markup::ontology::Source::findByName(ontologyName);
  test(!source.classes().empty(), "Could not find source.");

  auto tagsToAdd = tagger->parameters()->findGroup("tagsToAdd");
  std::cout << "Marking group1 with \"" << source.classes()[0].name << "\"\n"
            << source.classes()[0].description << "\n";
  tagsToAdd->setNumberOfGroups(1);
  tagsToAdd->findAs<smtk::attribute::StringItem>(0, "name")->setValue(source.classes()[0].name);
  tagsToAdd->findAs<smtk::attribute::StringItem>(0, "url")->setValue(source.classes()[0].url);
  tagsToAdd->findAs<smtk::attribute::StringItem>(0, "ontology")->setValue(ontologyName);

  auto result = tagger->operate();
  auto outcome =
    static_cast<smtk::operation::Operation::Outcome>(result->findInt("outcome")->value());
  test(outcome == smtk::operation::Operation::Outcome::SUCCEEDED, "Expected to tag group1.");
  std::cout << "group1 tags:\n";
  smtk::markup::OntologyIdentifier* tag = nullptr;
  group1->ontologyClasses().visit([&](const smtk::markup::OntologyIdentifier* oid) {
    tag = const_cast<smtk::markup::OntologyIdentifier*>(oid);
    const auto* ont = oid->parent().node();
    std::cout << "  " << oid->name() << " (id " << oid->ontologyId().data() << ", ontology "
              << ont->name() << ", " << ont->url() << ")\n";
  });
  test(group1->ontologyClasses().size() == 1, "Expected 1 tag.");
  test(tag && tag->name() == "accept hand", "Expected \"accept hand\" tag.");

  // Remove the tag we added.
  tagsToAdd->setNumberOfGroups(0);
  auto tagsToRemove = tagger->parameters()->findComponent("tagsToRemove");
  tagsToRemove->appendValue(tag->shared_from_this());
  result = tagger->operate();
  outcome = static_cast<smtk::operation::Operation::Outcome>(result->findInt("outcome")->value());
  test(outcome == smtk::operation::Operation::Outcome::SUCCEEDED, "Expected to untag group1.");
  test(group1->ontologyClasses().empty(), "Expected to remove tag, but it's still there.");

  return true;
}

} // anonymous namespace

int TestTag(int, char** const)
{
  smtk::io::Logger::instance().setFlushToStdout(true);
  auto managers = smtk::markup::createTestManagers();
  auto markupRegistry = smtk::plugin::addToManagers<smtk::markup::Registrar>(
    managers->get<smtk::resource::Manager::Ptr>(), managers->get<smtk::operation::Manager::Ptr>());
  bool ok = testTagIndividual(managers);
  return ok ? 0 : 1;
}
