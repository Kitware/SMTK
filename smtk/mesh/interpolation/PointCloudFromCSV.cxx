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

#include "smtk/Regex.h"

#include <fstream>
#include <stdexcept>
#include <vector>

namespace smtk
{
namespace mesh
{

namespace
{
bool registered = PointCloudFromCSV::registerClass();
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
  smtk::regex re(",");
  while (std::getline(infile, line))
  {
    // Passing -1 as the submatch index parameter performs splitting
    smtk::sregex_token_iterator first{ line.begin(), line.end(), re, -1 }, last;

    // We are looking for (x, y, z, value), but we will also accept
    // (x, y, value). So, we must have at least 3 components.
    std::size_t dist = std::distance(first, last);
    if (dist < 3)
    {
      throw std::invalid_argument("File does not contain enough parameters.");
    }

    coordinates.push_back(std::stod(*(first++)));
    coordinates.push_back(std::stod(*(first++)));
    coordinates.push_back(dist == 4 ? std::stod(*(first++)) : 0.);
    values.push_back(std::stod(*(first++)));
  }

  infile.close();

  return smtk::mesh::PointCloud(std::move(coordinates), std::move(values));
}
} // namespace mesh
} // namespace smtk
