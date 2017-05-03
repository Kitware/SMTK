//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/Helpers.h"

#include "boost/filesystem.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace smtk
{
namespace io
{

bool Helpers::isDirectoryASubdirectory(
  const std::string& aas, const std::string& bbs, std::string& brela)
{
  using namespace boost::filesystem;
  path aa(aas);
  path bb(bbs);
  path ac = weakly_canonical(aa);
  path bc =
    bb.is_relative() ? weakly_canonical(absolute(bb, ac).lexically_normal()) : weakly_canonical(bb);

  // Inputs to try:
  // + /tmp/foo/bar/baz/aa /tmp/foo/bb
  // + /tmp/foo/bar/baz/a bb
  // + /tmp/foo/bar/baz/a ../../barf/bb

  /*
  std::cout
    << "aa " << aa << "\n"
    << "bb " << bb << "\n"
    << "canonical(aa) " << ac << "\n"
    << "canonical(bb) " << bc << "\n"
    ;
    */

  path ar = weakly_canonical(aa);
  path br = bc;

  // Want to use std::mismatch(ar.begin(), ar.end(), br.begin(), br.end()).first == ar.end();
  // but it fails to compile on macos because "path::iterator does not provide a call operator".
  // return std::mismatch(ar.begin(), ar.end(), br.begin(), br.end()).first == ar.end();

  path::const_iterator ai;
  path::const_iterator bi;
  int ns = 0;
  bool diverged = false;
  for (ai = ar.begin(), bi = br.begin(); ai != ar.end() && bi != br.end(); ++ai, ++bi)
  {
    if (!diverged && *ai == *bi)
    {
      // std::cout << "  \"" << *ai << "\"\n";
      ++ns;
    }
    else
    {
      diverged = true;
    }
  }
  /*
  std::cout
    << "Paths have " << ns << " elements in common\n"
    << "Paths " << (diverged ? "have" : "have not") << " diverged\n"
    << "br rel ar is " << br.lexically_relative(ar) << "\n"
    << "ai @ end " << (ai == ar.end() ? "Y" : "N") << " bi @ end " << (bi == br.end() ? "Y" : "N") << "\n"
    ;
    */
  bool result = !diverged && ai == ar.end();
  if (result)
  {
    brela = br.lexically_relative(ar).string();
  }
  return result;
}

bool Helpers::isSMTKFilename(const std::string& pathToFile, std::string& containingDir)
{
  using namespace ::boost::filesystem;
  path ptf = weakly_canonical(pathToFile);
  /*
  std::cout
    << ptf << " stem " << ptf.stem() << "\n"
    << ptf << " extension " << ptf.extension() << "\n"
    ;
    */
  if (ptf.stem() != "" && ptf.extension() == ".smtk")
  {
    containingDir = ptf.parent_path().string();
    return true;
  }
  return false;
}

std::string Helpers::uniqueFilename(const std::string& start, std::set<std::string>& preExisting,
  const std::string& defaultStem, const std::string& defaultExtension, const std::string& base)
{
  using namespace ::boost::filesystem;
  std::string init(start);
  if (is_directory(relative(init, base)))
  {
    init = (path(init) / (defaultStem + "_1" + defaultExtension)).string();
  }
  if (preExisting.find(init) == preExisting.end() && !exists(relative(init, base)))
  {
    preExisting.insert(init);
    return init;
  }
  // Something matches; change the filename stem and keep trying until we succeed.
  using namespace ::boost::filesystem;
  path fname(init);
  std::string ext = fname.extension().string();
  if (ext.empty())
  {
    ext = defaultExtension;
  }
  do
  {
    std::string stem = fname.stem().string();
    if (stem.empty() || stem == ".")
    {
      stem = defaultStem + "_1";
    }
    else
    { // look for a pre-existing "_#" pattern.
      std::size_t found = stem.find_last_of("_");
      if (found == std::string::npos)
      {
        stem += "_1";
      }
      else
      {
        std::istringstream convert(stem.substr(found + 1));
        int seqnum = 0;
        if (convert >> seqnum)
        {
          std::ostringstream update;
          update << stem.substr(0, found) << "_" << (seqnum + 1);
          stem = update.str();
        }
        else
        {
          stem += "_1";
        }
      }
    }
    fname.remove_filename();
    fname /= (stem + ext);
  } while (preExisting.find(fname.string()) != preExisting.end() || exists(relative(fname, base)));
  preExisting.insert(fname.string());
  return fname.string();
}

} // namespace io
} // namespace smtk
