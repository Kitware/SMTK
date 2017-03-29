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

#include <sstream>

namespace smtk {
  namespace common {

PathsHelperUnix::PathsHelperUnix()
{
  Paths::s_bundleDir.clear();
  Paths::s_toplevelDir.clear();
  Paths::s_executableDir.clear();
  Paths::s_workerSearchPaths.clear();

  std::set<std::string> workerSearch;
  workerSearch.insert(Paths::currentDirectory());
  workerSearch.insert(Paths::s_toplevelDirCfg + "/var/smtk/workers");
  workerSearch.insert(Paths::s_toplevelDirCfg + "/var/smtk/" + smtk::common::Version::number() + "/workers");

  Paths::s_executableDir = Paths::s_executable;
  std::string::size_type pos = Paths::s_executableDir.rfind('/');
  if (pos != std::string::npos)
    Paths::s_executableDir = Paths::s_executableDir.substr(0, pos);

  if (Paths::s_toplevelDir.empty())
    Paths::s_toplevelDir = Paths::s_toplevelDirCfg;
  if (Paths::s_executableDir.empty())
    Paths::s_executableDir = Paths::s_toplevelDir + "/bin";

  PathsHelperUnix::AddSplitPaths(
    workerSearch, Environment::getVariable("SMTK_WORKER_SEARCH_PATH"));

  Paths::s_workerSearchPaths =
    std::vector<std::string>(
      workerSearch.begin(), workerSearch.end());
}

void PathsHelperUnix::AddSplitPaths(
  std::set<std::string>& split,
  const std::string& src)
{
  std::stringstream envSearch(src);
  std::string spath;
  while (std::getline(envSearch, spath, ':'))
    if (!spath.empty())
      split.insert(spath);
}

  } // namespace common
} // namespace smtk
