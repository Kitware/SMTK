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

#include "smtk/common/Version.h"

#include "smtk/Options.h"
#include "smtk/io/Logger.h"

#include <cstdio>
#include <iostream>
#include <sstream>

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
#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
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
Paths::Paths() = default;

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

/// Create a directory (and all its containing directories as needed).
///
/// Returns true if the directory pre-existed or was successfully
/// created and false if it could not be created (e.g., due to
/// permissions, a full filesystem, or a plain file whose name
/// matches a directory in the \a path).
bool Paths::createDirectory(const std::string& path)
{
  if (Paths::fileExists(path))
  {
    return true;
  }
  return boost::filesystem::create_directories(path);
}

/// Is the given path a file?
bool Paths::fileExists(const std::string& path)
{
  bool ok = false;
  try
  {
    ok = boost::filesystem::exists(path.c_str());
  }
  catch (boost::filesystem::filesystem_error&)
  {
    // Do nothing. If we get here, it is because we do not
    // have permission to read a containing directory; assume
    // the file does not exist (to the best of our knowledge).
  }
  return ok;
}

/// Is the path relative (i.e., not absolute)?
///
/// Note that on Windows a path requires a drive
/// letter to be absolute, so more paths than you may
/// expect will be reported as relative.
bool Paths::isRelative(const std::string& path)
{
  return boost::filesystem::path(path).is_relative();
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

/// Return the directory containing the library that describes \a func.
std::string Paths::pathToLibraryContainingFunction(void (*func)())
{
  return boost::dll::symbol_location(*func).parent_path().string();
}

/// Return the directory containing this library.
std::string Paths::pathToThisLibrary()
{
  return boost::dll::symbol_location(smtk::common::Paths::pathToThisLibrary).parent_path().string();
}

/// Return true when \a pathA and \a pathB resolve to the same location.
///
/// Note: both input paths must exist on the filesystem, otherwise this
/// function will emit an error message and return false.
bool Paths::areEquivalent(const std::string& pathA, const std::string& pathB)
{
  boost::system::error_code err;
  bool result = boost::filesystem::equivalent(pathA, pathB, err);
  if (err)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), err.message());
    return false;
  }
  return result;
}

/// Return the canonical version of a path, which must exist on disk.
/// The canonical path is an unambiguous and absolute path with (1) no
/// single- or double-dots and (2) no symbolic links.
/// On error, an empty string is returned and a log message is generated.
std::string Paths::canonical(const std::string& path, const std::string& base)
{
  boost::system::error_code err;
  auto canonPath = base.empty() ? boost::filesystem::canonical(path, err)
                                : boost::filesystem::canonical(path, base, err);
  if (err)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), err.message());
    return std::string();
  }
  return canonPath.string();
}

/// Return the directory containing a file, given a path to the file.
std::string Paths::directory(const std::string& path)
{
  auto fullpath = boost::filesystem::path(path);
  try
  {
    if (boost::filesystem::exists(fullpath))
    {
      return boost::filesystem::is_directory(fullpath) ? fullpath.string()
                                                       : fullpath.parent_path().string();
    }
  }
  catch (boost::filesystem::filesystem_error&)
  {
    // Do nothing.
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

/// Return the path with file extension replaced
std::string Paths::replaceExtension(const std::string& path, const std::string& newExtension)
{
  return boost::filesystem::path(path).replace_extension(newExtension).string();
}

/// Return the path with filename replaced
std::string Paths::replaceFilename(const std::string& path, const std::string& newFilename)
{
  return boost::filesystem::path(path).parent_path().append(newFilename).string();
}

std::string Paths::tempDirectory()
{
  return boost::filesystem::temp_directory_path().string();
}

std::string Paths::uniquePath()
{
  return boost::filesystem::unique_path().string();
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

std::vector<std::string> Paths::findAvailablePlugins(const std::set<std::string>& pluginNames)
{
  std::vector<std::string> pluginList;
  boost::filesystem::path corePath = smtk::common::Paths::pathToThisLibrary();
  auto version = smtk::common::Version::number();
  std::ostringstream pluginTop;
  pluginTop << "smtk-" << version;
  boost::filesystem::path pluginDir = pluginTop.str();
  pluginDir = corePath / pluginDir;

  std::string ending;
#if !defined(_WIN32) && !defined(_WIN64)
  ending = ".so";
#else
  ending = ".dll";
#endif
  for (const auto& pp : boost::filesystem::directory_iterator(pluginDir))
  {
    if (boost::filesystem::is_directory(pp))
    {
      if (
        pluginNames.empty() || pluginNames.find(pp.path().filename().string()) != pluginNames.end())
      {
        auto pluginFile = pp / (pp.path().filename().string() + ending);
        if (boost::filesystem::is_regular_file(pluginFile))
        {
          pluginList.push_back(pluginFile.string());
        }
      }
    }
  }

  return pluginList;
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
