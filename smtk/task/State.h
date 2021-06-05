//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_State_h
#define smtk_task_State_h

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <iostream>
#include <string>

namespace smtk
{
namespace task
{

/// The set of states that a task may take on.
enum class State
{
  Unavailable, //!< The task's dependencies are unmet.
  Incomplete,  //!< The task is available but its objective is not accomplished.
  Completable, //!< The task is available and accomplished but has not been marked complete.
  Completed    //!< The task has been marked completed by the user.
};

/// A type-conversion operation to cast enumerants to strings.
inline std::string stateName(const State& s)
{
  static std::array<std::string, 4> names{
    { "unavailable", "incomplete", "completable", "completed" }
  };
  return names[static_cast<int>(s)];
}

/// A type-conversion operation to cast strings to enumerants.
inline State stateEnum(const std::string& s)
{
  std::string stateName(s);
  std::transform(stateName.begin(), stateName.end(), stateName.begin(), [](unsigned char c) {
    return std::tolower(c);
  });
  if (stateName.substr(0, 7) == "state::")
  {
    stateName = stateName.substr(7);
  }
  if (stateName == "incomplete")
  {
    return State::Incomplete;
  }
  if (stateName == "completable")
  {
    return State::Completable;
  }
  if (stateName == "completed")
  {
    return State::Completed;
  }
  return State::Unavailable;
}

/// States may be appended to streams.
inline std::ostream& operator<<(std::ostream& os, const State& s)
{
  os << stateName(s);
  return os;
}

} // namespace task
} // namespace smtk

#endif // smtk_task_State_h
