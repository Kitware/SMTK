//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_GroupOps_h
#define smtk_operation_GroupOps_h

#include "smtk/operation/groups/DeleterGroup.h"

namespace smtk
{
namespace operation
{

/// A structure that holds operations configured via association.
///
/// This structure is passed to a DispositionFunction that may
/// programmatically modify operation parameters or ask users for input.
struct SMTKCORE_EXPORT DispositionBatch
{
  std::unordered_map<smtk::operation::Operation::Index, smtk::operation::Operation::Ptr> ops;
  std::set<smtk::resource::PersistentObject::Ptr> noMatch;
  std::set<smtk::resource::PersistentObject::Ptr> cannotAssociate;
  std::size_t numberOfObjects{ 0 };
  std::size_t numberOfBlockedOperations{ 0 };
  std::size_t numberOfBlockedAssociations{ 0 };

  /// Return true if the batch of \a ops is able to run and all input objects
  /// are associated to an operation in \a ops.
  ///
  /// A DispositionFunction will be invoked only when this method returns false;
  bool ableToOperate() const;
};

/// A function that determines the disposition of a batch of operations
/// that, for some reason, is not able to operate on a set of objects.
///
/// This is used by functions (such as DeleteObjects) to ask users for
/// feedback when operations in a group (such as DeleterGroup) either fails
/// to associate to input objects or is subsequently unable to operate once
/// associated.
///
/// This function may modify the operations it is passed (for example, to
/// enable options to delete dependent objects, etc.).
///
/// If this function returns true, the operation should continue even if some
/// subset of the objects cannot be processed. If this function returns false,
/// then no deletion should be attempted.
using DispositionFunction = std::function<bool(DispositionBatch& batch)>;

/// Given a \a Container of persistent objects and an operation \a GroupType,
/// configure a batch of operations appropriate for the inputs and validate
/// that each operation is able to run before launching any of them.
///
/// Optionally, if some \a objects have no corresponding operation or cannot
/// be associated to their corresponding operation, call the \a dispositionFn
/// to either fix the operations or ask the user whether to proceed.
///
/// This function will return true if any operations were launched and
/// false otherwise (i.e., the \a objects container was empty, had no
/// corresponding operations for any input, or was vetoed by the user).
template<typename GroupType, typename Container>
bool SMTK_ALWAYS_EXPORT processObjectsWithGroup(
  const Container& objects,
  const smtk::operation::Manager::Ptr& operationManager,
  DispositionFunction dispositionFn = [](DispositionBatch&) { return false; })
{
  // Ignore invalid input.
  if (!operationManager || objects.empty())
  {
    return false;
  }

  GroupType opGroup(operationManager);
  DispositionBatch batch;
  batch.numberOfObjects = objects.size();
  for (const auto& object : objects)
  {
    smtk::operation::OperationPtr op;
    auto index = opGroup.matchingOperation(*object);
    if (index)
    {
      auto it = batch.ops.find(index);
      if (it == batch.ops.end())
      {
        op = operationManager->create(index);
        batch.ops[index] = op;
      }
      else
      {
        op = it->second;
      }
    }
    if (!op)
    {
      batch.noMatch.insert(object);
    }
    else if (!op->parameters()->associate(object))
    {
      batch.cannotAssociate.insert(object);
    }
  }

  // Now that all possible objects are associated, see if the operations we've
  // configured are able to operate and collect statistics for the disposition
  // function if not.
  for (const auto& entry : batch.ops)
  {
    if (!entry.second->ableToOperate())
    {
      ++batch.numberOfBlockedOperations;
      batch.numberOfBlockedAssociations +=
        entry.second->parameters()->associations()->numberOfValues();
    }
  }

  if (!batch.ableToOperate() && !dispositionFn(batch))
  {
    // Abort the entire batch without launching any operations.
    return false;
  }

  // Either the operations are properly configured or the disposition
  // function has indicated we should ignore failures and proceed.
  for (const auto& entry : batch.ops)
  {
    operationManager->launchers()(entry.second);
  }

  return true;
}

template<typename Container>
bool SMTK_ALWAYS_EXPORT deleteObjects(
  const Container& objects,
  const Manager::Ptr& operationManager,
  DispositionFunction dispositionFn = [](DispositionBatch&) { return false; })
{
  bool launched = processObjectsWithGroup<DeleterGroup>(objects, operationManager, dispositionFn);
  return launched;
}

} // namespace operation
} // namespace smtk

#endif // smtk_operation_GroupOps_h
