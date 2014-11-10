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
#ifndef __smtk_common_PathsHelperUnix_h
#define __smtk_common_PathsHelperUnix_h

#include "smtk/SMTKCoreExports.h"

#include <set>
#include <string>

namespace smtk {
  namespace common {

class SMTKCORE_EXPORT PathsHelperUnix
{
public:
  PathsHelperUnix();

  static void AddSplitPaths(
    std::set<std::string>& splitPaths,
    const std::string& envVar);
};

  } // namespace common
} // namespace smtk

#endif // __smtk_common_PathsHelperUnix_h
