//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/plugin/Registry.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/project/Registrar.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>

// This test verifies that project instances can be associated to
// SMTK attributes.

const char* templateString =
  "<SMTK_AttributeResource Version=\"4\">\n"
  "  <Definitions>\n"
  "    <AttDef Type=\"test-assoc\">\n"
  "      <AssociationsDef Name=\"project\" NumberOfRequiredValues=\"1\"\n"
  "       Extensible=\"false\" OnlyResources=\"true\">\n"
  "        <Accepts>\n"
  "          <Resource Name=\"smtk::project::Project\" />\n"
  "        </Accepts>\n"
  "      </AssociationsDef>\n"
  "    </AttDef>\n"
  "  </Definitions>\n"
  "</SMTK_AttributeResource>\n";

int TestProjectAssociation(int /*unused*/, char** const /*unused*/)
{
  std::cout << templateString << std::endl;
  // Create managers
  smtk::resource::ManagerPtr resManager = smtk::resource::Manager::create();
  auto attributeRegistry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(resManager);
  smtk::operation::ManagerPtr opManager = smtk::operation::Manager::create();
  smtk::project::ManagerPtr prjManager = smtk::project::Manager::create(resManager, opManager);
  auto projectRegistry = smtk::plugin::addToManagers<smtk::project::Registrar>(prjManager);

  // Register and create test project
  // The process of registering our foo project also registers a resource with the same name.
  prjManager->registerProject("foo_project");
  test(resManager->registered("foo_project"));
  auto project = prjManager->create("foo_project");
  test(project != nullptr);

  // Create attribute resource
  auto attResource = resManager->create<smtk::attribute::Resource>();
  test(attResource != nullptr);
  smtk::io::AttributeReader reader;
  smtk::io::Logger logger;
  bool err = reader.readContents(attResource, templateString, logger);
  test(!err);

  // Create attribute
  auto att = attResource->createAttribute("test-assoc");
  test(att != nullptr);

  test(att->associate(project));
  return 0;
}
