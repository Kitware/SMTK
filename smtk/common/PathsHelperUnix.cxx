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
#include "smtk/common/PathsHelperUnix.h"
#include "smtk/common/Environment.h"
#include "smtk/common/Paths.h"
#include "smtk/common/Version.h"

#include <cstdlib>
#include <sstream>

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

namespace smtk
{
namespace common
{

PathsHelperUnix::PathsHelperUnix()
{
  Paths::s_bundleDir.clear();
  Paths::s_toplevelDir.clear();
  Paths::s_executableDir.clear();
  Paths::s_workerSearchPaths.clear();
  Paths::s_userConfigurationDirectory.clear();

  std::set<std::string> workerSearch;
  workerSearch.insert(Paths::currentDirectory());
  workerSearch.insert(Paths::s_toplevelDirCfg + "/var/smtk/workers");
  workerSearch.insert(
    Paths::s_toplevelDirCfg + "/var/smtk/" + smtk::common::Version::number() + "/workers");

  // Detect the path to the current executable.
  if (Paths::s_executable.empty())
  {
    std::vector<char> buf;
    buf.resize(512);
    ssize_t result = 0;
    for (; result >= 0 && result < static_cast<ssize_t>(buf.size()); buf.resize(buf.size() * 2))
    {
      result = readlink("/proc/self/exe", buf.data(), buf.size());
      if (result > 0 && static_cast<std::vector<char>::size_type>(result) < buf.size())
      {
        buf[result] = '\0';
        Paths::s_executable = buf.data();
        break;
      }
    }
  }
  // Use the current executable path to determine the directory containing it.
  Paths::s_executableDir = Paths::s_executable;
  std::string::size_type pos = Paths::s_executableDir.rfind('/');
  if (pos != std::string::npos)
  {
    Paths::s_executableDir = Paths::s_executableDir.substr(0, pos);
  }
  // If we have a valid directory locating the executable, look to see if it
  // can be used as a "top-level" directory (i.e., does it contain other
  // subdirectories that will hold workflow data?).
  if (!Paths::s_executableDir.empty())
  {
    std::filesystem::path ed = Paths::s_executableDir;
    // Only accept the directory containing the "bin" directory
    // if its parent also has "share":
    if (std::filesystem::exists(ed.parent_path() / "share"))
    {
      Paths::s_toplevelDir = ed.parent_path().string();
    }
  }

  // If we failed to obtain directories, fall back to the project's install prefix.
  if (Paths::s_toplevelDir.empty())
    Paths::s_toplevelDir = Paths::s_toplevelDirCfg;
  if (Paths::s_executableDir.empty())
    Paths::s_executableDir = Paths::s_toplevelDir + "/bin";

  PathsHelperUnix::AddSplitPaths(workerSearch, Environment::getVariable("SMTK_WORKER_SEARCH_PATH"));

  Paths::s_workerSearchPaths = std::vector<std::string>(workerSearch.begin(), workerSearch.end());

  {
    std::filesystem::path cfgDir;
    // On Linux, we put configuration files in ~/.config
    const char* baseDir = getenv("XDG_CONFIG_HOME");
    if (!baseDir)
    {
      const char* homePath = getenv("HOME");
      if (!homePath)
      {
        cfgDir = "/tmp";
      }
      else
      {
        cfgDir = std::string(homePath);
        cfgDir = cfgDir / ".config";
      }
    }
    else
    {
      cfgDir = baseDir;
    }
    Paths::s_userConfigurationDirectory = cfgDir;
  }
  {
    auto* pw = getpwuid(getuid());
    const char* homePath = pw->pw_dir;
    if (homePath)
    {
      Paths::s_userHomeDirectory = std::string(homePath);
    }
    else
    {
      homePath = getenv("HOME");
      if (homePath)
      {
        Paths::s_userHomeDirectory = std::string(homePath);
      }
    }
  }
  Paths::s_userDocumentDirectory = Paths::s_userHomeDirectory / "Documents";
}

void PathsHelperUnix::AddSplitPaths(std::set<std::string>& split, const std::string& src)
{
  std::stringstream envSearch(src);
  std::string spath;
  while (std::getline(envSearch, spath, ':'))
    if (!spath.empty())
      split.insert(spath);
}

} // namespace common
} // namespace smtk
