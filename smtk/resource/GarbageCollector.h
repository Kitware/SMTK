//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_GarbageCollector_h
#define smtk_resource_GarbageCollector_h

#include "smtk/operation/Observer.h"

#include "smtk/SystemConfig.h"

#include <map>
#include <memory>
#include <string>

namespace smtk
{

namespace resource
{

/**\brief A class that owns ephemeral objects and remove them when abandoned.
  *
  * This class will queue an operation (which you provide) to delete the
  * ephemeral objects associated with the operation when their shared-pointer
  * use-count drops to 1 (indicating they are owned solely by the resource
  * managing them).
  *
  * Note that your deletion operation must (a) accept associations, (b) prevent
  * its associations from holding references (i.e., "HoldReference"="false"),
  * and (c) require at least 1 associated persistent object.
  *
  * This class works by adding an observer to the registered operation manager
  * and iterating over its set of delete-operations' associations when any operation
  * completes successfully.
  *
  * Note that an operation, A, passed to the garbage collector is not guaranteed
  * to run; some other operation, B, may delete the associated objects first.
  * In that case, when B completes successfully, the garbage collector will
  * run, see that A has no valid associated objects (since they are weak references
  * to the deleted object), and remove operation A from its list of collectors.
  * It is also possible A will never be invoked because the program exits
  * while holding references.
  */
class SMTKCORE_EXPORT GarbageCollector : smtkEnableSharedPtr(GarbageCollector)
{
public:
  smtkTypeMacroBase(smtk::resource::GarbageCollector);
  smtkCreateMacro(smtk::resource::GarbageCollector);
  virtual ~GarbageCollector();

  GarbageCollector(const GarbageCollector&) = delete;
  GarbageCollector& operator=(const GarbageCollector&) = delete;

  using WeakManagerPtr = smtk::operation::WeakManagerPtr;
  using Key = smtk::operation::Observers::Key;
  using ObserverMap = std::map<WeakManagerPtr, Key, std::owner_less<WeakManagerPtr>>;

  /// Queue an operation to clean up an ephemeral resource.
  bool add(const smtk::operation::OperationPtr& deleter);

protected:
  enum Status
  {
    Invalid = 0,
    NotReady = 1,
    Ready = 2
  };

  GarbageCollector();

  int collectGarbage(
    const smtk::operation::Operation&,
    smtk::operation::EventType,
    smtk::operation::Operation::Result);

  /// Returns true if the operation is ready to be evaluated, false otherwise.
  static Status checkOperation(const smtk::operation::Operation* op);

  /// The set of operations used to collect garbage
  /// (and whose associations indicate which objects are garbage).
  std::set<smtk::operation::OperationPtr> m_garbage;

  /// The "ids" of observers calling collectGarbage after each operation.
  ObserverMap m_observers;
};
} // namespace resource
} // namespace smtk

#endif // smtk_resource_GarbageCollector_h
