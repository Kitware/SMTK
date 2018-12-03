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
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Manager.h"

#include <cassert>
#include <exception>

namespace
{
int UNABLE_TO_OPERATE =
  static_cast<int>(smtk::operation::Operation::Outcome::UNABLE_TO_OPERATE);       // 0
int FAILED = static_cast<int>(smtk::operation::Operation::Outcome::FAILED);       // 2
int SUCCEEDED = static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED); // 3
}

namespace smtk
{
namespace project
{
Manager::Manager()
{
}

Manager::~Manager()
{
}

void Manager::setCoreManagers(
  smtk::resource::ManagerPtr& resourceManager, smtk::operation::ManagerPtr& operationManager)
{
  if (this->m_project)
  {
    throw std::runtime_error("Cannot set internal managers with project open");
  }
  assert(!!resourceManager);
  assert(!!operationManager);
  this->m_resourceManager = resourceManager;
  this->m_resourceManager->registerResource<smtk::attribute::Resource>();
  this->m_operationManager = operationManager;
}

smtk::attribute::AttributePtr Manager::getProjectSpecification() const
{
  auto newTemplate = this->getProjectTemplate();
  std::string name = "new-project";
  auto defn = newTemplate->findDefinition(name);
  // std::cout << "defn:" << defn << std::endl;
  auto att = newTemplate->createAttribute(name, defn);
  return att;
}

std::tuple<int, ProjectPtr> Manager::createProject(smtk::attribute::AttributePtr specification)
{
  auto nullReturn = std::make_tuple(UNABLE_TO_OPERATE, smtk::project::ProjectPtr());
  if (this->m_project)
  {
    std::cerr << "Cannot create project - must close current project first" << std::endl;
    return nullReturn;
  }
  if (!this->m_resourceManager)
  {
    std::cerr << "Cannot create project - resource manager not set" << std::endl;
    return nullReturn;
  }
  if (!this->m_operationManager)
  {
    std::cerr << "Cannot create project - operation manager not set" << std::endl;
    return nullReturn;
  }

  auto newProject = smtk::project::Project::create();
  newProject->setCoreManagers(this->m_resourceManager, this->m_operationManager);
  int outcome = newProject->build(specification);
  if (outcome == SUCCEEDED)
  {
    this->m_project = newProject;
  }
  return std::make_tuple(SUCCEEDED, newProject);
}

int Manager::saveProject()
{
  if (!this->m_project)
  {
    return UNABLE_TO_OPERATE;
  }

  return this->m_project->save();
}

int Manager::closeProject()
{
  if (!this->m_project)
  {
    return UNABLE_TO_OPERATE;
  }

  return this->m_project->close();
}

std::tuple<int, ProjectPtr> Manager::openProject(const std::string& projectPath)
{
  smtk::project::ProjectPtr nullProject;
  auto nullReturn = std::make_tuple(UNABLE_TO_OPERATE, nullProject);
  if (!this->m_project)
  {
    std::cerr << "Cannot open project - must close current project first" << std::endl;
    return nullReturn;
  }
  if (!this->m_resourceManager)
  {
    std::cerr << "Cannot create project - must set resource manager first" << std::endl;
    return nullReturn;
  }
  if (!this->m_operationManager)
  {
    std::cerr << "Cannot create project - must set operation manager first" << std::endl;
    return nullReturn;
  }

  // ProjectPtr project = smtk::project::ProjectPtr::create();
  auto project = smtk::project::Project::create();
  project->setCoreManagers(this->m_resourceManager, this->m_operationManager);
  int outcome = project->open(projectPath);
  if (outcome == SUCCEEDED)
  {
    return std::make_tuple(SUCCEEDED, project);
  }

  // (else)
  return std::make_tuple(outcome, nullProject);
}

smtk::attribute::ResourcePtr Manager::getProjectTemplate() const
{
  auto reader = smtk::io::AttributeReader();
  auto newTemplate = smtk::attribute::Resource::create();
  auto logger = smtk::io::Logger();
  reader.readContents(newTemplate, NewProjectTemplate, logger);
  return newTemplate;
}

} // namespace project
} // namespace smtk
