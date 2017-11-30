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
#include "smtk/common/Paths.h"

#if !defined(_WIN32) || defined(__CYGWIN__)
#include "smtk/common/PathsHelperUnix.h"
#ifdef __APPLE__
#include "smtk/common/PathsHelperMacOSX.h"
#endif
#else
#include "smtk/common/PathsHelperWindows.h"
#endif

#include "smtk/Options.h"

#include <stdio.h>

#include <sys/stat.h>
#include <sys/types.h>

#if !defined(_WIN32) || defined(__CYGWIN__)
#include <unistd.h>
#define smtkGetCurrentDir getcwd
#else
#include <direct.h>
#define smtkGetCurrentDir _getcwd
#endif

SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/dll.hpp>
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace common
{

/// The path to the current process's executable on the filesystem.
std::string Paths::s_executable;
/// The toplevel install-directory determined at configure-time; sometimes used as a hint.
std::string Paths::s_toplevelDirCfg(SMTK_INSTALL_PREFIX);

/// The last time the program path or other global setting was changed.
int Paths::s_lastSet = 0;
/// The last time the search-path cache was generated from global settings.
int Paths::s_lastGen = -1;

/// The directory containing the executable.
std::string Paths::s_executableDir;
/// The top-level install directory for SMTK.
std::string Paths::s_toplevelDir;
/// The bundle directory for SMTK on Mac OS X.
std::string Paths::s_bundleDir;
/// The ordered list of directories to search for Remus worker files.
std::vector<std::string> Paths::s_workerSearchPaths;

/// Construct a path-discovery instance without providing the current executable's path.
Paths::Paths()
{
}

/// Construct a path-discovery instance, providing the current executable's path for use as a starting point.
Paths::Paths(const std::string& argv0)
{
  if (argv0 != Paths::s_executable)
  {
    Paths::s_executableDir = argv0;
    Paths::s_lastSet = Paths::s_lastGen + 1;
  }
}

/// Return the current working directory.
std::string Paths::currentDirectory()
{
  char path[FILENAME_MAX];
  std::string result;
  if (smtkGetCurrentDir(path, sizeof(path)))
  {
    path[FILENAME_MAX - 1] = '\0';
    result = path;
  }
  return result;
}

/// Is the given path a directory?
bool Paths::directoryExists(const std::string& path)
{
  struct stat info;
  if (stat(path.c_str(), &info) != 0)
    return false;
  else if (info.st_mode & S_IFDIR)
    return true;
  return false;
}

/// Filter the input \a src, returning the subset of paths that are existing directories.
std::vector<std::string> Paths::pruneInvalidDirectories(const std::vector<std::string>& src)
{
  std::vector<std::string>::const_iterator it;
  std::vector<std::string> result;
  for (it = src.begin(); it != src.end(); ++it)
    if (Paths::directoryExists(*it))
      result.push_back(*it);
  return result;
}

/// Return the directory containing the library that describes <func>.
std::string Paths::pathToLibraryContainingFunction(void (*func)(void))
{
  return boost::dll::symbol_location(*func).parent_path().string();
}

// Return the directory containing this library.
std::string Paths::pathToThisLibrary()
{
  return boost::dll::symbol_location(smtk::common::Paths::pathToThisLibrary).parent_path().string();
}

/// Return the directory containing a file, given a path to the file.
std::string Paths::directory(const std::string& path)
{
  auto fullpath = boost::filesystem::path(path);
  if (boost::filesystem::exists(fullpath))
  {
    return boost::filesystem::is_directory(fullpath) ? fullpath.string()
                                                     : fullpath.parent_path().string();
  }
  return fullpath.parent_path().string();
}

/// Return the file name, given a path to the file.
std::string Paths::filename(const std::string& path)
{
  return boost::filesystem::path(path).filename().string();
}

/// Return the file name without extension, given a path to the file.
std::string Paths::stem(const std::string& path)
{
  return boost::filesystem::path(path).stem().string();
}

/// Return the file extension, given a path to the file.
std::string Paths::extension(const std::string& path)
{
  return boost::filesystem::path(path).extension().string();
}

/**\brief Return the best guess at the directory containing the current process's executable.
  */
std::string Paths::executableDirectory()
{
  this->update();
  return Paths::s_executableDir;
}

/**\brief Return the best guess at the top-level SMTK install directory.
  */
std::string Paths::toplevelDirectory()
{
  this->update();
  return Paths::s_toplevelDir;
}

/**\brief Return the best guess at the bundle directory.
  *
  * This should be empty on all platforms except Mac OS X and iOS.
  * Otherwise, it should point to the top of the main bundle (i.e., [NSBundle mainBundle]).
  */
std::string Paths::bundleDirectory()
{
  this->update();
  return Paths::s_bundleDir;
}

/**\brief Return the best guess at the top-level SMTK install directory.
  */
std::vector<std::string> Paths::workerSearchPaths(bool pruneInvalid)
{
  this->update();

  return pruneInvalid ? Paths::pruneInvalidDirectories(Paths::s_workerSearchPaths)
                      : Paths::s_workerSearchPaths;
}

/**\brief Return the top-level SMTK install directory configured when the package was built.
  */
std::string Paths::toplevelDirectoryConfigured()
{
  return Paths::s_toplevelDirCfg;
}

/**\brief Force the path cache to be rebuilt.
  *
  * This is useful if you believe an environment variable has changed
  * or some other undetectable event has occurred.
  */
void Paths::forceUpdate()
{
  Paths::s_lastSet = Paths::s_lastGen + 1;
  this->update();
}

/**\brief Update cached search paths and top-level install directories, if needed.
  */
bool Paths::update()
{
  if (Paths::s_lastSet > Paths::s_lastGen)
  {
// Generate new path caches.
// The helpers are friends of this class and update
// Paths' static-member cache directories themselves.
#if !defined(_WIN32) || defined(__CYGWIN__)
    PathsHelperUnix unixHelper;
#include "smtk/common/PathsHelperUnix.h"
#ifdef __APPLE__
    PathsHelperMacOSX macOSXHelper;
#endif
#else
    PathsHelperWindows windowsHelper;
#endif

    Paths::s_lastGen = Paths::s_lastSet;

    // Let's look for search paths relative to the location of the library
    // containing this method.
    boost::filesystem::path smtkLibDir =
      boost::dll::symbol_location(Paths::currentDirectory).parent_path();
    boost::filesystem::path smtkBinDir = smtkLibDir.parent_path() / "bin";
    if (boost::filesystem::is_directory(smtkBinDir))
    {
      Paths::s_workerSearchPaths.push_back(smtkBinDir.string());
    }

    return true;
  }
  return false;
}

} // namespace common
} // namespace smtk
