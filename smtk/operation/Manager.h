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

#include "smtk/operation/Metadata.h"
#include "smtk/operation/MetadataContainer.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"

#include <string>
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
  smtkTypeMacroBase(Manager);
  smtkCreateMacro(Manager);

  virtual ~Manager();

  /// Register a resource identified by its class type and unique name.
  template <typename ResourceType>
  bool registerOperation(const std::string&);

  /// Register an operation identified by its type index.
  bool registerOperation(Metadata&&);

  /// Construct an operation identified by its unique name.
  std::shared_ptr<smtk::operation::Operation> create(const std::string&);

  /// Construct an operation identified by its type index.
  std::shared_ptr<smtk::operation::Operation> create(const Operation::Index&);

  /// Construct an operation identified by its class type.
  template <typename OperationType>
  smtk::shared_ptr<OperationType> create();

  // We expose the underlying containers for metadata; this means of access
  // should not be necessary for most use cases.

  /// Return the map of metadata.
  MetadataContainer& metadata() { return m_metadata; }

  /// Return the observers associated with this manager.
  Observers& observers() { return m_observers; }
  const Observers& observers() const { return m_observers; }

  /// Return the metadata observers associated with this manager.
  Metadata::Observers& metadataObservers() { return m_metadataObservers; }
  const Metadata::Observers& metadataObservers() const { return m_metadataObservers; }

  /// Assign a resource manager to manage resources created by operations.
  ///
  /// This method constructs an observer that registers created resources to the
  /// resource manager. It also constructs a metadata observer that assigns the
  /// resource manager to all generated instances of operations that inherit from
  /// ResourceManagerOperation.
  bool registerResourceManager(smtk::resource::ManagerPtr&);

  /// Given a resource component, return a set of indices for operations that can
  /// accept the component as input.
  std::set<Operation::Index> availableOperations(const smtk::resource::ComponentPtr&) const;

private:
  Manager();

  /// A container for all operation observers.
  Observers m_observers;

  /// A container for all operation metadata observers.
  Metadata::Observers m_metadataObservers;

  /// Observer index for resource manager.
  Observers::Key m_resourceObserver;

  /// Metadata Observer index for resource manager.
  Metadata::Observers::Key m_resourceMetadataObserver;

  /// A container for all registered operation metadata.
  MetadataContainer m_metadata;
};

template <typename OperationType>
smtk::shared_ptr<OperationType> Manager::create()
{
  return smtk::static_pointer_cast<OperationType>(
    this->create(std::type_index(typeid(OperationType)).hash_code()));
}

template <typename OperationType>
bool Manager::registerOperation(const std::string& uniqueName)
{
  // For standard operations (i.e. those defined in C++), the pattern is to use
  // the hash of the type_index as its index, the specification defined by
  // OperationType::createSpecification() and the creation method
  // OperationType::create(). This method is simply a shorthand that constructs a
  // metadata instance that adheres to this convention.

  return Manager::registerOperation(
    Metadata(uniqueName, std::type_index(typeid(OperationType)).hash_code(),
      std::dynamic_pointer_cast<Operation>(OperationType::create())->createSpecification(),
      []() { return OperationType::create(); }));
}
}
}

#endif // smtk_operation_Manager_h
