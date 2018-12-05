//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/Manager.h"
#include "smtk/project/NewProjectTemplate.h"
#include "smtk/project/Project.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

namespace smtk
{
namespace project
{
smtk::project::ManagerPtr Manager::create(
  smtk::resource::ManagerPtr& resourceManager, smtk::operation::ManagerPtr& operationManager)
{
  if (!resourceManager || !operationManager)
  {
    return smtk::project::ManagerPtr();
  }

  return smtk::project::ManagerPtr(new Manager(resourceManager, operationManager));
}

Manager::Manager(
  smtk::resource::ManagerPtr& resourceManager, smtk::operation::ManagerPtr& operationManager)
  : m_resourceManager(resourceManager)
  , m_operationManager(operationManager)
{
  resourceManager->registerResource<smtk::attribute::Resource>();
}

Manager::~Manager()
{
  if (this->m_project)
  {
    this->m_project->close();
  }
}

smtk::attribute::AttributePtr Manager::getProjectSpecification() const
{
  auto newTemplate = this->getProjectTemplate();
  std::string name = "new-project";
  auto defn = newTemplate->findDefinition(name);
  auto att = newTemplate->createAttribute(name, defn);
  return att;
}

ProjectPtr Manager::createProject(smtk::attribute::AttributePtr specification,
  bool replaceExistingDirectory, smtk::io::Logger& logger)
{
  auto newProject = smtk::project::Project::create();
  newProject->setCoreManagers(this->m_resourceManager, this->m_operationManager);
  bool success = newProject->build(specification, logger, replaceExistingDirectory);
  if (success)
  {
    this->m_project = newProject;
    return newProject;
  }

  // (else)
  return smtk::project::ProjectPtr();
}

bool Manager::saveProject(smtk::io::Logger& logger)
{
  if (!this->m_project)
  {
    smtkErrorMacro(logger, "No current project to save.");
    return false;
  }

  return this->m_project->save(logger);
}

bool Manager::closeProject(smtk::io::Logger& logger)
{
  if (!this->m_project)
  {
    smtkErrorMacro(logger, "No current project to close.");
    return false;
  }

  bool closed = this->m_project->close();
  if (closed)
  {
    this->m_project = nullptr;
  }

  return closed;
}

ProjectPtr Manager::openProject(const std::string& projectPath, smtk::io::Logger& logger)
{
  if (this->m_project)
  {
    smtkErrorMacro(logger, "Cannot open project - must close current project first");
    return smtk::project::ProjectPtr();
  }

  auto project = smtk::project::Project::create();
  project->setCoreManagers(this->m_resourceManager, this->m_operationManager);
  bool success = project->open(projectPath, logger);
  if (success)
  {
    this->m_project = project;
    return project;
  }

  // (else)
  return smtk::project::ProjectPtr();
}

smtk::attribute::ResourcePtr Manager::getProjectTemplate() const
{
  auto reader = smtk::io::AttributeReader();
  auto newTemplate = smtk::attribute::Resource::create();
  reader.readContents(newTemplate, NewProjectTemplate, smtk::io::Logger::instance());
  return newTemplate;
}

} // namespace project
} // namespace smtk
