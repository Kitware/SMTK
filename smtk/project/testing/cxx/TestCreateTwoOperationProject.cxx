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
#include "smtk/attribute/operators/Import.h"
#include "smtk/common/Managers.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/operation/operators/ImportPythonOperation.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/WriteResource.h"
#include "smtk/plugin/Manager.h"
#include "smtk/plugin/Registry.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/project/Registrar.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/json/Helper.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Registrar.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/plugin/Client.txx"

#include "nlohmann/json.hpp"

#include "boost/filesystem.hpp"

#include <exception>

#include <fstream>
#include <iostream>
#include <string>

/** \brief Creates a "TwoOperation" project instance.
 *
 * The project is initialized with one attribute resource, and the task manager
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

void clearDirectory(const std::string& path)
{
  ::boost::filesystem::path boostPath(path);
  if (::boost::filesystem::exists(boostPath))
  {
    std::cout << "Deleting existing directory " << path << std::endl;
    ::boost::filesystem::remove_all(boostPath);
  }
}
} // namespace

/** \brief Reads project from disk and does some minimal checks */
void readProject(smtk::common::ManagersPtr managers)
{
  auto opManager = managers->get<smtk::operation::ManagerPtr>();
  auto readOp = opManager->create<smtk::operation::ReadResource>();
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
    "Input project wrong number of adaptors; should be " << ADAPTOR_COUNT << " not "
                                                         << taskManager.adaptorInstances().size());
}

int TestCreateTwoOperationProject(int /*unused*/, char** const /*unused*/)
{
  clearDirectory(projectDirectory);

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

  // Register "basic" project type, which can be read by default modelbuilder
  projectManager->registerProject("basic");

  // Build the project
  smtk::project::Project::Ptr project = projectManager->create("basic");
  smtkTest(!!project, "Failed to create project");
  projectManager->add(
    project->index(), project); // we didn't run a Create operation so we must do this manually.

  smtk::task::Manager& taskManager = project->taskManager();
  smtk::plugin::Manager::instance()->registerPluginsTo(project->taskManager().shared_from_this());
  {
    // Inject tasks from json file
    std::string jsonPath = data_root + "/projects/src/two-operations.json";
    std::ifstream ifs(jsonPath);
    smtkTest(ifs.good(), "ifstream error with jsonPath " << jsonPath);
    nlohmann::json j = nlohmann::json::parse(ifs);
    ifs.close();
    // std::cout << j.dump(2) << std::endl;

    taskManager.taskInstances().pauseWorkflowNotifications(true);

    auto& resourceHelper = smtk::resource::json::Helper::instance();
    resourceHelper.setManagers(managers);
    try
    {
      auto& taskHelper =
        smtk::task::json::Helper::pushInstance(taskManager, resourceHelper.managers());
      taskHelper.setManagers(resourceHelper.managers());
      from_json(j, taskManager);
      smtk::task::json::Helper::popInstance();
    }
    catch (std::exception& e)
    {
      smtkTest(false, "Exception " << e.what());
    }
    // bool ok = smtk::task::json::jsonManager::deserialize(managers, j);
    taskManager.taskInstances().pauseWorkflowNotifications(false);

    smtkTest(
      taskManager.taskInstances().size() == TASK_COUNT,
      "New project wrong number of tasks; should be " << TASK_COUNT << " not "
                                                      << taskManager.taskInstances().size());
    smtkTest(
      taskManager.adaptorInstances().size() == ADAPTOR_COUNT,
      "New project wrong number of adaptors; should be " << ADAPTOR_COUNT << " not "
                                                         << taskManager.adaptorInstances().size());

    // Load sbt file and add attribute resource to the project.
    auto readOp = operationManager->create<smtk::attribute::Import>();
    std::string sbtPath = data_root + "/projects/src/two-operations.sbt";
    readOp->parameters()->findFile("filename")->setValue(sbtPath);
    auto readResult = readOp->operate();
    int outcome = readResult->findInt("outcome")->value();
    if (outcome != OP_SUCCEEDED)
    {
      std::string log = readOp->log().convertToString();
      std::cerr << log << std::endl;
      smtkTest(false, "failed to read " << sbtPath);
    }

    auto readResource = readResult->findResource("resource")->value();
    auto attResource = std::dynamic_pointer_cast<smtk::attribute::Resource>(readResource);
    project->resources().add(attResource, "attributes");
  }

  // Write project to scratch directory
  {
    std::string projectLocation = projectDirectory + "TwoOperations.project.smtk";

    auto writeOp = operationManager->create<smtk::operation::WriteResource>();
    smtkTest(!!writeOp, "No write operator");
    writeOp->parameters()->associate(project);
    writeOp->parameters()->findFile("filename")->setIsEnabled(true);
    writeOp->parameters()->findFile("filename")->setValue(projectLocation);
    auto writeResult = writeOp->operate();
    int outcome = writeResult->findInt("outcome")->value();
    if (outcome != OP_SUCCEEDED)
    {
      std::string log = writeOp->log().convertToString();
      std::cerr << log << std::endl;
      smtkTest(false, "failed to write " << projectLocation);
    }
    std::cout << "Wrote " << projectLocation << std::endl;
  }

  projectManager->remove(project);
  resourceManager->remove(project);
  project.reset();

  // Sanity check the output
  readProject(managers);

  // Finis
  return 0;
}
