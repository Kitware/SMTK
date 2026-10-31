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
/**\brief Verify installed auxiliary-file discovery using temporary filesystem fixtures.
 *
 * Check bundle priority, fallback to a relocated install tree for individual
 * files, directory lookup, and normalization and deduplication of search paths.
 * Also verify that missing files and invalid requests return empty paths, and
 * that the search includes a candidate relative to the loaded SMTK library.
 * The fixtures supply cached runtime locations without modifying an installation;
 * they do not test platform-specific executable or app-bundle detection.
 */
#include "smtk/common/Paths.h"
#include "smtk/common/testing/cxx/helpers.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace
{
// Supply runtime locations without modifying a real installation or requiring
// a particular platform's executable layout. Restore the shared Paths cache.
class AuxiliaryPathsFixture : public smtk::common::Paths
{
public:
  AuxiliaryPathsFixture()
  {
    this->toplevelDirectory();
    m_bundle = s_bundleDir;
    m_top = s_toplevelDir;
    m_root = std::filesystem::path(Paths::tempDirectory()) / Paths::uniquePath();
    std::filesystem::create_directories(m_root);
    s_bundleDir = (m_root / "Example.app").string();
    s_toplevelDir = (m_root / "relocated-install").string();
  }

  ~AuxiliaryPathsFixture()
  {
    s_bundleDir = m_bundle;
    s_toplevelDir = m_top;
    std::error_code error;
    std::filesystem::remove_all(m_root, error);
  }

  void setTop(const std::filesystem::path& top) { s_toplevelDir = top.string(); }

private:
  std::string m_bundle;
  std::string m_top;
  std::filesystem::path m_root;
};
} // namespace

int main(int, char*[])
{
  namespace fs = std::filesystem;
  AuxiliaryPathsFixture paths;
  const fs::path bundle = fs::path(paths.bundleDirectory()) / "Contents/Resources/smtk";
  const fs::path install = fs::path(paths.toplevelDirectory()) / "share/smtk";
  const auto libraryPrefix = fs::path(smtk::common::Paths::pathToThisLibrary()).parent_path();
  const auto libraryRoot = (libraryPrefix / "share/smtk").lexically_normal();
  auto roots = paths.auxiliarySearchPaths();
  smtkTest(roots.front() == bundle, "Bundle resources should take priority.");
  smtkTest(roots.back() == install, "Relocated install resources should be a fallback.");
  smtkTest(
    std::find(roots.begin(), roots.end(), libraryRoot) != roots.end(),
    "Include resources relative to the loaded library.");
  smtkTest(
    !fs::exists(bundle) && !fs::exists(install),
    "Return candidate directories even before resources exist.");

  const fs::path relative = fs::path("workflows") / smtk::common::Paths::uniquePath() / "mesh.smtk";
  fs::create_directories((bundle / relative).parent_path());
  fs::create_directories((install / relative).parent_path());
  std::ofstream(install / relative) << "install template";
  smtkTest(
    paths.findAuxiliaryPath(relative) == install / relative,
    "A bundle directory without this resource must fall back to the install tree.");
  std::ofstream(bundle / relative) << "bundle template";
  smtkTest(
    paths.findAuxiliaryPath(relative) == bundle / relative,
    "Prefer bundled resources when both locations contain the requested file.");
  smtkTest(
    paths.findAuxiliaryPath(relative.parent_path()) == (bundle / relative).parent_path(),
    "Auxiliary lookup must support directories as well as files.");
  fs::remove(bundle / relative);
  smtkTest(
    paths.findAuxiliaryPath(relative) == install / relative,
    "Resolve each file independently instead of caching one auxiliary root.");
  smtkTest(
    paths.findAuxiliaryPath(relative.parent_path() / "missing.smtk").empty(),
    "Missing resources must return an empty path.");
  smtkTest(
    paths.findAuxiliaryPath(fs::path("unused") / ".." / relative) == install / relative,
    "Normalize relative paths before lookup.");
  smtkTest(paths.findAuxiliaryPath({}).empty(), "Reject an empty auxiliary path.");
  smtkTest(paths.findAuxiliaryPath(install / relative).empty(), "Reject absolute auxiliary paths.");
  smtkTest(
    paths.findAuxiliaryPath(fs::path("..") / "outside.smtk").empty(),
    "Reject paths that escape the auxiliary root.");

  paths.setTop(libraryPrefix);
  roots = paths.auxiliarySearchPaths();
  smtkTest(
    std::count(roots.begin(), roots.end(), libraryRoot) == 1,
    "Remove duplicate search directories while preserving order.");
  return 0;
}
