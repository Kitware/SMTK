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

#include "smtk/io/Logger.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
#include <boost/dll.hpp>
SMTK_THIRDPARTY_POST_INCLUDE

#include <cstdio> // for remove()
#include <fstream>
#include <iostream>

#include <cassert>

using namespace smtk::common;

namespace
{

bool directoryDoesNotThrow(bool allowAccess)
{
  std::cout << "Test parent directory computation when access " << (allowAccess ? "is" : "is not")
            << " allowed.\n";
  bool ok = true;
  using boost::filesystem::create_directories;
  using boost::filesystem::permissions;
  using boost::filesystem::perms;
  using boost::filesystem::remove_all;

  auto unlikely = smtk::common::Paths::uniquePath();
  auto unlikelyChild = unlikely + "/" + smtk::common::Paths::uniquePath();
  std::cout << "  Unlikely dirs: \"" << unlikelyChild << "\n";

  if (smtk::common::Paths::canonical(smtk::common::Paths::currentDirectory()).empty())
  {
    // We don't have a temp directory; skip the rest.
    return ok;
  }

  // Create a directory and subdirectory, then remove permissions from the parent.
  create_directories(unlikelyChild);
  if (!allowAccess)
  {
    permissions(unlikelyChild, perms::remove_perms | perms::all_all);
    permissions(unlikely, perms::remove_perms | perms::all_all);
  }
  try
  {
    auto parent = smtk::common::Paths::directory(unlikelyChild);
    std::cout << "  Parent of " << unlikelyChild << " is " << parent << "\n";

    // When directory access is allowed, since unlikelyChild is a directory,
    // it returns the input directory as the directory portion of the path
    // rather than its parent. In the case where allowAccess is false, we
    // should expect the parent directory to be returned (since we cannot
    // inspect unlikelyChild to determine its path). However, some platforms
    // (fedora33/36, windows) behave differently than others (macos) so we do
    // not force a validity check when access is disallowed.
    if (allowAccess)
    {
      smtkTest(parent == unlikelyChild, "Expected parent directories to match.");
      ok &= (parent == unlikelyChild);
    }
  }
  catch (...)
  {
    std::cerr << "ERROR: Caught exception trying to identify parent directory.\n";
    ok = false;
  }
  if (!allowAccess)
  {
    // Add permissions back and delete the directories whether we failed or not.
    permissions(unlikely, perms::add_perms | perms::all_all);
    permissions(unlikelyChild, perms::add_perms | perms::all_all);
  }
  remove_all(unlikely);

  return ok;
}

} // namespace

int main(int argc, char* argv[])
{
  smtk::io::Logger::instance().setFlushToStdout(true);
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
  test(instp1 == fmwkp1, "On Mac OS X, default toplevel dir should be bundle dir.");
  test(
    exedirp1 == fmwkp1 + "/Contents/MacOS",
    "On Mac OS X, the executable directory should be inside the bundle.");
#else
  test(
    instp1 == instcfgp1,
    "On this platform, default toplevel dir "
    "should be configure-time install prefix.");
#if !defined(_WIN32) || defined(__CYGWIN__)
  test(
    exedirp1 == instp1 + "/bin",
    "On this platform, the executable directory should be inside the "
    "configure-time install prefix when no executable path is provided.");
#else
  test(
    exedirp1 == instp1,
    "On this platform, the executable directory should be exactly the "
    "configure-time install prefix when no executable path is provided.");
#endif
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

  Environment::setVariable(
    "SMTK_WORKER_SEARCH_PATH",
#if !defined(_WIN32) || defined(__CYGWIN__)
    "EnvTestA:EnvTestB"
#else
    "EnvTestA;EnvTestB" // Note semicolon separator
#endif
  );
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

  // Test extraction of filename parts
  std::string example("example.tar.gz");
  auto extension = smtk::common::Paths::extension(example);
  smtkTest(extension == ".gz", "Expected \".gz\" extension, got \"" << extension << "\".");
  auto stem = smtk::common::Paths::stem(example);
  smtkTest(stem == "example.tar", "Expected \"example\" stem, got \"" << stem << "\".");
  auto filename = smtk::common::Paths::filename(example);
  smtkTest(
    filename == example,
    "Expected filename and example path to match, got \"" << filename << "\".");
  auto replacement = smtk::common::Paths::replaceExtension(example, ".bz2");
  smtkTest(
    replacement == "example.tar.bz2",
    "Expected \"example.tar.bz2\", got \"" << replacement << "\".");
  auto dir = smtk::common::Paths::directory(example);
  smtkTest(dir.empty(), "Expected empty directory, got \"" << dir << "\".");
  // TODO: Add more tests of directory (absolute and relative paths).

  // Test Paths::uniquePath(), Paths::canonical(), and Paths::areEquivalent().
  auto unlikely = smtk::common::Paths::uniquePath() + ".text";
  std::string canonical = smtk::common::Paths::canonical(unlikely);
  std::cout << "Unlikely path: \"" << unlikely << "\" vs canonical \"" << canonical << "\"\n";
  smtkTest(canonical.empty(), "Expected failure for non-existent path.");

  if (smtk::common::Paths::canonical(smtk::common::Paths::currentDirectory()).empty())
  {
    // We don't have a temp directory; skip the rest.
    return 0;
  }

  // Create the file on disk so we can properly canonicalize it.
  std::ofstream tmp(unlikely.c_str());
  tmp.write("foo\n", 4);
  if (!tmp.good())
  {
    // We couldn't write to the temp directory; skip the rest.
    return 0;
  }
  tmp.close();

  canonical = smtk::common::Paths::canonical(smtk::common::Paths::currentDirectory());
  std::cout << "Current directory: \"" << canonical << "\"\n";
  smtkTest(
    smtk::common::Paths::areEquivalent(canonical, smtk::common::Paths::currentDirectory()),
    "Expected test-binary directory to resolve properly.");

  canonical = smtk::common::Paths::canonical(".");
  std::cout << "Current path: \"" << canonical << "\"\n";
  smtkTest(canonical != ".", "Expected current working directory to be absolute.");
  smtkTest(
    !smtk::common::Paths::isRelative(canonical),
    "Expected canonical version of current working directory to be absolute.");

  canonical = smtk::common::Paths::canonical(unlikely, smtk::common::Paths::currentDirectory());
  std::cout << "File path relative to explicit directory: \"" << canonical << "\"\n";
  smtkTest(
    smtk::common::Paths::areEquivalent(canonical, unlikely),
    "Expected relative with explicit base-directory to resolve properly.");

  remove(unlikely.c_str());

  smtkTest(
    directoryDoesNotThrow(false), "Expected Paths::directory() to succeed without exception.");
  smtkTest(
    directoryDoesNotThrow(true), "Expected Paths::directory() to succeed without exception.");

  return 0;
}
