//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_job_operators_AddJobToQueue_h
#define smtk_job_operators_AddJobToQueue_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace job
{

/**\brief An operation to add a job (jobs are components) to a queue (queues are resources).
  *
  * The associated job is presumed not to belong to the queue which it claims as its parent.
  * (If it is, this operation is a no-op.)
  *
  * If the job is marked to be automatically scheduled, it will be scheduled by this operation.
  * Otherwise, the job will be added but not scheduled.
  */
class SMTKCORE_EXPORT AddJobToQueue : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::job::AddJobToQueue);
  smtkCreateMacro(AddJobToQueue);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  void generateSummary(Operation::Result&) override;
  const char* xmlDescription() const override;
};
} // namespace job
} // namespace smtk

#endif // smtk_job_operators_AddJobToQueue_h
