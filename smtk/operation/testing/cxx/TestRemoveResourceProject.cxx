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
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/operators/Import.h"

#include "smtk/common/UUIDGenerator.h"
#include "smtk/io/Logger.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/operation/operators/ImportResource.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/RemoveResource.h"
#include "smtk/operation/operators/WriteResource.h"

#include "smtk/plugin/Registry.h"

#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/project/Registrar.h"
#include "smtk/project/operators/Define.h"

#ifdef VTK_SUPPORT
#include "smtk/session/vtk/Registrar.h"
#endif

#include "smtk/common/testing/cxx/helpers.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

// Test if removing a project removes its resources.
// The test project contains an SMTK attribute
// resource plus (if vtk support is built) an SMTK vtk model resource.

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

void cleanup(const std::string& location)
{
  //first verify the file exists
  ::boost::filesystem::path path(location);
  printf("Removing: %s...\n", location.c_str());
  if (::boost::filesystem::exists(path))
  {
    printf("Removing: %s...Success\n", location.c_str());
    //remove the file_path if it exists.
    ::boost::filesystem::remove_all(path);
  }
}

} // namespace

int TestRemoveResourceProject(int /*unused*/, char** const /*unused*/)
{
  // Set the file path
  std::string projectDirectory = write_root + "/TestRemoveResourceProject";
  cleanup(projectDirectory);
  std::string projectFileLocation = projectDirectory + "/foo.smtk";

  auto managers = smtk::common::Managers::create();
  // Construct smtk managers
  auto resourceRegistry = smtk::plugin::addToManagers<smtk::resource::Registrar>(managers);
  auto attributeRegistry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(managers);
  auto operationRegistry = smtk::plugin::addToManagers<smtk::operation::Registrar>(managers);
  auto projectRegistry = smtk::plugin::addToManagers<smtk::project::Registrar>(managers);
  // access smtk managers
  auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
  auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
  auto projectManager = managers->get<smtk::project::Manager::Ptr>();

  // Initialize operations
  auto attributeOpRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager, operationManager);
  auto operationOpRegistry =
    smtk::plugin::addToManagers<smtk::operation::Registrar>(resourceManager, operationManager);
#ifdef VTK_SUPPORT
  auto vtkRegistry =
    smtk::plugin::addToManagers<smtk::session::vtk::Registrar>(resourceManager, operationManager);
#endif
  auto projectOpRegistry =
    smtk::plugin::addToManagers<smtk::project::Registrar>(resourceManager, projectManager);

  // Register a new project type
  projectManager->registerProject("foo");

  // Create a project and write it to disk.
  std::size_t numberOfResources = 0;
  smtk::resource::ResourcePtr resource;
  smtk::project::Project::Ptr project = projectManager->create("foo");
  smtkTest(!!project, "Failed to create a project");

  {
    // Create an import operator
    smtk::operation::Operation::Ptr importAnyOp =
      operationManager->create<smtk::operation::ImportResource>();
    smtk::operation::Operation::Ptr importSBTOp =
      operationManager->create<smtk::attribute::Import>();

    smtkTest(!!importSBTOp, "No sbt import operator");
    smtkTest(!!importAnyOp, "No any import operator");

    // Input data files
    std::map<std::string, std::string> importPaths = {
      { "my attributes", "/attribute/attribute_collection/elasticity.sbt" }
    };
#ifdef VTK_SUPPORT
    importPaths["my model"] = "/model/3d/genesis/pillbox4.gen";
#endif
    for (auto& keyval : importPaths)
    {
      auto role = keyval.first;
      auto path = keyval.second;

      std::string importFilePath(data_root);
      importFilePath += path;
      std::string ext = boost::filesystem::path(importFilePath).extension().string();

      smtk::operation::Operation::Result importOpResult;
      if (ext == ".sbt")
      {
        importSBTOp->parameters()->findFile("filename")->setValue(importFilePath);
        importOpResult = importSBTOp->operate();
      }
      else
      {
        importAnyOp->parameters()->findFile("filename")->setValue(importFilePath);
        importOpResult = importAnyOp->operate();
      }

      auto resourceItem = importOpResult->findResource("resource");
      project->resources().add(resourceItem->value(), role);
      if (!resource)
      {
        resource = resourceItem->value();
      }
      ++numberOfResources;
    }
  }

  smtkTest(
    project->resources().size() == static_cast<std::size_t>(numberOfResources),
    "Failed to add a resource to the project");
  {
    // Create a write operator
    smtk::operation::WriteResource::Ptr writeOp =
      operationManager->create<smtk::operation::WriteResource>();

    smtkTest(!!writeOp, "No write operator");

    writeOp->parameters()->associate(project);
    writeOp->parameters()->findFile("filename")->setIsEnabled(true);
    writeOp->parameters()->findFile("filename")->setValue(projectFileLocation);

    smtkTest(writeOp->ableToOperate(), "Write operation unable to operate");

    // Execute the operation
    smtk::operation::Operation::Result writeOpResult = writeOp->operate();
  }

  std::set<smtk::attribute::Resource::Ptr> myAttSet =
    project->resources().findByRole<smtk::attribute::Resource>("my attributes");

  smtkTest(!myAttSet.empty(), "Resulting project does not contain attribute resource");

  smtkTest(
    myAttSet.size() == 1,
    "Resulting project contains more than one(1) attribute resource with role \"my "
    "attributes\"");

  smtk::attribute::Resource::Ptr myAtts = *(myAttSet.begin());

  smtkTest(myAtts->clean(), "Attribute resource is marked modified");

  {
    std::vector<smtk::attribute::DefinitionPtr> defList;
    myAtts->definitions(defList);
    smtkTest(defList.size() == 3, "Attribute resource missing definitions: " << defList.size());
  }

  smtkTest(
    projectManager->projects().size() == 1 && resourceManager->size() == numberOfResources,
    "Should have project and loaded resources");

  // test resource removal does not occur when part of a project
  smtk::operation::Operation::Ptr removeOp =
    operationManager->create<smtk::operation::RemoveResource>();
  removeOp->parameters()->associate(resource);
  removeOp->operate();
  smtkTest(resourceManager->size() == numberOfResources, "Should still have loaded resources");

  // test removing the project
  removeOp->parameters()->removeAllAssociations();
  removeOp->parameters()->associate(project);
  removeOp->operate();
  smtkTest(
    resourceManager->empty(),
    "Should have no resources after removing project " << resourceManager->size());
  // projectManager->remove(project);
  smtkTest(projectManager->projects().empty(), "Should have no projects");

#ifdef NDEBUG
  cleanup(projectDirectory);
#endif

  return 0;
}
