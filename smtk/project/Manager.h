//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_Manager_h
#define smtk_project_Manager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/TypeName.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"

#include "smtk/project/Container.h"
#include "smtk/project/Metadata.h"
#include "smtk/project/MetadataContainer.h"
#include "smtk/project/MetadataObserver.h"
#include "smtk/project/Observer.h"

#include "smtk/resource/Manager.h"

#include <set>
#include <tuple>
#include <type_traits>

namespace smtk
{
namespace project
{

/// A Manager for projects. Like other managers in SMTK, the Project Manager
/// has functionality for registering Project types and for creating and
/// accessing Projects.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypedefs(smtk::project::Manager);

  static std::shared_ptr<Manager> create(
    const smtk::resource::ManagerPtr& resourceManager,
    const smtk::operation::ManagerPtr& operationManager)
  {
    return smtk::shared_ptr<Manager>(new Manager(resourceManager, operationManager));
  }

  virtual ~Manager() = default;

  /// Register a project identified by its class type, available resources and
  /// operations, and its version.
  template<
    typename ProjectType,
    typename ResourcesTuple = std::tuple<>,
    typename OperationsTuple = std::tuple<>>
  bool registerProject(const std::string& version = "0.0.0");

  /// Register a project identified by a unique name, available resources and
  /// operations, and its version. . The resulting project will simply be an
  /// instance of smtk::project::Project.
  bool registerProject(
    const std::string& name,
    const std::set<std::string>& resources = {},
    const std::set<std::string>& operations = {},
    const std::string& version = "0.0.0");

  /// Register a project identified by its metadata.
  bool registerProject(smtk::project::Metadata&&);

  /// Unregister a project identified by its typename, type index or class type.
  bool unregisterProject(const std::string&);
  bool unregisterProject(const Project::Index&);
  template<typename ProjectType>
  bool unregisterProject();

  /// Register an operation identified by its class type or Metadata.
  template<typename OperationType>
  bool registerOperation();
  bool registerOperation(smtk::operation::Metadata&&);

  /// Register a tuple of operations identified by their class types.
  template<typename Tuple>
  bool registerOperations()
  {
    return Manager::registerOperations<0, Tuple>();
  }

  /// Check if a project identified by its typename, type index or class type is
  /// registered.
  bool registered(const std::string&) const;
  bool registered(const Project::Index&) const;
  template<typename ProjectType>
  bool registered() const;

  /// Unegister an operation identified by its typename, type index or class type.
  bool unregisterOperation(const std::string&);
  bool unregisterOperation(const smtk::operation::Operation::Index&);
  template<typename OperationType>
  bool unregisterOperation();

  // Unregister a tuple of operations identified by their class types.
  template<typename Tuple>
  bool unregisterOperations()
  {
    return Manager::unregisterOperations<0, Tuple>();
  }

