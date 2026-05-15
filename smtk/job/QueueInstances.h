//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_QueueInstances_h
#define smtk_job_QueueInstances_h

#include "smtk/job/Queue.h"

#include "smtk/common/Instances.h"

namespace smtk
{
namespace job
{
class Manager;

/// A class that manages instances of job::Queue objects.
class SMTKCORE_EXPORT QueueInstances : public smtk::common::Instances<smtk::job::Queue>
{
public:
  smtkTypeMacro(smtk::job::QueueInstances);
  smtkSuperclassMacro(smtk::common::Instances<smtk::job::Queue>);
  smtkCreateMacro(smtk::job::QueueInstances);

  QueueInstances() = default;
  QueueInstances(smtk::job::Manager* jobManager);
  QueueInstances(QueueInstances&&) = default;
  virtual ~QueueInstances() = default;

  /// Manage a job queue.
  ///
  /// We override manage() so we can set the job-manager on each queue.
  /// This allows each queue to fetch job definitions (needed when
  /// creating/restoring jobs from persistent storage).
  bool manage(const std::shared_ptr<Queue>& instance) override;

  Queue* findByName(const std::string& name);

protected:
  smtk::job::Manager* m_manager{ nullptr };
};

} // namespace job
} // namespace smtk

#endif // smtk_job_QueueInstances_h
