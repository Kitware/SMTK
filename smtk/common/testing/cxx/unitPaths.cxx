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
#include "smtk/common/Paths.h"
#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>

#include <assert.h>

using namespace smtk::common;

int main(int argc, char* argv[])
{
  Paths p1;
  std::string fmwkp1 = p1.bundleDirectory();
  std::cout << "Bundle directory \"" << fmwkp1 << "\"\n";

  std::string exedirp1 = p1.executableDirectory();
  std::cout << "Executable directory \"" << exedirp1 << "\"\n";

  std::string instcfgp1 = p1.toplevelDirectoryConfigured();
  std::cout << "Configure-time top-level directory \"" << instcfgp1 << "\"\n";

  std::string instp1 = p1.toplevelDirectory();
  std::cout << "Toplevel without hint \"" << instp1 << "\"\n";
#ifdef __APPLE__
  test(instp1 == fmwkp1,
    "On Mac OS X, default toplevel dir should be bundle dir.");
  test(exedirp1 == fmwkp1 + "/Contents/MacOS",
    "On Mac OS X, the executable directory should be inside the bundle.");
#else
  test(instp1 == instcfgp1,
    "On this platform, default toplevel dir "
    "should be configure-time install prefix.");
#  if !defined(_WIN32) || defined(__CYGWIN__)
  test(exedirp1 == instp1 + "/bin",
    "On this platform, the executable directory should be inside the "
    "configure-time install prefix when no executable path is provided.");
#  else
  test(exedirp1 == instp1,
    "On this platform, the executable directory should be exactly the "
    "configure-time install prefix when no executable path is provided.");
#  endif
#endif

  test(argc > 0, "Program name not provided by executable!");

  Paths p2(argv[0]);
  std::string instp2 = p2.toplevelDirectory();
  std::cout << "Toplevel *with* hint \"" << instp2 << "\"\n";

  Paths p3;
  std::string fmwkp3 = p3.bundleDirectory();
  std::string exedirp3 = p3.executableDirectory();
  std::string instcfgp3 = p3.toplevelDirectoryConfigured();
  std::string instp3 = p3.toplevelDirectory();
  std::cout << "Toplevel without hint after previous hint \"" << instp3 << "\"\n";

  test(instp3 == instp2, "Toplevel directories do not match");

  // Now ask p1 for paths again and ensure they match the versions from p3
  // which have been cached.
  fmwkp1 = p1.bundleDirectory();
  exedirp1 = p1.executableDirectory();
  instcfgp1 = p1.toplevelDirectoryConfigured();
  instp1 = p1.toplevelDirectory();
  test(fmwkp3 == fmwkp1, "Stage 1 and stage 3 frameworks should match.");
  test(exedirp3 == exedirp1, "Stage 1 and stage 3 executable dirs should match.");
  test(instcfgp3 == instcfgp1, "Stage 1 and stage 3 config-time toplevel dirs should match.");
  test(instp3 == instp1, "Stage 1 and stage 3 toplevel dirs should match.");

  Environment::setVariable("SMTK_WORKER_SEARCH_PATH", "EnvTestA:EnvTestB");
  p3.forceUpdate();
  int testCount = 0;
  typedef std::vector<std::string> StringList;
  StringList workerDirs = p1.workerSearchPaths(false);
  std::cout << "Search for workers in:\n";
  for (StringList::iterator it = workerDirs.begin(); it != workerDirs.end(); ++it)
    {
    std::cout << "  " << *it << "\n";
    if (it->substr(0, 7) == "EnvTest" && it->size() == 8)
      ++testCount;
    }
  test(testCount == 2, "Expected to find EnvTestA and EnvTestB in list above.");

  return 0;
}
