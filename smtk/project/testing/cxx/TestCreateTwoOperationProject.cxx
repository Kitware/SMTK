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

#include <fstream>
#include <iostream>
#include <string>

/** \brief Creates and Reads back a "TwoOperation" project instance.
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
    std::cerr << "Deleting existing directory " << path << std::endl;
    ::boost::filesystem::remove_all(boostPath);
  }
}

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

  auto resource = readResult->findResource("resourcesCreated")->value();
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

bool testInheritedAPI(const smtk::project::Project::Ptr& project)
{
  // Test the find(), visit(), and queryOperation() methods
  // inherited from smtk::resource::Resource.
  bool ok = true;
  std::array<std::size_t, 4> numComp{ { 0, 0, 0, 0 } };
  auto qop1 = project->queryOperation("*");
  auto qop2 = project->queryOperation("");
  auto qop3 = project->queryOperation("smtk::task::Task");
  smtk::resource::Component::Visitor visitor =
    [&project, &qop1, &qop2, &qop3, &numComp](const smtk::resource::Component::Ptr& comp) {
      if (comp)
      {
        ++numComp[0];
        if (qop1(*comp))
        {
          ++numComp[1];
        }
        if (qop2(*comp))
        {
          ++numComp[2];
        }
        if (qop3(*comp))
        {
          ++numComp[3];
        }

        ::test(
          comp == project->find(comp->id()), "Could not retrieve visited component with find().");
      }
    };
  project->visit(visitor);
  std::cerr << "Visited " << numComp[0] << " project components, including " << numComp[3]
            << " tasks.\n";
  ::test(
    numComp[0] ==
      project->taskManager().taskInstances().size() +
        project->taskManager().adaptorInstances().size(),
    "Expected to visit 9 tasks+adaptors as project components.");
  ::test(
    numComp[1] ==
      project->taskManager().taskInstances().size() +
        project->taskManager().adaptorInstances().size(),
    "Expected to visit 9 tasks of type '*'.");
  ::test(numComp[2] == 0, "Expected to visit 0 tasks with empty query.");
  return ok;
}

} // namespace

int TestCreateTwoOperationProject(int /*unused*/, char** const /*unused*/)
{
  clearDirectory(projectDirectory);

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
    std::cerr << "Imported operation \"" << opName << "\"" << std::endl;
  }
  auto taskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(managers);

  // Register "basic" project type, which can be read by default modelbuilder
  projectManager->registerProject("basic");

  // Build the project
  smtk::project::Project::Ptr project = projectManager->create("basic");
  smtkTest(!!project, "Failed to create project");
  projectManager->add(
    project->index(), project); // we didn't run a Create operation so we must do this manually.

  smtk::task::Manager& taskManager = project->taskManager();
  {
    ::test(
      taskManager.resource() == project.get(),
      "Expected task manager to refer to its parent project.");
    // Inject tasks from json file
    std::string jsonPath = data_root + "/projects/src/two-operations.json";
    std::ifstream ifs(jsonPath);
    smtkTest(ifs.good(), "ifstream error with jsonPath " << jsonPath);
    nlohmann::json j = nlohmann::json::parse(ifs);
    ifs.close();
    // std::cerr << j.dump(2) << std::endl;

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

    auto readResource = readResult->findResource("resourcesCreated")->value();
    auto attResource = std::dynamic_pointer_cast<smtk::attribute::Resource>(readResource);
    project->resources().add(attResource, "attributes");
  }

  // Test inherited resource API.
  testInheritedAPI(project);

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
    std::cerr << "Wrote " << projectLocation << std::endl;
  }

  std::cerr << "Testing Project Contents\n";
  // Test that filtering for tasks and worklets by type works.
  auto tasks = project->filter("smtk::task::GatherResources");
  smtkTest(tasks.size() == 1, "Expected to filter 1 GatherResources task from the project.");

  tasks = project->filter("smtk::task::FillOutAttributes");
  smtkTest(tasks.size() == 2, "Expected to filter 2 FillOutAttributes tasks from the project.");

  tasks = project->filter("smtk::task::SubmitOperation");
  smtkTest(tasks.size() == 2, "Expected to filter 2 SubmitOperation tasks from the project.");

  auto worklets = project->filter("'smtk::task::Worklet'");
  smtkTest(worklets.empty(), "Expected to filter 0 worklets from project.");
  std::cerr << "Project Contents Passed!\n";

  projectManager->remove(project);
  std::cerr << "Removed project from ProjectManager!";
  resourceManager->remove(project);
  std::cerr << "Removed project from ResourceManager!";
  project.reset();
  std::cerr << "Project reset!";

  readProject(managers);

  // Finis
  return 0;
}
