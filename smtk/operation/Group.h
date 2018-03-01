//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_Group_h
#define smtk_operation_Group_h

#include "smtk/CoreExports.h"

#include "smtk/operation/Operation.h"

#include <set>
#include <string>

namespace smtk
{
namespace operation
{
class Manager;

/// This class describes a stateless object associated with an operation manager
/// that provides a unified API for grouping, ungrouping and querying operation
/// groups. The persistent data associated with operation groups is held in the
/// operation's specification, and is accessed only in the context of an
/// operation's association with a manager (i.e. operations that are not
/// registered with a manager do not have the concept of groups).
///
/// The design isolates the notion of operator grouping to the application scope
/// of SMTK; operations are designed to be added to groups when they are
/// registered to a manager, and an application subsequently performs operation
/// group queries through the manager's interface.
///
/// A group is simply a string value used to associate collections of operations
/// for aggregate manipulation (e.g. read operations that need to be associated
/// with the File->Open menu command). To prevent the misuse of groups in an
/// ad-hoc fashion for attaching custom data to operations, it is highly
/// recommended that group names be declared at compile-time and referenced only
/// through their field, and not by value. In the future, this API may use a
/// strong typedef pattern for group names to further enforce this convention.
class SMTKCORE_EXPORT Group
{
  friend class Manager;

public:
  const std::string& name() const { return m_name; }

  /// Register an operation identified by it's unique name and give it values.
  bool registerOperation(
    const std::string&, std::set<std::string> values = std::set<std::string>());

  /// Register an operation identified by its type index and give it values.
  bool registerOperation(
    const Operation::Index&, std::set<std::string> values = std::set<std::string>());

  /// Register an operation identified by its class type and give it values.
  template <typename OperationType>
  bool registerOperation(std::set<std::string> values = std::set<std::string>());

  /// Unregister an operation.
  bool unregisterOperation(const std::string&);

  /// Unregister an operation identified by its type index.
  bool unregisterOperation(const Operation::Index&);

  /// Unregister an operation identified by its class type.
  template <typename OperationType>
  bool unregisterOperation();

  /// Check if an operation identified by it's unique name is in the group.
  bool has(const std::string&) const;

  /// Check if an operation identified by its type index is in the group.
  bool has(const Operation::Index&) const;

  /// Check if an operation identified by its class type is in the group.
  template <typename OperationType>
  bool has() const;

  /// Obtain values for an operation identified by it's unique name is in the
  /// group.
  std::set<std::string> values(const std::string&) const;

  /// Obtain values for an operation identified by its type index is in the
  /// group.
  std::set<std::string> values(const Operation::Index&) const;

  /// Obtain values for an operation identified by its class type is in the
  /// group.
  template <typename OperationType>
  std::set<std::string> values() const;

  /// Return a set of operation indices that belong to this group.
  std::set<Operation::Index> operations() const;

  /// Return a set of operation unique names that belong to this group.
  std::set<std::string> operationNames() const;

private:
  Group(const std::string& name, std::shared_ptr<smtk::operation::Manager> manager)
    : m_name(name)
    , m_manager(manager)
  {
  }

  std::string m_name;

  std::weak_ptr<smtk::operation::Manager> m_manager;
};

template <typename OperationType>
bool Group::registerOperation(std::set<std::string> values)
{
  return registerOperation(std::type_index(typeid(OperationType)).hash_code(), values);
}

template <typename OperationType>
bool Group::unregisterOperation()
{
  return unregisterOperation(std::type_index(typeid(OperationType)).hash_code());
}

template <typename OperationType>
bool Group::has() const
{
  return has(std::type_index(typeid(OperationType)).hash_code());
}

template <typename OperationType>
std::set<std::string> Group::values() const
{
  return values(std::type_index(typeid(OperationType)).hash_code());
}
}
}

#endif // smtk_operation_Metadata_h
