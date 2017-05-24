//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_io_Helpers_h
#define __smtk_io_Helpers_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/SystemConfig.h"

#include <set>
#include <string>

namespace smtk
{
namespace io
{

/**\brief A collection of methods to aid I/O.
  *
  */
class SMTKCORE_EXPORT Helpers
{
public:
  /**\brief Given 2 directories (which may not yet exist), determine
    * as far as the filesystem will let us whether \a dirB is a subdirectory of \a dirA.
    *
    * When returning true, \a bRelativeToA is set to the relative path of \bb.
    */
  static bool isDirectoryASubdirectory(
    const std::string& dirA, const std::string& dirB, std::string& bRelativeToA);

  /**\brief Returns true if \a pathToFile is an SMTK file (ends in .smtk).
    *
    * When returning true, the directory contains the file is stored in \a containingDir.
    */
  static bool isSMTKFilename(const std::string& pathToFile, std::string& containingDir);

  /**\brief Construct a unique filename given a starting point and a set of pre-existing filenames.
    *
    * The return value is inserted into \a preExisting just prior to return.
    * If \a start is a directory rather than a file or a stem cannot be detected,
    * then \a defaultStem will be used as the stem of the file name.
    *
    * All names will be taken relative to the \a base directory.
    */
  static std::string uniqueFilename(const std::string& start, std::set<std::string>& preExisting,
    const std::string& defaultStem, const std::string& defaultExtension = "",
    const std::string& base = ".");
};

} // namespace io
} // namespace smtk

#endif // __smtk_io_Helpers_h
