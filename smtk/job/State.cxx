//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/State.h"

namespace smtk
{
namespace job
{

std::string stateAsString(State state)
{
  switch (state)
  {
    case State::Unscheduled:
      return "Unscheduled";
    case State::Scheduled:
      return "Scheduled";
    case State::Running:
      return "Running";
    case State::Canceled:
      return "Canceled";
    case State::Completed:
      return "Completed";
    default:
      break;
  }
  return "unknown";
}

} // namespace job
} // namespace smtk
