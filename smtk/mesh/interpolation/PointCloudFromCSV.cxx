//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/interpolation/PointCloudFromCSV.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/Paths.h"

// We use either STL regex or Boost regex, depending on support. These flags
// correspond to the equivalent logic used to determine the inclusion of Boost's
// regex library.
#if defined(SMTK_CLANG) ||                                                                         \
  (defined(SMTK_GCC) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)) ||                 \
  defined(SMTK_MSVC)
#include <regex>
using std::regex;
using std::sregex_token_iterator;
using std::regex_replace;
using std::regex_search;
using std::regex_match;
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::sregex_token_iterator;
using boost::regex_replace;
using boost::regex_search;
using boost::regex_match;
#endif

#include <fstream>
#include <stdexcept>
#include <vector>

namespace smtk
{
namespace mesh
{

namespace
{
static bool registered = PointCloudFromCSV::registerClass();
}

bool PointCloudFromCSV::valid(const std::string& fileName) const
{
  // For now, let's assume that the .csv suffix is a sufficient check.
  return smtk::common::Paths::extension(fileName) == ".csv";
}

smtk::mesh::PointCloud PointCloudFromCSV::operator()(const std::string& fileName)
{
  std::vector<double> coordinates;
  std::vector<double> values;

  std::ifstream infile(fileName.c_str());
  if (!infile.good())
  {
    throw std::invalid_argument("File cannot be read.");
  }
  std::string line;
  regex re(",");
  while (std::getline(infile, line))
  {
    // Passing -1 as the submatch index parameter performs splitting
    sregex_token_iterator first{ line.begin(), line.end(), re, -1 }, last;

    // We are looking for (x, y, z, value). So, we must have at least 4
    // components.
    if (std::distance(first, last) < 4)
    {
      throw std::invalid_argument("File does not contain enough parameters.");
    }

    coordinates.push_back(std::stod(*(first++)));
    coordinates.push_back(std::stod(*(first++)));
    coordinates.push_back(std::stod(*(first++)));
    values.push_back(std::stod(*(first++)));
  }

  infile.close();

  return smtk::mesh::PointCloud(std::move(coordinates), std::move(values));
}
}
}
