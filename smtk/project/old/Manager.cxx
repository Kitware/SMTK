//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/old/Manager.h"
#include "smtk/project/old/NewProjectTemplate.h"
#include "smtk/project/old/Project.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#include "boost/filesystem.hpp"

namespace smtk
{
namespace project
{
namespace old
{
smtk::project::old::ManagerPtr Manager::create(
  smtk::resource::ManagerPtr& resourceManager,
  smtk::operation::ManagerPtr& operationManager)
{
  if (!resourceManager || !operationManager)
  {
    return smtk::project::old::ManagerPtr();
  }

  return smtk::project::old::ManagerPtr(new Manager(resourceManager, operationManager));
}

Manager::Manager(
  smtk::resource::ManagerPtr& resourceManager,
  smtk::operation::ManagerPtr& operationManager)
  : m_resourceManager(resourceManager)
  , m_operationManager(operationManager)
{
  resourceManager->registerResource<smtk::attribute::Resource>();
}

Manager::~Manager()
{
  if (m_project)
  {
    m_project->close();
  }
}

smtk::attribute::AttributePtr Manager::getProjectSpecification()
{
  auto newTemplate = this->getProjectTemplate();
  std::string name = "new-project";
  auto att = newTemplate->findAttribute(name);
  if (!att)
  {
    auto defn = newTemplate->findDefinition(name);
    att = newTemplate->createAttribute(name, defn);
  }
  return att;
}

ProjectPtr Manager::createProject(
  smtk::attribute::AttributePtr specification,
  bool replaceExistingDirectory,
  smtk::io::Logger& logger)
{
  if (m_project)
  {
    smtkErrorMacro(logger, "Cannot initalize new project - must close current project first");
    return ProjectPtr();
  }

  auto newProject = this->initProject(logger);
  if (!newProject)
  {
    return newProject;
  }

  bool success = newProject->build(specification, logger, replaceExistingDirectory);
  if (success)
  {
    m_project = newProject;
    return newProject;
  }

  // (else)
  return smtk::project::old::ProjectPtr();
}

bool Manager::saveProject(smtk::io::Logger& logger)
{
  if (!m_project)
  {
    smtkErrorMacro(logger, "No current project to save.");
    return false;
  }

  return m_project->save(logger);
}

ProjectPtr Manager::saveAsProject(const std::string& directory, smtk::io::Logger& logger)
{
  if (m_project == nullptr)
  {
    smtkErrorMacro(logger, "No current project to copy");
    return nullptr;
  }

  auto projectPath = boost::filesystem::path(directory);
  if (boost::filesystem::exists(projectPath) && !boost::filesystem::is_empty(projectPath))
  {
    smtkErrorMacro(logger, "Cannot copy project to non-empty directory: \"" << directory << "\"");
    return nullptr;
  } // if (projectPath exists)

  if (!boost::filesystem::exists(projectPath) && !boost::filesystem::create_directory(projectPath))
  {
    smtkErrorMacro(logger, "Unable to create project directory: \"" << directory << "\"");
    return nullptr;
  }

  // Build the new project
  auto newProject = this->initProject(logger);
  if (newProject == nullptr)
  {
    return nullptr;
  }

  // Set location and name
  newProject->m_name = projectPath.filename().string();
  newProject->m_directory = projectPath.string();

  if (!m_project->copyTo(newProject, logger))
  {
    smtkErrorMacro(logger, "Failed copying project content");
    return nullptr;
  }

  if (!newProject->save(logger))
  {
    return nullptr;
  }

  //Finally, close the current project and switch to the new project
  m_project->close();
  m_project = newProject;

  return newProject;
}

bool Manager::closeProject(smtk::io::Logger& logger)
{
  if (!m_project)
  {
    smtkErrorMacro(logger, "No current project to close.");
    return false;
  }

  bool closed = m_project->close();
  if (closed)
  {
    m_project = nullptr;
  }

  return closed;
}

ProjectPtr Manager::openProject(const std::string& projectPath, smtk::io::Logger& logger)
{
  auto newProject = this->initProject(logger);
  if (!newProject)
  {
    return newProject;
  }

  bool success = newProject->open(projectPath, logger);
  if (success)
  {
    m_project = newProject;
    return newProject;
  }

  // (else)
  return ProjectPtr();
}

smtk::operation::OperationPtr Manager::getExportOperator(smtk::io::Logger& logger, bool reset) const
{
  if (!m_project)
  {
    smtkErrorMacro(logger, "Cannot get export operator because no project is loaded");
    return nullptr;
  }

  return m_project->getExportOperator(logger, reset);
}

smtk::attribute::ResourcePtr Manager::getProjectTemplate()
{
  // The current presumption is to reuse the previous project settings.
  // This might be revisited for usability purposes.
  if (m_template)
  {
    return m_template;
  }

  auto reader = smtk::io::AttributeReader();
  m_template = smtk::attribute::Resource::create();
  reader.readContents(m_template, NewProjectTemplate, smtk::io::Logger::instance());
  return m_template;
}

ProjectPtr Manager::initProject(smtk::io::Logger& logger)
{
  auto resManager = m_resourceManager.lock();
  if (!resManager)
  {
    smtkErrorMacro(logger, "Resource manager is null");
    return ProjectPtr();
  }

  auto opManager = m_operationManager.lock();
  if (!opManager)
  {
    smtkErrorMacro(logger, "Operation manager is null");
    return ProjectPtr();
  }

  auto project = smtk::project::old::Project::create();
  project->setCoreManagers(resManager, opManager);
  return project;
}

} // namespace old
} // namespace project
} // namespace smtk
