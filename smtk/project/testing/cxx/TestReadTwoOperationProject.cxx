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
#include "smtk/operation/operators/RemoveResource.h"
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

#include "boost/filesystem.hpp"

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
  // Instantiate smtk plugin clients
  using ResClient = smtk::plugin::Client<smtk::resource::Registrar, smtk::common::Managers>;
  static auto resClient = std::dynamic_pointer_cast<ResClient>(ResClient::create());

  using OpClient = smtk::plugin::Client<
    smtk::operation::Registrar,
    smtk::operation::Manager,
    smtk::common::Managers,
    smtk::resource::Manager>;
  static auto opClient = std::dynamic_pointer_cast<OpClient>(OpClient::create());

  using ProjClient = smtk::plugin::Client<
    smtk::project::Registrar,
    smtk::project::Manager,
    smtk::common::Managers,
    smtk::operation::Manager,
    smtk::resource::Manager>;
  static auto projClient = std::dynamic_pointer_cast<ProjClient>(ProjClient::create());

  using TaskClient = smtk::plugin::Client<smtk::task::Registrar, smtk::task::Manager>;
  static auto taskClient = std::dynamic_pointer_cast<TaskClient>(TaskClient::create());

  // Create smtk managers
  auto managers = smtk::common::Managers::create();
  smtk::plugin::Manager::instance()->registerPluginsTo(managers);

  auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  auto projectManager = managers->get<smtk::project::Manager::Ptr>();

  // Import MathOp operation (used in the SubmitOperation tasks)
  {
    auto importOp = operationManager->create("smtk::operation::ImportPythonOperation");
    std::string importPath = data_root + "/operations/math_op.py";
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

  // Register "basic" project type for our input
  projectManager->registerProject("basic");
  int addedCount = 0;
  int removedCount = 0;
  auto key = projectManager->observers().insert(
    [&](const smtk::project::Project&, smtk::project::EventType event) {
      std::cout << "Observing project event " << static_cast<int>(event) << "\n";
      if (event == smtk::project::EventType::ADDED)
      {
        ++addedCount;
      }
      if (event == smtk::project::EventType::REMOVED)
      {
        ++removedCount;
      }
    },
    0,
    true,
    "TestReadTwoOperationProject observer.");

  std::shared_ptr<smtk::project::Project> project;
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
    project = std::dynamic_pointer_cast<smtk::project::Project>(resource);
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

  // Test closing the project and verify that the project manager's observers are invoked.
  {
    auto removeOp = operationManager->create<smtk::operation::RemoveResource>();
    smtkTest(!!removeOp, "No remove operation");
    removeOp->parameters()->associations()->setValue(project);
    auto removeResult = removeOp->operate();
    int removeOutcome = removeResult->findInt("outcome")->value();
    smtkTest(removeOutcome == OP_SUCCEEDED, "Failed to remove project.");
    smtkTest(addedCount == 1, "Failed to observe project being added.");
    smtkTest(removedCount == 1, "Failed to observe project being removed.");
  }

  return 0;
}
