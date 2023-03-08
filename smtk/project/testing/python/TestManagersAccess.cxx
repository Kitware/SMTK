//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/Managers.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/operation/operators/ImportPythonOperation.h"
#include "smtk/plugin/Registry.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Registrar.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <string>

/** \brief Tests that python operations can access smtk managers */
int main(int argc, char* argv[])
{
  smtkTest(argc >= 2, "argument with path to python operation (file) missing");

  // Create smtk managers
  auto managers = smtk::common::Managers::create();
  auto resourceRegistry = smtk::plugin::addToManagers<smtk::resource::Registrar>(managers);
  auto attributeRegistry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(managers);
  auto operationRegistry = smtk::plugin::addToManagers<smtk::operation::Registrar>(managers);
  auto projectRegistry = smtk::plugin::addToManagers<smtk::project::Registrar>(managers);

  // Access smtk managers
  auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  auto projectManager = managers->get<smtk::project::Manager::Ptr>();

  // Initialize operations
  auto attributeOpRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager, operationManager);
  auto operationOpRegistry =
    smtk::plugin::addToManagers<smtk::operation::Registrar>(resourceManager, operationManager);
  auto projectOpRegistry =
    smtk::plugin::addToManagers<smtk::project::Registrar>(resourceManager, projectManager);

  // Register a new project type and create one instance
  projectManager->registerProject("xyzzy");
  smtk::project::Project::Ptr project = projectManager->create("xyzzy");
  smtkTest(project != nullptr, "failed to create project.");
  smtkTest(project->setName("plugh"), "failed to set project name.");

  // Import the Python operation passed in as argv[1]
  smtk::operation::ImportPythonOperation::Ptr importOp =
    operationManager->create<smtk::operation::ImportPythonOperation>();
  importOp->parameters()->findFile("filename")->setValue(std::string(argv[1]));
  auto importResult = importOp->operate();
  int importOutcome = importResult->findInt("outcome")->value();
  smtkTest(
    importOutcome == static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "failed to import " << argv[1]);

  // Create the Python operation and run
  std::string operationName = importResult->findString("unique_name")->value();
  auto testOp = operationManager->create(operationName);
  auto testResult = testOp->operate();
  int testOutcome = testResult->findInt("outcome")->value();
  smtkTest(
    testOutcome == static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "python operation failed ()" << argv[1] << ")");

  return 0;
}
