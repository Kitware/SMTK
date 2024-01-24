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

#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{

/// This object provides applications a way to configure a task using
/// information adapted from its dependencies.
class SMTKCORE_EXPORT Adaptor : public smtk::resource::Component
{
public:
  smtkTypeMacro(smtk::task::Adaptor);
  smtkSuperclassMacro(smtk::resource::Component);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  /// Task adaptors are configured using JSON.
  using Configuration = nlohmann::json;

  /// Construct an unconfigured adaptor.
  Adaptor();
  Adaptor(const Configuration& config);
  Adaptor(const Configuration& config, Task* from, Task* to);

  /// Destructor must be virtual.
  ~Adaptor() override = default;

  const smtk::resource::ResourcePtr resource() const override;
  std::string name() const override;
  const common::UUID& id() const override { return m_id; }
  // TODO: Once task::Manager indexes adaptors by UUID, this will need to change.
  bool setId(const common::UUID& uid) override;

  /// Subclasses must override this method and respond to changes in
  /// the state of the upstream task as provided.
  ///
  /// Note this method replaces `reconfigureTask()` method which has been removed.
  virtual bool updateDownstreamTask(State upstreamPrev, State upstreamNext) = 0;

  /// The task this adaptor uses to fetch configuration parameters.
  Task* from() const { return m_from; }
  /// The task to which this adaptor applies configuration parameters.
  Task* to() const { return m_to; }

protected:
  void configureId(const Configuration& config);

  smtk::common::UUID m_id;
  Task* m_from{ nullptr };
  Task* m_to{ nullptr };
  smtk::task::Task::Observers::Key m_observer;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_Adaptor_h
