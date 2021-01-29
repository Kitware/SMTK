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
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/io/Logger.h"
#include "smtk/model/Resource.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"
#include "smtk/session/vtk/Registrar.h"

#include <boost/filesystem.hpp>

#include <string>
#include <vector>

namespace
{
const std::string DATA_ROOT = SMTK_DATA_DIR;
const std::string TEMP_ROOT = SMTK_SCRATCH_DIR;
const std::string ORIG_ROOT = TEMP_ROOT + "/project-orig";
const std::string COPY_ROOT = TEMP_ROOT + "/project-copy";

class ProjectBuilder
{
public:
  ProjectBuilder(smtk::project::ManagerPtr& projectManager);
  void createProject();
  void copyProject();

protected:
  smtk::project::ManagerPtr m_projectManager;
  smtk::project::ProjectPtr m_firstProject;
  smtk::project::ProjectPtr m_secondProject;
}; // class ProjectBuilder

ProjectBuilder::ProjectBuilder(smtk::project::ManagerPtr& manager)
  : m_projectManager(manager)
{
}

void ProjectBuilder::createProject()
{
  // Configure new-project specification
  auto spec = m_projectManager->getProjectSpecification();
  spec->findDirectory("workspace-path")->setValue(TEMP_ROOT);
  spec->findString("project-folder")->setValue("project-orig");

  boost::filesystem::path dataRoot(DATA_ROOT);
  boost::filesystem::path templatePath =
    dataRoot / "attribute/attribute_collection/Basic2DFluid.sbt";
  boost::filesystem::path modelPath = dataRoot / "model/3d/genesis/casting-mesh1.gen";

  spec->findFile("simulation-template")->setValue(templatePath.string());
  auto modelGroupItem = spec->findGroup("model-group");
  modelGroupItem->setIsEnabled(true);
  modelGroupItem->findAs<smtk::attribute::FileItem>("model-file")->setValue(modelPath.string());
  modelGroupItem->findAs<smtk::attribute::VoidItem>("copy-file")->setIsEnabled(true);

  // Create the project
  m_firstProject = m_projectManager->createProject(spec);
  smtkTest(m_firstProject != nullptr, "failed to create first project");
  smtkTest(m_projectManager->saveProject(), "failed to save first project");
}

void ProjectBuilder::copyProject()
{
  auto logger = smtk::io::Logger::instance();
  m_secondProject = m_projectManager->saveAsProject(COPY_ROOT, logger);
  smtkTest(m_secondProject != nullptr, "failed to copy project");
  smtkTest(
    m_projectManager->getCurrentProject() == m_secondProject, "failed to update current project");

  // Sanity check that the .smtkproject file created
  auto projectFilePath = boost::filesystem::path(COPY_ROOT) / ".smtkproject";
  smtkTest(
    boost::filesystem::exists(projectFilePath), "copied project is missing .smtkproject file");

  // Also check that .gen file was copied
  auto nativePath = boost::filesystem::path(COPY_ROOT) / "casting-mesh1.gen";
  smtkTest(boost::filesystem::exists(nativePath), "native model file not copied");
}

} // namespace

int TestProjectCopy(int /*unused*/, char** const /*unused*/)
{
  // Clear out orig and copy folders
  std::vector<std::string> folderList = { ORIG_ROOT, COPY_ROOT };
  for (auto folder : folderList)
  {
    boost::filesystem::path p(folder);
    boost::filesystem::remove_all(p);
  }

  auto resManager = smtk::resource::Manager::create();
  auto opManager = smtk::operation::Manager::create();
  auto projectManager = smtk::project::Manager::create(resManager, opManager);
  {
    // Initialize smtk managers
    smtk::attribute::Registrar::registerTo(resManager);
    smtk::attribute::Registrar::registerTo(opManager);
    smtk::session::vtk::Registrar::registerTo(resManager);
    smtk::session::vtk::Registrar::registerTo(opManager);
    smtk::operation::Registrar::registerTo(opManager);
    opManager->registerResourceManager(resManager);

    ProjectBuilder builder(projectManager);
    builder.createProject();
    builder.copyProject();

    smtkTest(projectManager->closeProject(), "error closing copied project");
  }

  // Sanity check
  smtkTest(resManager->empty(), "resource manager not empty");

  // Delete original project and reload copy
  boost::filesystem::path origPath(ORIG_ROOT);
  boost::filesystem::remove_all(origPath);

  auto project = projectManager->openProject(COPY_ROOT);
  smtkTest(project != nullptr, "Unable to reload copied project");

  auto attResource = project->findResource<smtk::attribute::Resource>("default");
  smtkTest(attResource != nullptr, "copied attribute resource missing");
  auto modelResource = project->findResource<smtk::model::Resource>("default");
  smtkTest(modelResource != nullptr, "copied model resource missing");
  bool hasAssoc = attResource->associations().count(modelResource) == 1;
  smtkTest(hasAssoc, "copied attribute resource not associated to model resource");

  // Close and delete the project files
  smtkTest(projectManager->closeProject(), "failed closing copied project");
  boost::filesystem::path copyPath(COPY_ROOT);
  boost::filesystem::remove_all(copyPath);

  return 0;
}
