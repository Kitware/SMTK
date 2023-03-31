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

#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

#include "smtk/project/Operation.h"
#include "smtk/project/Project.h"

#include "smtk/project/operators/Read.h"
#include "smtk/project/operators/Write.h"

#include "smtk/common/UUIDGenerator.h"

namespace
{

void InitializeObserver(smtk::project::Manager* manager, smtk::project::Observer fn)
{
  if (!manager || !fn)
  {
    return;
  }

  for (const auto& project : manager->projects())
  {
    fn(*project, smtk::project::EventType::ADDED);
  }
}

void InitializeMetadataObserver(smtk::project::Manager* manager, smtk::project::MetadataObserver fn)
{
  if (!manager || !fn)
  {
    return;
  }

  for (const auto& metadata : manager->metadata())
  {
    fn(metadata, true);
  }
}
} // namespace

namespace smtk
{
namespace project
{
Manager::Manager(
  const smtk::resource::ManagerPtr& resourceManager,
  const smtk::operation::ManagerPtr& operationManager)
  : m_observers(std::bind(InitializeObserver, this, std::placeholders::_1))
  , m_metadataObservers(std::bind(InitializeMetadataObserver, this, std::placeholders::_1))
  , m_resourceManager(resourceManager)
  , m_operationManager(operationManager)
{
  // Define a metadata observer that appends the assignment of the project
  // manager to the create functor for operations that inherit from
  // smtk::project::Operation.
  auto operationMetadataObserver = [&, this](const smtk::operation::Metadata& md, bool adding) {
    if (!adding)
      return;

    // We are only interested in operations that inherit from
    // smtk::project::Operation.
    if (std::dynamic_pointer_cast<smtk::project::Operation>(md.create()) == nullptr)
    {
      return;
    }

    // This metadata observer actually manipulates the metadata, so we need a
    // const cast. This is an exception to the rule of metadata observers.
    smtk::operation::Metadata& metadata = const_cast<smtk::operation::Metadata&>(md);

    auto create = metadata.create;
    metadata.create = [=]() {
      auto op = create();
      std::dynamic_pointer_cast<smtk::project::Operation>(op)->setProjectManager(
        this->shared_from_this());
      return op;
    };
  };

  // Add this metadata observer to the set of metadata observers, invoking it
  // immediately on all extant metadata.
  m_operationMetadataObserverKey = operationManager->metadataObservers().insert(
    operationMetadataObserver,
    "Append the assignment of the project manager to the create functor "
    "for operations that inherit from smtk::project::Operation");
}

bool Manager::registerProject(
  const std::string& name,
  const std::set<std::string>& resources,
  const std::set<std::string>& operations,
  const std::string& version)
{
  return registerProject(Metadata(
    name,
    std::hash<std::string>{}(name),
    [name](
      const smtk::common::UUID& id, const std::shared_ptr<smtk::common::Managers>&) -> ProjectPtr {
      ProjectPtr project = Project::create(name);
      project->setId(id);
      return project;
    },
    resources,
    operations,
    version));
}

bool Manager::registerProject(Metadata&& metadata)
{
  auto alreadyRegisteredMetadata = m_metadata.get<IndexTag>().find(metadata.index());
  if (alreadyRegisteredMetadata == m_metadata.get<IndexTag>().end())
  {
    auto inserted = m_metadata.get<IndexTag>().insert(metadata);
    if (inserted.second)
    {
      if (auto operationManager = m_operationManager.lock())
      {
        if (!operationManager->registered<smtk::project::Read>())
        {
          operationManager->registerOperation<smtk::project::Read>();
        }
        if (!operationManager->registered<smtk::project::Write>())
        {
          operationManager->registerOperation<smtk::project::Write>();
        }
      }

      if (auto resourceManager = m_resourceManager.lock())
      {
        smtk::resource::Resource::Index resourceIndex =
          std::type_index(typeid(smtk::resource::Resource)).hash_code();
        smtk::resource::Resource::Index projectIndex =
          std::type_index(typeid(smtk::project::Project)).hash_code();
        resourceManager->registerResource(smtk::resource::Metadata(
          inserted.first->typeName(),
          inserted.first->index(),
          { resourceIndex, projectIndex, metadata.index() },
          inserted.first->create,
          &smtk::project::read,
          &smtk::project::write));
      }
      smtk::operation::ReaderGroup(this->operationManager())
        .registerOperation(
          smtk::common::typeName<smtk::project::Read>(), inserted.first->typeName());
      smtk::operation::WriterGroup(this->operationManager())
        .registerOperation(
          smtk::common::typeName<smtk::project::Write>(), inserted.first->typeName());

      m_metadataObservers(*inserted.first, true);
      return true;
    }
  }

  return false;
}

bool Manager::unregisterProject(const std::string& typeName)
{
  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata != m_metadata.get<NameTag>().end())
  {
    m_metadata.get<NameTag>().erase(metadata);
    m_metadataObservers(*metadata, false);
    return true;
  }

