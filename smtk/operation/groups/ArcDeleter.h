//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_ArcDeleter_h
#define smtk_operation_ArcDeleter_h

#include "smtk/CoreExports.h"

#include "smtk/common/TypeName.h"

#include "smtk/operation/Group.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ReferenceItemDefinition.h"

#include <string>
#include <unordered_set>

namespace smtk
{
namespace operation
{
class Manager;

/**\brief A group holding operations which can disconnect a pair of Component instances.
  *
  * All operations added to this group must
  * + have a discrete string item named "arc type";
  * + have a single *active* group item child of "arc type" whose name _ends with_ "endpoints";
  * + have two children of the "endpoints" group whose names are:
  *   + "from" – a ReferenceItem holding the predecessor component the arc
  *   + "to" – a ReferenceItem holding the successor component of the arc
  * The active "endpoints" group must require 0 values but be extensible (so that many arcs of
  * the same type may be deleted at once).
  *
  * Because it is possible to have many GroupItem children of "arc type" that are
  * active for different values of "arc type," a single deletion operation can support
  * arcs that require a variety of different endpoints to be removed.
  *
  * Note that there is no way for operations in this group to handle
  * 1. multiple arcs of the same type between the same pair of nodes (i.e., multi-arcs).
  *    If this is problematic, this operation group will need to be modified to model multi-arcs.
  * 2. multiple arcs of differing type within the same run of the same operation instance.
  *    (This is because the string-item can only take on a single discrete value at a time.)
  *
  * Note that operations should implement ableToOperate so it guarantees the operation
  * will succeed if it is allowed to run. User interfaces exposing operations in this
  * group should verify that all operations to remove the user-specified arcs are
  * able to operate before launching any of the operations. This does **not**
  * guarantee success (e.g., arc deletion could be contingent on the existence of other
  * arcs scheduled for deletion by a separate operation), but handles the most common
  * case.
  */
class SMTKCORE_EXPORT ArcDeleter : protected Group
{
public:
  static constexpr const char* const type_name = "arcDeleter";

  ArcDeleter(std::shared_ptr<smtk::operation::Manager> manager)
    : Group(type_name, manager)
  {
  }

  /// Register an operation.
  ///
  /// The operation's parameters will be examined to determine the types of arcs
  /// that can be deleted.
  template<typename OperationType>
  bool registerOperation();
  bool registerOperation(Operation::Index operationIndex);

  using Group::unregisterOperation;

  /// Return the set of all arc types which have operations that can perform deletion.
  std::set<std::string> allArcTypes() const;

  /// Return the set of all arc type + operation index combinations that support deletion.
  std::set<std::pair<std::string, Operation::Index>> allArcDeleters() const;

  /// Given an ordered pair of objects and an arc type, return operations
  /// that can delete such an arc.
  std::unordered_set<Operation::Index> matchingOperations(
    smtk::string::Token arcType,
    const smtk::resource::PersistentObject& from,
    const smtk::resource::PersistentObject& to) const;

  /// Given an operation (presumably from matchingOperations(…) above),
  /// configure it to delete the given arc.
  ///
  /// This returns true if \a op was successfully modified to include
  /// the arc defined by \a arcType, \a from, and \a to; and false otherwise.
  ///
  /// If \a op's parameters have "arc type" set to a different value
  /// (i.e., \a arcType is "foo" but `op->findString("arc type")->value()`
  /// returns "bar"), this method will return false; an operation should
  /// only delete arcs of a single type at a time.
  static bool appendArc(
    smtk::operation::Operation& op,
    smtk::string::Token arcType,
    const std::shared_ptr<smtk::resource::PersistentObject>& from,
    const std::shared_ptr<smtk::resource::PersistentObject>& to);

protected:
  /// Return all the arc types the operation with the given \a parameterDefinition supports.
  ///
  /// This will return false and \a arcTypes will be unchanged if \a parameterDefinition
  /// is not of the expected form.
  ///
  /// This will return true if at least one arc type is supported by the operation
  /// (whether the arc type was already present in \a arcTypes or was inserted by
  /// this method).
  bool introspectArcTypes(
    Operation::Definition parameterDefinition,
    std::set<std::string>& arcTypes);

  /// Given the definition for an operation's parameters and an arc type,
  /// return item-definition pointers for the "from" and "to" reference
  /// items which hold endpoints.
  ///
  /// This is used to determine whether a pair of objects can have
  /// an \a arcType arc removed from between them.
  std::pair<
    smtk::attribute::ConstReferenceItemDefinitionPtr,
    smtk::attribute::ConstReferenceItemDefinitionPtr>
  endpointItemDefs(Operation::Definition parameterDefinition, smtk::string::Token arcType) const;
};

template<typename OperationType>
bool ArcDeleter::registerOperation()
{
  Operation::Index index = std::type_index(typeid(OperationType)).hash_code();
  return this->registerOperation(index);
}

} // namespace operation
} // namespace smtk

#endif // smtk_operation_ArcDeleter_h
