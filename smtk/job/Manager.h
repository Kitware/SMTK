//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_Manager_h
#define smtk_job_Manager_h

#include "smtk/common/Active.h"
#include "smtk/common/Instances.h"
#include "smtk/job/DefinitionInstances.h"
#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/QueueInstances.h"

namespace smtk
{
namespace job
{

using ActiveQueue = smtk::common::Active<QueueInstances>;

/// A job manager that tracks available queues for scheduling jobs.
///
/// Eventually, log parsers will also be registered to this manager.
///
/// Site-specific Registrars may add queues for HPC centers only for users with access.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypeMacroBase(smtk::job::Manager);
  smtkCreateMacro(smtk::job::Manager);

  Manager();
  Manager(Manager&&) = delete;
  virtual ~Manager() = default;

  /// Return the set of managed queue objects.
  const QueueInstances& queues() const;
  QueueInstances& queues();

  /// Return the set of managed job definitions.
  const DefinitionInstances& jobTypes() const;
  DefinitionInstances& jobTypes();

  /// Return the object which tracks the active queue (from queues()).
  ///
  /// Only one queue may be active at a time (making it the default
  /// queue for scheduling jobs).
  smtk::common::Active<QueueInstances>& activeQueue();

  /// Returns the currently active queue (rather than an object
  /// which tracks changes to the currently active queue).
  Queue* defaultQueue() const;

private:
  QueueInstances m_queues;
  DefinitionInstances m_definitions;
  ActiveQueue m_activeQueue;
};

} // namespace job
} // namespace smtk

#endif // smtk_job_Manager_h