  return false;
}

bool Manager::unregisterProject(const Project::Index& index)
{
  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata != m_metadata.get<IndexTag>().end())
  {
    m_metadata.get<IndexTag>().erase(metadata);
    return true;
  }

  return false;
}

bool Manager::registered(const std::string& typeName) const
{
  const auto metadata = m_metadata.get<NameTag>().find(typeName);
  return metadata != m_metadata.get<NameTag>().end();
}

bool Manager::registered(const Project::Index& index) const
{
  const auto metadata = m_metadata.get<IndexTag>().find(index);
  return metadata != m_metadata.get<IndexTag>().end();
}

bool Manager::registerOperation(smtk::operation::Metadata&& metadata)
{
  if (auto operationManager = this->operationManager())
  {
    return operationManager->registered(metadata.typeName())
      ? true
      : operationManager->registerOperation(std::move(metadata));
  }
  return false;
}

bool Manager::unregisterOperation(const std::string& typeName)
{
  if (auto operationManager = this->operationManager())
  {
    return operationManager->unregisterOperation(typeName);
  }
  return false;
}

bool Manager::unregisterOperation(const smtk::operation::Operation::Index& index)
{
  if (auto operationManager = this->operationManager())
  {
    return operationManager->unregisterOperation(index);
  }
  return false;
}

smtk::project::Project::Ptr Manager::create(
  const std::string& typeName,
  const std::shared_ptr<smtk::common::Managers>& m)
{
  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata != m_metadata.get<NameTag>().end())
  {
    // Create the resource using its index
    return this->create(metadata->index(), m);
  }

  return smtk::project::Project::Ptr();
}

smtk::project::Project::Ptr Manager::create(
  const Project::Index& index,
  const std::shared_ptr<smtk::common::Managers>& m)
{
  smtk::common::UUID uuid;

  // Though the chances are super small, we ensure here that the UUID associated
  // to our new project is unique to the manager's set of resources.
  do
  {
    uuid = smtk::common::UUIDGenerator::instance().random();
  } while (m_projects.find(uuid) != m_projects.end());

  return this->create(index, uuid, m);
}

smtk::project::Project::Ptr Manager::create(
  const std::string& typeName,
  const smtk::common::UUID& id,
  const std::shared_ptr<smtk::common::Managers>& m)
{
  smtk::project::ProjectPtr project;

  // Locate the metadata associated with this project type
  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata != m_metadata.get<NameTag>().end())
  {
    // Create the project with the appropriate UUID
    project = metadata->create(id, m);
    this->add(metadata->index(), project);
    if (project)
    {
      project->taskManager().setManagers(m);
    }
  }

  return project;
}

smtk::project::Project::Ptr Manager::create(
  const Project::Index& index,
  const smtk::common::UUID& id,
  const std::shared_ptr<smtk::common::Managers>& mm)
{
  smtk::project::ProjectPtr project;

  // Locate the metadata associated with this project type
  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata != m_metadata.get<IndexTag>().end())
  {
    // Create the project with the appropriate UUID
    project = metadata->create(id, mm);
    if (project)
    {
      project->taskManager().setManagers(mm);
    }
    this->add(index, project);
  }

  return project;
}