  /// Construct a project identified by its typename, type index or class type.
  ProjectPtr create(const std::string&, const std::shared_ptr<smtk::common::Managers>& = nullptr);
  ProjectPtr create(
    const Project::Index&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);
  template<typename ProjectType>
  smtk::shared_ptr<ProjectType> create(const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Construct a project with a given UUID identified by its typename, type
  /// index or class type.
  ProjectPtr create(
    const std::string&,
    const smtk::common::UUID&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);
  ProjectPtr create(
    const Project::Index&,
    const smtk::common::UUID&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);
  template<typename ProjectType>
  smtk::shared_ptr<ProjectType> create(
    const smtk::common::UUID&,
    const std::shared_ptr<smtk::common::Managers>& = nullptr);

  /// Returns the project that relates to the given uuid.  If no association
  /// exists this will return a null pointer
  ProjectPtr get(const smtk::common::UUID& id);
  ConstProjectPtr get(const smtk::common::UUID& id) const;
  template<typename ProjectType>
  smtk::shared_ptr<ProjectType> get(const smtk::common::UUID&);
  template<typename ProjectType>
  smtk::shared_ptr<const ProjectType> get(const smtk::common::UUID&) const;

  /// Returns the project that relates to the given url.  If no association
  /// exists this will return a null pointer
  ProjectPtr get(const std::string&);
  ConstProjectPtr get(const std::string&) const;
  template<typename ProjectType>
  smtk::shared_ptr<ProjectType> get(const std::string&);
  template<typename ProjectType>
  smtk::shared_ptr<const ProjectType> get(const std::string&) const;

  /// Returns a set of projects that have a given typename, type index or class
  /// type.
  std::set<ProjectPtr> find(const std::string&);
  std::set<ProjectPtr> find(const Project::Index&);
  template<typename ProjectType>
  std::set<smtk::shared_ptr<ProjectType>> find();

  /// Add a project identified by its type index. Returns true if the project
  /// was added or already is part of this manager. If the project is currently
  /// part of a different manager, we will reparent it to this manager.
  bool add(const Project::Index&, const ProjectPtr&);

  /// Add a project identified by its class type. Returns true if the project
  /// was added or already is part of this manager.
  template<typename ProjectType>
  bool add(const smtk::shared_ptr<ProjectType>&);

  /// Returns true if the project was added or already is part of this manager.
  bool add(const ProjectPtr&);

  /// Removes a project from a given Manager. This doesn't explicitly release
  /// the memory of the project, it only stops the tracking of the project
  /// by the manager.
  bool remove(const ProjectPtr&);

  /// Return the set of projects.
  Container& projects() { return m_projects; }
  const Container& projects() const { return m_projects; }
  std::set<smtk::project::ProjectPtr> projectsSet() const;

  /// Return the map of metadata.
  MetadataContainer& metadata() { return m_metadata; }

  /// Return the observers associated with this manager.
  Observers& observers() { return m_observers; }
  const Observers& observers() const { return m_observers; }

  /// Return the metadata observers associated with this manager.
  Metadata::Observers& metadataObservers() { return m_metadataObservers; }
  const Metadata::Observers& metadataObservers() const { return m_metadataObservers; }

  smtk::resource::ManagerPtr resourceManager() { return m_resourceManager.lock(); }
  smtk::operation::ManagerPtr operationManager() { return m_operationManager.lock(); }

private:
  Manager(
    const smtk::resource::ManagerPtr& resourceManager,
    const smtk::operation::ManagerPtr& operationManager);

  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  registerOperations()
  {
    bool registered = this->registerOperation<typename std::tuple_element<I, Tuple>::type>();
    return registered && Manager::registerOperations<I + 1, Tuple>();
  }

  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  registerOperations()
  {
    return true;
  }

  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  unregisterOperations()
  {
    bool unregistered = this->unregisterOperation<typename std::tuple_element<I, Tuple>::type>();
    return unregistered && Manager::unregisterOperations<I + 1, Tuple>();
  }

  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  unregisterOperations()
  {
    return true;
  }

  /// A container for projects.
  Container m_projects;

  /// A container for all registered project metadata.
  MetadataContainer m_metadata;

  /// A container for all project observers.
  Observers m_observers;

  /// A container for all project metadata observers.
  Metadata::Observers m_metadataObservers;

  std::weak_ptr<smtk::resource::Manager> m_resourceManager;
  std::weak_ptr<smtk::operation::Manager> m_operationManager;

  smtk::operation::MetadataObservers::Key m_operationMetadataObserverKey;

  /// An observer for m_operationManager that adds and removes projects
  /// to/from this manager as directed by Operation::Result items.
  smtk::operation::Observers::Key m_operationManagerObserver;
};

namespace detail
{
template<std::size_t I, typename Tuple>
inline typename std::enable_if<I != std::tuple_size<Tuple>::value>::type tupleToTypeNames(
  std::set<std::string>& typeNames)
{
  typeNames.insert(smtk::common::typeName<typename std::tuple_element<I, Tuple>::type>());
  return tupleToTypeNames<I + 1, Tuple>(typeNames);
}

template<std::size_t I, typename Tuple>
inline typename std::enable_if<I == std::tuple_size<Tuple>::value>::type tupleToTypeNames(
  std::set<std::string>& /*typeNames*/)
{
  return;
}

template<typename Tuple>
std::set<std::string> tupleToTypeNames()
{
  std::set<std::string> typeNames;
  tupleToTypeNames<0, Tuple>(typeNames);
  return typeNames;
}
} // namespace detail

template<typename ProjectType, typename ResourcesTuple, typename OperationsTuple>
bool Manager::registerProject(const std::string& version)
{
  return registerProject(Metadata(
    smtk::common::typeName<ProjectType>(),
    std::type_index(typeid(ProjectType)).hash_code(),
    [](const smtk::common::UUID& id, const std::shared_ptr<smtk::common::Managers>&) {
      Project::Ptr project = ProjectType::create();
      project->setId(id);
      return project;
    },
    detail::tupleToTypeNames<ResourcesTuple>(),
    detail::tupleToTypeNames<OperationsTuple>(),
    version));
}

template<typename ProjectType>
bool Manager::registered() const
{
  return this->registered(std::type_index(typeid(ProjectType)).hash_code());
}

template<typename OperationType>
bool Manager::registerOperation()
{
  // Operations that inherit from smtk::project::Operation have an API for
  // accessing a project manager. Often, the operation's specification will be
  // defined using the project manager. We therefore assign the operation's
  // project manager before constructing its specification.
  auto op = OperationType::create();
  op->setProjectManager(shared_from_this());
  auto specification = op->specification();

  return Manager::registerOperation(smtk::operation::Metadata(
    smtk::common::typeName<OperationType>(),
    std::type_index(typeid(OperationType)).hash_code(),
    specification,
    []() { return OperationType::create(); }));
}

template<typename OperationType>
bool Manager::unregisterOperation()
{
  return this->unregisterOperation(std::type_index(typeid(OperationType)).hash_code());
}

template<typename ProjectType>
std::shared_ptr<ProjectType> Manager::create(const std::shared_ptr<smtk::common::Managers>& m)
{
  return std::static_pointer_cast<ProjectType>(
    this->create(std::type_index(typeid(ProjectType)).hash_code(), m));
}

template<typename ProjectType>
std::shared_ptr<ProjectType> Manager::create(
  const smtk::common::UUID& id,
  const std::shared_ptr<smtk::common::Managers>& m)
{
  return std::static_pointer_cast<ProjectType>(
    this->create(std::type_index(typeid(ProjectType)).hash_code(), id, m));
}

template<typename ProjectType>
std::shared_ptr<ProjectType> Manager::get(const smtk::common::UUID& id)
{
  return std::static_pointer_cast<ProjectType>(this->get(id));
}

template<typename ProjectType>
std::shared_ptr<const ProjectType> Manager::get(const smtk::common::UUID& id) const
{
  return std::static_pointer_cast<const ProjectType>(this->get(id));
}

template<typename ProjectType>
std::shared_ptr<ProjectType> Manager::get(const std::string& url)
{
  return std::static_pointer_cast<ProjectType>(this->get(url));
}

template<typename ProjectType>
std::shared_ptr<const ProjectType> Manager::get(const std::string& url) const
{
  return std::static_pointer_cast<const ProjectType>(this->get(url));
}

template<typename ProjectType>
std::set<std::shared_ptr<ProjectType>> Manager::find()
{
  std::set<std::shared_ptr<Project>> tmp = this->find(smtk::common::typeName<ProjectType>());
  std::set<std::shared_ptr<ProjectType>> projects;
  for (const auto& project : tmp)
  {
    projects.insert(std::static_pointer_cast<ProjectType>(project));
  }
  return projects;
}

template<typename ProjectType>
bool Manager::add(const smtk::shared_ptr<ProjectType>& project)
{
  return this->add(std::type_index(typeid(ProjectType)).hash_code(), project);
}
} // namespace project
} // namespace smtk

#endif
