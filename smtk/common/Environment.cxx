//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/common/Environment.h"

#include <stdlib.h>

namespace smtk
{
namespace common
{

/// Return true if the environment variable exists, false otherwise.
bool Environment::hasVariable(const std::string& varName)
{
  char* buf;
#if !defined(_WIN32) || defined(__CYGWIN__)
  buf = getenv(varName.c_str());
  return buf && buf[0];
#else
  bool valid;

  valid = (_dupenv_s(&buf, NULL, varName.c_str()) == 0) && (buf != NULL);
  free(buf); //perfectly valid to free a NULL pointer
  return valid;
#endif
}

/// Return the value of the environment variable (if it exists; an empty string otherwise).
std::string Environment::getVariable(const std::string& varName)
{
  char* buf;
#if !defined(_WIN32) || defined(__CYGWIN__)
  buf = getenv(varName.c_str());
  if (buf && buf[0])
    return buf;
  return std::string();
#else
  bool valid;
  std::string result;

  valid = (_dupenv_s(&buf, NULL, varName.c_str()) == 0) && (buf != NULL);
  if (valid)
    result = buf;
  free(buf); //perfectly valid to free a NULL pointer
  return result;
#endif
}

/// Set the value of the environment variable \a varName to \a value.
void Environment::setVariable(const std::string& varName, const std::string& value)
{
#if !defined(_WIN32) || defined(__CYGWIN__)
  setenv(varName.c_str(), value.c_str(), 1);
#else
  _putenv_s(varName.c_str(), value.c_str());
#endif
}

} // namespace common
} // namespace smtk
