//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_job_operators_JobUpdated_h
#define smtk_job_operators_JobUpdated_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace job
{

/**\brief An operation to update a job's temporal variables that indicate progress.
  *
  * As a job makes its way through a queue (becoming scheduled, running, completion/cancellation),
  * queues may monitor their jobs. This operation allows queues to launch operations to update
  * the jobs so user-interface elements can reflect this information.
  *
  * The state of the job (relative to the queue), its status (relative to job success/failure),
  * and the stage (progress) are all modeled and may be updated by this operation.
  *
  * Do not use this operation to add, schedule, or cancel a job; there are operations
  * specifically for those. This operation should only be used to update the job stage/state/status
  * while the job is running.
  * Because of this, some state/status transitions will be rejected (without operation failure)
  * if they cause a job to "regress" (move backward in the list of allowable transitions).
  * This can happen if a job is terminated but a "logs/progress" file modification causes
  * an out-of-sequence launch of this operation. In that case, the stage will be updated but
  * not the state or status.
  */
class SMTKCORE_EXPORT JobUpdated : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::job::JobUpdated);
  smtkCreateMacro(JobUpdated);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  void generateSummary(Operation::Result&) override;
  const char* xmlDescription() const override;
};
} // namespace job
} // namespace smtk

#endif // smtk_job_operators_JobUpdated_h
