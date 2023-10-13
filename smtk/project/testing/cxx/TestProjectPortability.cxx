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

// This test verifies that projects can be moved to a new location in the file
// system and still load succesfully. The test project contains an SMTK attribute
// resource plus (if vtk support is built) an SMTK vtk model resource.
//
// This

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

bool copyDir(const path& source, const path& destination, const path& ignore)
{
  // do error checking and create the destination folder
  try
  {
    // Check whether the function call is valid
    if (!exists(source) || !is_directory(source))
    {
      std::cerr << "Source directory " << source.string()
                << " does not exist or is not a "
                   "directory.\n";
      return false;
    }
    // Create the destination directory
    if (!exists(destination) && !create_directory(destination))
    {
      std::cerr << "Unable to create destination directory " << destination.string() << "\n";
      return false;
    }
  }
  catch (filesystem_error const& e)
  {
    std::cerr << e.what() << "\n";
    return false;
  }

  // Iterate through the source directory
  for (directory_iterator file(source); file != directory_iterator(); ++file)
  {
    try
    {
      path current(file->path());
      if (is_directory(current))
      {
        // avoid infintely recursive copying (since the target is in the source directory)
        if (current != ignore)
        {
          // Found directory: Recursion
          if (!copyDir(current, destination / current.filename(), ignore))
          {
            // if this return is triggered, the error message should already have been output
            return false;
          }
        }
      }
      else
      {
        // Found file: Copy
        copy_file(current, destination / current.filename());
      }
    }
    catch (filesystem_error const& e)
    {
      std::cerr << e.what() << "\n";
      return false;
    }
  }
  return true;
}
} // namespace

int TestProjectPortability(int /*unused*/, char** const /*unused*/)
{
  // Set the file path
  std::string projectDirectory = write_root + "/TestProjectPortability/original";
  cleanup(projectDirectory);
  std::string projectFileLocation = projectDirectory + "/foo.smtk";

  std::string newProjectDirectory = write_root + "/TestProjectPortability/moved";
  cleanup(newProjectDirectory);
  std::string newProjectFileLocation = newProjectDirectory + "/foo.smtk";

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Create common::Managers "application state".
  auto managers = smtk::common::Managers::create();
  managers->insertOrAssign(resourceManager);
  managers->insertOrAssign(operationManager);

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);
  operationManager->setManagers(managers);

  // Create a project manager
  smtk::project::ManagerPtr projectManager =
    smtk::project::Manager::create(resourceManager, operationManager);

  auto attributeRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager, operationManager);
#ifdef VTK_SUPPORT
  auto vtkRegistry =
    smtk::plugin::addToManagers<smtk::session::vtk::Registrar>(resourceManager, operationManager);
