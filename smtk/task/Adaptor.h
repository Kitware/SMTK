//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_Adaptor_h
#define smtk_task_Adaptor_h

#include "smtk/common/Deprecation.h"
#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{

/// This object provides applications a way to configure a task using
/// information adapted from its dependencies.
class SMTKCORE_EXPORT Adaptor : smtkEnableSharedPtr(Adaptor)
{
public:
  smtkTypeMacroBase(smtk::task::Adaptor);

  /// Task adaptors are configured using JSON.
  using Configuration = nlohmann::json;

  /// Construct an unconfigured adaptor.
  Adaptor();
  Adaptor(const Configuration& config);
  Adaptor(const Configuration& config, Task* from, Task* to);

  /// Destructor must be virtual.
  virtual ~Adaptor() = default;

  SMTK_DEPRECATED_IN_23_08("Use updateDownstreamTask() instead.")
  virtual bool reconfigureTask();

  /// Subclasses must override this method and respond to changes in
  /// the state of the upstream task as provided.
  ///
  /// Note that the default implementation is provided only for
  /// backward compatibility and will be removed when the deprecated
  /// `reconfigureTask()` method is removed.
  virtual bool updateDownstreamTask(State upstreamPrev, State upstreamNext);

  /// The task this adaptor uses to fetch configuration parameters.
  Task* from() const { return m_from; }
  /// The task to which this adaptor applies configuration parameters.
  Task* to() const { return m_to; }

protected:
  Task* m_from;
  Task* m_to;
  smtk::task::Task::Observers::Key m_observer;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_Adaptor_h
