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
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/Managers.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/operation/operators/ImportPythonOperation.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/plugin/Manager.h"
#include "smtk/plugin/Registry.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/project/Registrar.h"
#include "smtk/resource/Manager.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Registrar.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/plugin/Client.txx"

// #include "nlohmann/json.hpp"

#include "boost/filesystem.hpp"

#include <exception>

#include <fstream>
#include <iostream>
#include <string>

/** \brief Reads the project created by TestCreateTwoOperationProject.
 *
 * The project includes one attribute resource, and the task manager
 * is loaded with a 5-task workflow containing a GatherResource task, 2 FillOutAttributes
 * tasks, and 2 SubmitOperation tasks.
*/

namespace
{
const std::string data_root = SMTK_DATA_DIR;     // defined in cmake file
const std::string write_root = SMTK_SCRATCH_DIR; // defined in cmake file
const std::string projectDirectory = write_root + "/TwoOperations/";
const std::string projectLocation = projectDirectory + "TwoOperations.project.smtk";

const int OP_SUCCEEDED = static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED);

const int TASK_COUNT = 5;
const int ADAPTOR_COUNT = 4;
} // namespace

int TestReadTwoOperationProject(int /*unused*/, char** const /*unused*/)
{
  // Copy the mystery managers/registry code from TestReadWriteProject.cxx
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();
  auto managers = smtk::common::Managers::create();
  managers->insert_or_assign(resourceManager);
  managers->insert_or_assign(operationManager);

  auto attributeRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager, operationManager);
  auto operationRegistry =
    smtk::plugin::addToManagers<smtk::operation::Registrar>(operationManager);

  operationManager->registerResourceManager(resourceManager);
  operationManager->setManagers(managers);

  // Create smtk::plugin::Client for task manager, so that we can successfully call
  // smtk::plugin::Manager::registerPluginsTo() when project is created.
  using Client = smtk::plugin::Client<smtk::task::Registrar, smtk::task::Manager>;
  static std::shared_ptr<Client> myClient;
  myClient = std::dynamic_pointer_cast<Client>(Client::create());

  // Import MathOp operation (used in the SubmitOperation tasks)
  {
    auto importOp = operationManager->create("smtk::operation::ImportPythonOperation");
    std::string importPath = data_root + "/projects/src/math_op.py";
    importOp->parameters()->findFile("filename")->setValue(importPath);
    auto importResult = importOp->operate();
    int outcome = importResult->findInt("outcome")->value();
    if (outcome != OP_SUCCEEDED)
    {
      smtkInfoMacro(smtk::io::Logger::instance(), importOp->log().convertToString());
      smtkErrorMacro(
        smtk::io::Logger::instance(), "Error importing " << importPath << ": outcome " << outcome);
      smtkTest(false, "Error importing " << importPath);
    }
    std::string opName = importResult->findString("unique_name")->value();
    std::cout << "Imported operation \"" << opName << "\"" << std::endl;
  }

  // Create project manager
  smtk::project::ManagerPtr projectManager =
    smtk::project::Manager::create(resourceManager, operationManager);
  auto projectRegistry =
    smtk::plugin::addToManagers<smtk::project::Registrar>(resourceManager, projectManager);
  auto taskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(managers);

  // Register "basic" project type for our input
  projectManager->registerProject("basic");

  // Read the project and sanity check the task manager
  {
    auto readOp = operationManager->create<smtk::operation::ReadResource>();
    smtkTest(!!readOp, "No read operation");
    readOp->parameters()->findFile("filename")->setValue(projectLocation);
    auto readResult = readOp->operate();
    int readOutcome = readResult->findInt("outcome")->value();
    if (readOutcome != OP_SUCCEEDED)
    {
      std::string log = readOp->log().convertToString();
      std::cerr << log << std::endl;
      smtkTest(false, "failed to read " << projectLocation);
    }

    auto resource = readResult->findResource("resource")->value();
    auto project = std::dynamic_pointer_cast<smtk::project::Project>(resource);
    smtkTest(!!project, "failed to read  project from location " << projectLocation);

    smtk::task::Manager& taskManager = project->taskManager();
    smtkTest(
      taskManager.taskInstances().size() == TASK_COUNT,
      "Input project wrong number of tasks; should be " << TASK_COUNT << " not "
                                                        << taskManager.taskInstances().size());
    smtkTest(
      taskManager.adaptorInstances().size() == ADAPTOR_COUNT,
      "Input project wrong number of adaptors; should be "
        << ADAPTOR_COUNT << " not " << taskManager.adaptorInstances().size());
  }

  return 0;
}
