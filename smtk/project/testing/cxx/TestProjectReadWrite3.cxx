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
#include "smtk/attribute/ResourceItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/Registrar.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/WriteResource.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/project/Registrar.h"
#include "smtk/resource/Manager.h"

#include <boost/filesystem.hpp>

// This test verifies that *empty* projects can be serialized to the file system
// and unserialized back.

namespace
{
const int OP_SUCCEEDED = static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED);

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}
} // namespace

int TestProjectReadWrite3(int /*unused*/, char** const /*unused*/)
{
  // Create smtk managers
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();
  smtk::project::Registrar::registerTo(resourceManager);

  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();
  smtk::operation::Registrar::registerTo(operationManager);
  operationManager->registerResourceManager(resourceManager);

  smtk::project::ManagerPtr projectManager =
    smtk::project::Manager::create(resourceManager, operationManager);
  smtk::project::Registrar::registerTo(projectManager);
  projectManager->registerProject("foo");

  std::string projectLocation = write_root + "/empty-project.smtk";

  // Create empty project and write it to disk.
  {
    smtk::project::Project::Ptr project = projectManager->create("foo");

    smtk::operation::WriteResource::Ptr writeOp =
      operationManager->create<smtk::operation::WriteResource>();

    auto att = writeOp->parameters();
    bool ok = att->associate(project); // , "failed to associate project to writeOp");
    smtkTest(ok, "failed to associate project to writeOp");
    writeOp->parameters()->findFile("filename")->setIsEnabled(true);
    smtkTest(
      writeOp->parameters()->findFile("filename")->setValue(projectLocation),
      "failed to set filename item");

    smtk::operation::Operation::Result writeResult = writeOp->operate();
    int writeOutcome = writeResult->findInt("outcome")->value();
    smtkTest(writeOutcome == OP_SUCCEEDED, "failed to write project, outcome " << writeOutcome);
  }

  // Read the project back in
  {
    smtk::operation::ReadResource::Ptr readOp =
      operationManager->create<smtk::operation::ReadResource>();
    readOp->parameters()->findFile("filename")->setValue(projectLocation);
    smtk::operation::Operation::Result readResult = readOp->operate();
    int readOutcome = readResult->findInt("outcome")->value();
    smtkTest(readOutcome == OP_SUCCEEDED, "failed to read project, outcome " << readOutcome);

    // Make sure project is there too
    auto project = readResult->findResource("resource")->valueAs<smtk::project::Project>();
    smtkTest(project != nullptr, "failed to return project");
    smtkTest(project->clean(), "project is marked modified");
  }

  cleanup(projectLocation);

  return 0;
}