#endif
  auto operationRegistry = smtk::plugin::addToManagers<smtk::project::Registrar>(operationManager);
  auto projectRegistry =
    smtk::plugin::addToManagers<smtk::project::Registrar>(resourceManager, projectManager);

  // Register a new project type
  projectManager->registerProject("foo");

  // Create a project and write it to disk.
  size_t numberOfResources = 0;
  {
    smtk::project::Project::Ptr project = projectManager->create("foo");
    if (!project)
    {
      std::cerr << "Failed to create a project\n";
      return 1;
    }
    projectManager->add(
      project->index(), project); // We didn't run a Create operation so we must add manually.

    {
      // Create an import operator
      smtk::operation::Operation::Ptr importAnyOp =
        operationManager->create<smtk::operation::ImportResource>();
      smtk::operation::Operation::Ptr importSBTOp =
        operationManager->create<smtk::attribute::Import>();

      if (!importSBTOp)
      {
        std::cerr << "No sbt import operator\n";
        return 1;
      }
      if (!importAnyOp)
      {
        std::cerr << "No any import operator\n";
        return 1;
      }

      // Input data files
      std::map<std::string, std::string> importPaths = {
        { "my attributes", "/attribute/attribute_collection/DoubleItemExample.sbt" }
      };
#ifdef VTK_SUPPORT
      importPaths["my model"] = "/model/3d/genesis/casting-mesh1.gen";
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
        ++numberOfResources;
      } // for (path)
    }

    if (project->resources().size() != static_cast<std::size_t>(numberOfResources))
    {
      std::cerr << "Failed to add a resource to the project\n";
      return 1;
    }

    {
      std::set<smtk::attribute::Resource::Ptr> myAtts =
        project->resources().findByRole<smtk::attribute::Resource>("my attributes");
      numberOfResources = myAtts.size();
    }

    {
      // Create a write operator
      smtk::operation::WriteResource::Ptr writeOp =
        operationManager->create<smtk::operation::WriteResource>();

      if (!writeOp)
      {
        std::cerr << "No write operator\n";
        return 1;
      }

      writeOp->parameters()->associate(project);
      writeOp->parameters()->findFile("filename")->setIsEnabled(true);
      writeOp->parameters()->findFile("filename")->setValue(projectFileLocation);

      if (!writeOp->ableToOperate())
      {
        std::cerr << "Write operation unable to operate\n";
        return 1;
      }

      // Execute the operation
      smtk::operation::Operation::Result writeOpResult = writeOp->operate();
    }
    projectManager->remove(project);
  }

  // copy the entire project folder to a new location
  if (!copyDir(projectDirectory, newProjectDirectory, newProjectDirectory))
  {
    std::cerr << "Copying the project to a new location failed\n";
    return 1;
  }

  // delete the original project folder
  remove_all(projectDirectory);

  // Read the project
  smtk::project::Project::Ptr project;
  {
    // Create a read operator
    smtk::operation::ReadResource::Ptr readOp =
      operationManager->create<smtk::operation::ReadResource>();

    if (!readOp)
    {
      std::cerr << "No read operator\n";
      return 1;
    }

    readOp->parameters()->findFile("filename")->setValue(newProjectFileLocation);

    if (!readOp->ableToOperate())
    {
      std::cerr << "Read operation unable to operate\n";
      return 1;
    }

    // Execute the operation
    smtk::operation::Operation::Result readOpResult = readOp->operate();

    // Test for success
    if (
      readOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Read operation failed\n";
      std::cerr << readOp->log().convertToString();
      return 1;
    }

    project = readOpResult->findResource("resource")->valueAs<smtk::project::Project>();

    if (!project)
    {
      std::cerr << "Resulting project is invalid\n";
      return 1;
    }
  }

  std::set<smtk::attribute::Resource::Ptr> myAttSet =
    project->resources().findByRole<smtk::attribute::Resource>("my attributes");

  if (myAttSet.empty())
  {
    std::cerr << "Resulting project does not contain attribute resource\n";
    return 1;
  }

  if (myAttSet.size() > 1)
  {
    std::cerr << "Resulting project contains more than one(1) attribute resource with role \"my "
                 "attributes\"\n";
    return 1;
  }

  smtk::attribute::Resource::Ptr myAtts = *(myAttSet.begin());

  if (!myAtts->clean())
  {
    std::cerr << "Attribute resource is marked modified\n";
    return 1;
  }

  {
    boost::filesystem::path myAttsPath(myAtts->location());
    boost::filesystem::path projectPath(newProjectDirectory);
    boost::filesystem::path resourcesPath = projectPath / "resources";
    if (myAttsPath.parent_path().compare(resourcesPath) != 0)
    {
      std::cerr << "Wrong attribute resource location: " << myAtts->location() << "\n";
      return 1;
    }
  }

  {
    std::vector<smtk::attribute::DefinitionPtr> defList;
    myAtts->definitions(defList);
    if (defList.size() != 2)
    {
      std::cerr << "Attribute resource missing definitions\n";
      return 1;
    }
  }

#ifdef NDEBUG
  cleanup(projectDirectory);
#endif

  return 0;
}
