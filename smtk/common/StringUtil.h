//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_common_StringUtil_h
#define __smtk_common_StringUtil_h

#include "smtk/CoreExports.h"

#include <string>
#include <vector>

namespace smtk {
  namespace common {

class SMTKCORE_EXPORT StringUtil
{
public:
  static std::string& trim(std::string& s);
  static std::string& trimLeft(std::string& s);
  static std::string& trimRight(std::string& s);

  static std::string& lower(std::string& s);
  static std::string& upper(std::string& s);

  static std::vector<std::string> split(
    const std::string& s, const std::string& sep,
    bool omitEmpty, bool trim);
};

  } // namespace common
} // namespace smtk

#endif // __smtk_common_StringUtil_h
