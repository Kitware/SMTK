//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/Status.h"

namespace smtk
{
namespace job
{

std::string statusAsString(Status status)
{
  switch (status)
  {
    case Status::Pending:
      return "Pending";
    case Status::Succeeded:
      return "Succeeded";
    case Status::Failed:
      return "Failed";
    case Status::Terminated:
      return "Terminated";
    default:
      break;
  }
  return "unknown";
}

} // namespace job
} // namespace smtk
