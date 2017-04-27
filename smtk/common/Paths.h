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
#ifndef __smtk_common_Paths_h
#define __smtk_common_Paths_h

#include "smtk/CoreExports.h"
#include "smtk/common/CompilerInformation.h"

#ifdef SMTK_MSVC
// Ignore symbol exposure warnings for STL classes.
#pragma warning(disable : 4251)
#endif

#include <string>
#include <vector>

namespace smtk
{
namespace common
{

/**\brief Obtain filesystem paths relevant to SMTK.
  *
  * This class encapsulates platform specific information
  * for obtaining search paths needed by SMTK.
  *
  * As much as possible, it uses run-time information
  * instead of configure- or compile-time information
  * to discover where things are located relative to
  * the current executable.
  */
class SMTKCORE_EXPORT Paths
{
public:
  Paths();
  Paths(const std::string& argv0);

  static std::string currentDirectory();
  static bool directoryExists(const std::string& path);
  static std::vector<std::string> pruneInvalidDirectories(const std::vector<std::string>& src);

  std::string executableDirectory();
  std::string toplevelDirectory();
  std::string bundleDirectory();
  std::vector<std::string> workerSearchPaths(bool pruneInvalid = true);

  std::string toplevelDirectoryConfigured();

  void forceUpdate();

protected:
  friend class PathsHelperUnix;
  friend class PathsHelperMacOSX;
  friend class PathsHelperWindows;

  bool update();

  static std::string s_executable;     // path to self (argv[0])
  static std::string s_toplevelDirCfg; // configure-time install prefix

  // timestamps for marking cache:
  static int s_lastSet;
  static int s_lastGen;

  // cached directories:
  static std::string s_executableDir;
  static std::string s_toplevelDir;
  static std::string s_bundleDir;
  static std::vector<std::string> s_workerSearchPaths;
};

} // namespace common
} // namespace smtk

#endif // __smtk_common_Paths_h
