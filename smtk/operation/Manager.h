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
#include "smtk/operation/NewOp.h"
#include "smtk/operation/Observer.h"

#include <string>
#include <typeinfo>

namespace smtk
{
namespace operation
{
/// An operation Manager is responsible for creating new operators and
/// filtering operators based on input type. Operator types must first be
/// registered with the Manager before operators of this type can be manipulated
/// by the manager.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypeMacroBase(Manager);
  smtkCreateMacro(Manager);

  virtual ~Manager();

  /// Register a resource identified by its class type and unique name.
  template <typename ResourceType>
  bool registerOperator(const std::string&);

  /// Register an operator identified by its type index.
  bool registerOperator(Metadata&&);

  /// Construct an operator identified by its unique name.
  std::shared_ptr<smtk::operation::NewOp> create(const std::string&);

  /// Construct an operator identified by its type index.
  std::shared_ptr<smtk::operation::NewOp> create(const NewOp::Index&);

  /// Construct an operator identified by its class type.
  template <typename OperatorType>
  smtk::shared_ptr<OperatorType> create();

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
  /// resource manager to all generated instances of operators that inherit from
  /// ResourceManagerOperator.
  bool registerResourceManager(smtk::resource::ManagerPtr&);

  /// Given a resource component, return a set of indices for operators that can
  /// accept the component as input.
  std::set<NewOp::Index> availableOperators(const smtk::resource::ComponentPtr&) const;

private:
  Manager();

  /// A container for all operator observers.
  Observers m_observers;

  /// A container for all operator metadata observers.
  Metadata::Observers m_metadataObservers;

  /// Observer index for resource manager.
  Observers::Key m_resourceObserver;

  /// Metadata Observer index for resource manager.
  Metadata::Observers::Key m_resourceMetadataObserver;

  /// A container for all registered operator metadata.
  MetadataContainer m_metadata;
};

template <typename OperatorType>
smtk::shared_ptr<OperatorType> Manager::create()
{
  return smtk::static_pointer_cast<OperatorType>(
    this->create(std::type_index(typeid(OperatorType)).hash_code()));
}

template <typename OperatorType>
bool Manager::registerOperator(const std::string& uniqueName)
{
  // For standard operators (i.e. those defined in C++), the pattern is to use
  // the hash of the type_index as its index, the specification defined by
  // OperatorType::createSpecification() and the creation method
  // OperatorType::create(). This method is simply a shorthand that constructs a
  // metadata instance that adheres to this convention.

  return Manager::registerOperator(
    Metadata(uniqueName, std::type_index(typeid(OperatorType)).hash_code(),
      std::dynamic_pointer_cast<NewOp>(OperatorType::create())->createSpecification(),
      []() { return OperatorType::create(); }));
}
}
}

#endif // smtk_operation_Manager_h
