//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_job_ArtifactStageTracker_h
#define smtk_extension_paraview_job_ArtifactStageTracker_h

#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"

#include <algorithm>
#include <map>
#include <utility>

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace job
{
namespace detail
{

/// Internal bookkeeping for stage notifications, independent of task-view lifetime.
class ArtifactStageTracker
{
public:
  /// Return the half-open range of newly completed stages for this job run.
  ///
  /// Advancing from stage 1 to stage 3 yields [1, 3), covering both stages
  /// even when polling missed an intermediate notification. An empty range
  /// means no reload is needed. Calling this method consumes the notification;
  /// stages are tracked even when none of their artifacts have loaded sources.
  std::pair<int, int> update(const smtk::job::Job& job)
  {
    auto* definition = job.jobType();
    if (!definition)
    {
      return { 0, 0 };
    }
    int& previous = m_completed[job.id()];
    if (
      job.status() == smtk::job::Status::Pending &&
      (job.state() == smtk::job::State::Unscheduled || job.state() == smtk::job::State::Scheduled))
    {
      // ScheduleJob can reuse the same UUID; its modified-job notification
      // resets this watermark even if the next progress update skips stages.
      previous = 0;
      return { 0, 0 };
    }

    const int count = static_cast<int>(definition->stages().size());
    // While running, stage() identifies the current stage: stages with smaller
    // indices have finished. Negative values indicate that execution has not begun.
    int completed = job.stage();
    if (job.status() == smtk::job::Status::Succeeded && job.state() == smtk::job::State::Completed)
    {
      // Process completion can arrive before the final progress-file update.
      completed = count;
    }
    else if (job.status() == smtk::job::Status::Failed && completed > 0)
    {
      // Progress files count the failed stage too. Only preceding stages
      // have successfully produced their artifacts.
      --completed;
    }
    completed = std::clamp(completed, 0, count);
    const int first = previous;
    // Keep the count monotonic within a run. Repeated or older progress values
    // must not cause an earlier stage's artifacts to be reloaded again.
    previous = std::max(previous, completed);
    return { first, previous };
  }

  /// Forget an expunged job; completed jobs remain tracked to suppress duplicate updates.
  void erase(const smtk::common::UUID& id) { m_completed.erase(id); }

private:
  std::map<smtk::common::UUID, int> m_completed;
};

} // namespace detail
} // namespace job
} // namespace paraview
} // namespace extension
} // namespace smtk

#endif
