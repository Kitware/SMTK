//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_Status_h
#define smtk_job_Status_h

#include "smtk/CoreExports.h"

#include <string>

namespace smtk
{
namespace job
{

/// Possible statuses for a job.
enum Status
{
  Pending,   //!< The job has not run to completion since creation or re-queueing.
  Succeeded, //!< The job script ran without error.
  Failed,    //!< The job script was run but produced an error.
  Terminated, //!< The job was run but terminated before completion. Not all queueing systems provide this.
};

SMTKCORE_EXPORT std::string statusAsString(Status status);

} // namespace job
} // namespace smtk

#endif // smtk_job_Status_h
