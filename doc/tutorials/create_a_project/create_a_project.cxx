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
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/WriteResource.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/project/Registrar.h"
#include "smtk/resource/Manager.h"

#include <iostream>
#include <string>

int main(int /*argc*/, char* /*argv*/[])
{
  // ++ 1 ++
  // Initialize SMTK managers
  smtk::resource::ManagerPtr resManager = smtk::resource::Manager::create();
  smtk::project::Registrar::registerTo(resManager);

  smtk::operation::ManagerPtr opManager = smtk::operation::Manager::create();
  smtk::operation::Registrar::registerTo(opManager);
  opManager->registerResourceManager(resManager);

  smtk::project::ManagerPtr projManager = smtk::project::Manager::create(resManager, opManager);
  smtk::project::Registrar::registerTo(projManager);

  // Register SMTK attribute feature
  smtk::attribute::Registrar::registerTo(resManager);
  smtk::attribute::Registrar::registerTo(opManager);
  // -- 1 --

  // ++ 2 ++
  // Create project with type "basic", a generic type that can be loaded into modelbuilder by default.
  projManager->registerProject("basic");
  smtk::project::ProjectPtr project = projManager->create("basic");
  // If you create the project directly (rather than through
  // an operation), you must add it to the manager explicitly:
  projManager->add(project);
  // -- 2 --

  // ++ 3 ++
  // Create a small attribute resource
  smtk::attribute::ResourcePtr attResource = resManager->create<smtk::attribute::Resource>();
  const std::string attTemplate =
    "<SMTK_AttributeResource Version=\"4\">"
    "  <Definitions>"
    "    <AttDef Type=\"Example\">"
    "      <ItemDefinitions>"
    "        <String Name=\"String Item\">"
    "          <DefaultValue>Yellow denotes default value</DefaultValue>"
    "        </String>"
    "        <Int Name=\"Integer Item\">"
    "          <DefaultValue>42</DefaultValue>"
    "        </Int>"
    "        <Double Name=\"Double Item\">"
    "          <DefaultValue>3.14159</DefaultValue>"
    "        </Double>"
    "      </ItemDefinitions>"
    "    </AttDef>"
    "  </Definitions>"
    "  <Views>"
    "    <View Type=\"Instanced\" Title=\"Example\" TopLevel=\"true\""
    "          FilterByAdvanceLevel=\"false\" FilterByCategory=\"false\">"
    "      <InstancedAttributes>"
    "        <Att Type=\"Example\" Name=\"example1\" />"
    "      </InstancedAttributes>"
    "    </View>"
    "  </Views>"
    "</SMTK_AttributeResource>";

  smtk::io::AttributeReader attReader;
  smtk::io::Logger logger;
  bool err = attReader.readContents(attResource, attTemplate, logger);
  if (err)
  {
    std::cerr << "ERROR: " << logger.convertToString() << std::endl;
    return -1;
  }

  // Create the example attribute instance
  attResource->createAttribute("example1", "Example");
  // -- 3 --

  // ++ 4 ++
  // Add the attribute resource to the project, stting its project role to "attributes".
  const std::string role = "attributes";
  project->resources().add(attResource, role);
  // -- 4 --

  // ++ 5 ++
  // Write the project to the filesystem using the default WriteResource operation.
  smtk::operation::WriteResource::Ptr writeOp = opManager->create<smtk::operation::WriteResource>();
  writeOp->parameters()->associate(project);
  writeOp->parameters()->findFile("filename")->setIsEnabled(true);
  writeOp->parameters()->findFile("filename")->setValue("basic-project.smtk");
  smtk::operation::Operation::Result writeResult = writeOp->operate();
  int writeOutcome = writeResult->findInt("outcome")->value();

  if (writeOutcome == static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cout << "Wrote project to the current directory.\n"
              << " This consists of file \"basic-example.smtk\" in the current directory\n"
              << " which is the project's resource file, and file \"resources/attributes.smtk\"\n"
              << " which is the attribute resource." << std::endl;
  }
  else
  {
    std::cout << "Failed to write project file, with outcome " << writeOutcome << ".\n";
    std::cout << logger.convertToString() << std::endl;
    return -1;
  }

  return 0;
  // -- 5 --
}
