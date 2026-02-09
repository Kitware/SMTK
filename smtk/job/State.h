//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_State_h
#define smtk_job_State_h

namespace smtk
{
namespace job
{

/// Possible states for a job in a queueing system.
///
/// The set of transitions that are possible include:
/// + Unscheduled→Scheduled
/// + Scheduled→{Running, Canceled}
/// + Running→{Canceled,Completed}
/// + Canceled→{Unscheduled, Scheduled}
/// + Completed→{Unscheduled, Scheduled}
enum State
{
  Unscheduled, //!< The job is not in a queue.
  Scheduled,   //!< The job is in a queue but has not been assigned resources.
  Running,     //!< The job in the queue and has been assigned resources.
  Canceled,    //!< The job was removed from the queue (possibly while running).
  Completed,   //!< The job exited the queue after completion (with a status of success or failure).
};

} // namespace job
} // namespace smtk

#endif // smtk_job_State_h
