//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_OperationFactory_h
#define smtk_project_OperationFactory_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/TypeName.h"

#include "smtk/operation/Operation.h"

namespace smtk
{
namespace project
{

/// An OperationFactory is a factory for a Project's Operations. It holds a
/// whitelist of Operations that are relevant to a given Project.
class SMTKCORE_EXPORT OperationFactory
{
public:
  OperationFactory(const std::weak_ptr<smtk::operation::Manager>& manager)
    : m_manager(manager)
  {
  }

  /// Register an operation type according to its typename, type index or class
  /// type.
  bool registerOperation(const std::string&);
  bool registerOperation(const smtk::operation::Operation::Index&);
  template<typename OperationType>
  bool registerOperation();

  /// Register a set of operation types according to their typenames.
  bool registerOperations(const std::set<std::string>&);

  /// Unregister an operation type according to its typename, type index or class
  /// type.
  bool unregisterOperation(const std::string&);
  bool unregisterOperation(const smtk::operation::Operation::Index&);
  template<typename OperationType>
  bool unregisterOperation();

  /// Construct an operation identified by its typename,type index or class type.
  std::shared_ptr<smtk::operation::Operation> create(const std::string&);
  std::shared_ptr<smtk::operation::Operation> create(const smtk::operation::Operation::Index&);
  template<typename OperationType>
  smtk::shared_ptr<OperationType> create();

  /// Return a whitelist of type names for all available operations. If the list
  /// is empty, all operations registered to the associated operation manager are
  /// available.
  const std::set<std::string>& types() const { return m_types; }
  std::set<std::string>& types() { return m_types; }

  /// Given a resource component, return a set of indices for operations that can
  /// accept the component as input.
  std::set<smtk::operation::Operation::Index> availableOperations(
    const smtk::resource::ComponentPtr&) const;

  std::shared_ptr<smtk::operation::Manager> manager() const { return m_manager.lock(); }

  void setManager(const std::weak_ptr<smtk::operation::Manager>& manager) { m_manager = manager; }

private:
  std::set<std::string> m_types;
  std::weak_ptr<smtk::operation::Manager> m_manager;
};

template<typename OperationType>
bool OperationFactory::registerOperation()
{
  return this->registerOperation(smtk::common::typeName<OperationType>());
}

template<typename OperationType>
bool OperationFactory::unregisterOperation()
{
  return this->unregisterOperation(smtk::common::typeName<OperationType>());
}

template<typename OperationType>
smtk::shared_ptr<OperationType> OperationFactory::create()
{
  return smtk::static_pointer_cast<OperationType>(
    this->create(std::type_index(typeid(OperationType)).hash_code()));
}
} // namespace project
} // namespace smtk

#endif