smtk::project::ProjectPtr Manager::get(const smtk::common::UUID& id)
{
  // No type casting is required, so we simply find and return the project by
  // key.
  typedef Container::index<IdTag>::type ProjectsById;
  ProjectsById& projects = m_projects.get<IdTag>();
  ProjectsById::iterator projectIt = projects.find(id);
  if (projectIt != projects.end())
  {
    return (*projectIt)->shared_from_this();
  }

  return smtk::project::ProjectPtr();
}

smtk::project::ConstProjectPtr Manager::get(const smtk::common::UUID& id) const
{
  // No type casting is required, so we simply find and return the project by
  // key.
  typedef Container::index<IdTag>::type ProjectsById;
  const ProjectsById& projects = m_projects.get<IdTag>();
  ProjectsById::const_iterator projectIt = projects.find(id);
  if (projectIt != projects.end())
  {
    return (*projectIt)->shared_from_this();
  }

  return smtk::project::ConstProjectPtr();
}

smtk::project::ProjectPtr Manager::get(const std::string& url)
{
  // No type casting is required, so we simply find and return the project by
  // key.
  typedef Container::index<LocationTag>::type ProjectsByLocation;
  ProjectsByLocation& projects = m_projects.get<LocationTag>();
  ProjectsByLocation::iterator projectIt = projects.find(url);
  if (projectIt != projects.end())
  {
    return (*projectIt)->shared_from_this();
  }

  return smtk::project::ProjectPtr();
}

smtk::project::ConstProjectPtr Manager::get(const std::string& url) const
{
  // No type casting is required, so we simply find and return the project by
  // key.
  typedef Container::index<LocationTag>::type ProjectsByLocation;
  const ProjectsByLocation& projects = m_projects.get<LocationTag>();
  ProjectsByLocation::const_iterator projectIt = projects.find(url);
  if (projectIt != projects.end())
  {
    return (*projectIt)->shared_from_this();
  }

  return smtk::project::ConstProjectPtr();
}

std::set<smtk::project::ProjectPtr> Manager::find(const std::string& typeName)
{
  std::set<smtk::project::ProjectPtr> values;
  for (const auto& project : m_projects)
  {
    if (project->isOfType(typeName))
    {
      values.insert(project);
    }
  }
  return values;
}

std::set<smtk::project::ProjectPtr> Manager::find(const Project::Index& index)
{
  std::set<smtk::project::ProjectPtr> values;
  for (const auto& project : m_projects)
  {
    if (project->isOfType(index))
    {
      values.insert(project);
    }
  }
  return values;
}

bool Manager::add(const smtk::project::ProjectPtr& project)
{
  // If the project is null, do not add it to the manager
  if (!project)
  {
    return false;
  }

  return this->add(project->index(), project);
}

bool Manager::add(const Project::Index& index, const smtk::project::Project::Ptr& project)
{
  if (!project)
  {
    return false;
  }

  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata == m_metadata.get<IndexTag>().end())
  {
    return false;
  }

  smtk::project::Project::Ptr p = const_cast<ProjectPtr&>(project);

  {
    // Set the project's managers
    p->m_manager = this;
    p->resources().setManager(m_resourceManager);
    p->operations().setManager(m_operationManager);

    // Set the project's resource and operation filters
    p->resources().registerResources(metadata->resources());
    p->operations().registerOperations(metadata->operations());
  }

  m_projects.insert(project);

  // Tell observers we just added a project
  m_observers(*project, smtk::project::EventType::ADDED);

  return true;
}

bool Manager::remove(const smtk::project::ProjectPtr& project)
{
  // Remove the project from the manager's set of projects
  return m_projects.erase(project->id()) > 0;
}
} // namespace project
} // namespace smtk
