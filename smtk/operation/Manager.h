//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_Manager_h
#define smtk_operation_Manager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"

#include "smtk/operation/Group.h"
#include "smtk/operation/Launcher.h"
#include "smtk/operation/Metadata.h"
#include "smtk/operation/MetadataContainer.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"

#include <array>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>

namespace smtk
{
namespace operation
{
/// An operation Manager is responsible for creating new operations and
/// filtering operations based on input type. Operation types must first be
/// registered with the Manager before operations of this type can be manipulated
/// by the manager.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypedefs(smtk::operation::Manager);
  smtkCreateMacro(Manager);

  virtual ~Manager();

  /// Register an operation identified by its class type.
  template<typename OperationType>
  bool registerOperation();

  /// Register an operation identified by its class type and type name.
  template<typename OperationType>
  bool registerOperation(const std::string&);

  /// Register an operation identified by its type index.
  bool registerOperation(Metadata&&);

  /// Register a tuple of operations identified by their class types.
  template<typename Tuple>
  bool registerOperations()
  {
    return Manager::registerOperations<0, Tuple>();
  }

  /// Register a tuple of operations identified by their class types and type
  /// names.
  template<typename Tuple>
  bool registerOperations(const std::array<std::string, std::tuple_size<Tuple>::value>& typeNames)
  {
    return Manager::registerOperations<0, Tuple>(typeNames);
  }

  /// Unegister an operation identified by its class type.
  template<typename OperationType>
  bool unregisterOperation();

  /// Unregister an operation identified by its type name.
  bool unregisterOperation(const std::string&);

  /// Unregister an operation identified by its type index.
  bool unregisterOperation(const Operation::Index&);

  // Unregister a tuple of operations identified by their class types.
  template<typename Tuple>
  bool unregisterOperations()
  {
    return Manager::unregisterOperations<0, Tuple>();
  }

  bool registered(const std::string&) const;
  bool registered(const Operation::Index&) const;
  template<typename OperationNtype>
  bool registered() const;

  /// Construct an operation identified by its type name.
  std::shared_ptr<smtk::operation::Operation> create(const std::string&);

  /// Construct an operation identified by its type index.
  std::shared_ptr<smtk::operation::Operation> create(const Operation::Index&);

  /// Construct an operation identified by its class type.
  template<typename OperationType>
  smtk::shared_ptr<OperationType> create();

  /// Return an operation's type index given its type name.
  ///
  /// If the \a typeName is not registered, this will return 0.
  Operation::Index registeredTypeIndex(const std::string& typeName) const;

  // We expose the underlying containers for metadata; this means of access
  // should not be necessary for most use cases.

  /// Return the map of metadata.
  MetadataContainer& metadata() { return m_metadata; }

  /// Return the launchers associated with this manager.
  Launchers& launchers() { return m_launchers; }
  const Launchers& launchers() const { return m_launchers; }

  /// Return the observers associated with this manager.
  Observers& observers() { return m_observers; }
  const Observers& observers() const { return m_observers; }

  /// Return the group observers associated with this manager.
  Group::Observers& groupObservers() { return m_groupObservers; }
  const Group::Observers& groupObservers() const { return m_groupObservers; }

  /// Return the metadata observers associated with this manager.
  Metadata::Observers& metadataObservers() { return m_metadataObservers; }
  const Metadata::Observers& metadataObservers() const { return m_metadataObservers; }

  // Return the managers instance that contains this manager, if it exists.
  smtk::common::Managers::Ptr managers() const { return m_managers.lock(); }
  void setManagers(const smtk::common::Managers::Ptr& managers) { m_managers = managers; }

  /// Assign a resource manager to manage resources created by operations.
  ///
  /// This method constructs an observer that registers created resources to the
  /// resource manager. It also constructs a metadata observer that assigns the
  /// resource manager to all generated instances of operations that inherit from
  /// ResourceManagerOperation.
  bool registerResourceManager(smtk::resource::ManagerPtr&);

  /// Return a set of type names for all operations.
  std::set<std::string> availableOperations() const;

  /// Given a resource component, return a set of indices for operations that can
  /// accept the component as input.
  std::set<Operation::Index> availableOperations(const smtk::resource::ComponentPtr&) const;

  /// Query the registered operator metadata for operator group names.
  std::set<std::string> availableGroups() const;

private:
  Manager();

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
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type registerOperations(
    const std::array<std::string, std::tuple_size<Tuple>::value>& typeNames)
  {
    bool registered =
      this->registerOperation<typename std::tuple_element<I, Tuple>::type>(typeNames.at(I));
    return registered && Manager::registerOperations<I + 1, Tuple>(typeNames);
  }

  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type registerOperations(
    const std::array<std::string, std::tuple_size<Tuple>::value>&)
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

  /// A container for all operation launchers.
  Launchers m_launchers;

  /// A container for all operation observers.
  Observers m_observers;

  /// A container for all operation group observers.
  Group::Observers m_groupObservers;

  /// A container for all operation metadata observers.
  Metadata::Observers m_metadataObservers;

  /// Observer index for resource manager.
  Observers::Key m_resourceObserver;

  /// A container for all registered operation metadata.
  MetadataContainer m_metadata;

  /// A weak pointer to the managers instance that contains this manager, if it
  /// exists.
  std::weak_ptr<smtk::common::Managers> m_managers;
};

template<typename OperationType>
bool Manager::unregisterOperation()
{
  return this->unregisterOperation(std::type_index(typeid(OperationType)).hash_code());
}

template<typename OperationType>
smtk::shared_ptr<OperationType> Manager::create()
{
  return smtk::static_pointer_cast<OperationType>(
    this->create(std::type_index(typeid(OperationType)).hash_code()));
}

template<typename OperationType>
bool Manager::registerOperation()
{
  return Manager::registerOperation<OperationType>(smtk::common::typeName<OperationType>());
}

template<typename OperationType>
bool Manager::registerOperation(const std::string& typeName)
{
  // For standard operations (i.e. those defined in C++), the pattern is to use
  // the hash of the type_index as its index, the specification defined by
  // OperationType::createSpecification() and the creation method
  // OperationType::create(). This method is simply a shorthand that constructs a
  // metadata instance that adheres to this convention.

  return Manager::registerOperation(Metadata(
    typeName,
    std::type_index(typeid(OperationType)).hash_code(),
    std::dynamic_pointer_cast<Operation>(OperationType::create())->createSpecification(),
    []() { return OperationType::create(); }));
}

template<typename OperationType>
bool Manager::registered() const
{
  return Manager::registered(std::type_index(typeid(OperationType)).hash_code());
}
} // namespace operation
} // namespace smtk

#endif // smtk_operation_Manager_h
