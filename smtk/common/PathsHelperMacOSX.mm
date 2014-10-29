#include "smtk/common/PathsHelperMacOSX.h"
#include "smtk/common/PathsHelperUnix.h"
#include "smtk/common/Paths.h"
#include "smtk/common/Environment.h"
#include "smtk/common/Version.h"

#import <Foundation/Foundation.h>

namespace smtk {
  namespace common {

PathsHelperMacOSX::PathsHelperMacOSX()
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
  if (pos != std::string::npos && pos != 0)
    {
    Paths::s_executableDir = Paths::s_executableDir.substr(0, pos);
    workerSearch.insert(Paths::s_executableDir);
    }

  NSBundle* bundle = [NSBundle mainBundle];
  if (bundle)
    {
    NSString* bpath = [bundle bundlePath];
    if (bpath)
      {
      Paths::s_bundleDir = [bpath UTF8String];
      workerSearch.insert(Paths::s_bundleDir + "/Contents/Resources");
      }
    }
  Paths::s_toplevelDir = Paths::s_bundleDir;
  if (Paths::s_toplevelDir.empty())
    Paths::s_toplevelDir = Paths::s_executableDir;
  if (Paths::s_executableDir.empty() && !Paths::s_toplevelDir.empty())
    Paths::s_executableDir = Paths::s_toplevelDir + "/Contents/MacOS";

  PathsHelperUnix::AddSplitPaths(
    workerSearch, Environment::getVariable("SMTK_WORKER_SEARCH_PATH"));

  Paths::s_workerSearchPaths =
    std::vector<std::string>(
      workerSearch.begin(), workerSearch.end());
}

  } // namespace common
} // namespace smtk
