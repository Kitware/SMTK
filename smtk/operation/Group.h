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

#include "smtk/operation/GroupObserver.h"
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
/// with the File->Open menu command). The API presented here is extremely
/// flexible, allowing for any number (including 0) of strings to be associated
/// to an operator within the context of the group. To prevent the misuse of
/// groups in an ad-hoc fashion for attaching custom data to operations, it is
/// highly recommended that this class be subclassed privately, and a narrowing
/// API for that group's context be used on top of this one. See
/// smtk::operation::ReaderGroup for an example of this use case.
class SMTKCORE_EXPORT Group
{
public:
  typedef GroupObserver Observer;
  typedef GroupObservers Observers;

  Group(const std::string& name, std::shared_ptr<smtk::operation::Manager> manager)
    : m_manager(manager)
    , m_name(name)
  {
  }

  const std::string& name() const { return m_name; };

  /// Register an operation identified by its unique name and give it values.
  bool registerOperation(
    const std::string&,
    std::set<std::string> values = std::set<std::string>());

  /// Register an operation identified by its type index and give it values.
  bool registerOperation(
    const Operation::Index&,
    std::set<std::string> values = std::set<std::string>());

  /// Register an operation identified by its class type and give it values.
  template<typename OperationType>
  bool registerOperation(std::set<std::string> values = std::set<std::string>());

  /// Unregister an operation identified by its unique name.
  bool unregisterOperation(const std::string&);

  /// Unregister an operation identified by its type index.
  bool unregisterOperation(const Operation::Index&);

  /// Unregister an operation identified by its class type.
  template<typename OperationType>
  bool unregisterOperation();

  /// Access the specification of an operation identified by its unique name.
  Operation::Specification specification(const std::string&) const;

  /// Access the specification of an operation identified by its type index.
  Operation::Specification specification(const Operation::Index&) const;

  /// Access the specification of an operation identified by its class type.
  template<typename OperationType>
  Operation::Specification specification() const;

  /// Check if an operation identified by it's unique name is in the group.
  bool contains(const std::string&) const;

  /// Check if an operation identified by its type index is in the group.
  bool contains(const Operation::Index&) const;

  /// Check if an operation identified by its class type is in the group.
  template<typename OperationType>
  bool contains() const;

  /// Obtain values for an operation identified by it's unique name is in the
  /// group.
  std::set<std::string> values(const std::string&) const;

  /// Obtain values for an operation identified by its type index is in the
  /// group.
  std::set<std::string> values(const Operation::Index&) const;

  /// Obtain values for an operation identified by its class type is in the
  /// group.
  template<typename OperationType>
  std::set<std::string> values() const;

  /// Return a set of operation indices that belong to this group.
  std::set<Operation::Index> operations() const;

  /// Return a set of operation unique names that belong to this group.
  std::set<std::string> operationNames() const;

  /// Given an operation index, return the operation's name.
  std::string operationName(const Operation::Index&) const;

  /// Given an operation index, return the operation's label.
  std::string operationLabel(const Operation::Index&) const;

protected:
  std::weak_ptr<smtk::operation::Manager> m_manager;

  /// Return a "degree of fit" measure indicating how well an operation matches an object.
  ///
  /// This method is intended for use by subclasses that need to choose a single operation
  /// among multiple allowable operations in the group.
  ///
  /// This method examines each operation in the group.
  /// If the operation's associationRule allows the object \a obj to be associated with
  /// the operation, then an integer is returned that indicates how well the object matches
  /// the operation's list of acceptable associations. Smaller numbers indicate better fits.
  /// If association is disallowed, the returned value is std::numeric_limits<std::size_t>::max().
  ///
  /// The "degree of fit" returned is the smallest number of hops along the inheritance diagram
  /// of obj (or the resource owning \a obj if it is a component) to reach any entry in the
  /// operation's "accepts" rules that allows association.
  ///
  /// \sa smtk::operation::DeleterGroup
  std::size_t operationObjectDistance(
    const Operation::Index& index,
    const smtk::resource::PersistentObject& obj) const;

  /// A utility to fetch association rules for a given operation (used in operationObjectDistance).
  smtk::attribute::ConstReferenceItemDefinitionPtr operationAssociationsRule(
    const Operation::Index& index) const;

  /// A utility to fetch reference-item rules for a given operation (used by subclasses such as ArcCreator).
  smtk::attribute::ConstReferenceItemDefinitionPtr operationReferenceItemRule(
    const Operation::Index& index,
    const std::string& itemName) const;

private:
  std::string m_name;
};

template<typename OperationType>
bool Group::registerOperation(std::set<std::string> values)
{
  return registerOperation(std::type_index(typeid(OperationType)).hash_code(), values);
}

template<typename OperationType>
bool Group::unregisterOperation()
{
  return unregisterOperation(std::type_index(typeid(OperationType)).hash_code());
}

template<typename OperationType>
Operation::Specification Group::specification() const
{
  return specification(std::type_index(typeid(OperationType)).hash_code());
}

template<typename OperationType>
bool Group::contains() const
{
  return contains(std::type_index(typeid(OperationType)).hash_code());
}

template<typename OperationType>
std::set<std::string> Group::values() const
{
  return values(std::type_index(typeid(OperationType)).hash_code());
}
} // namespace operation
} // namespace smtk

#endif // smtk_operation_Metadata_h
