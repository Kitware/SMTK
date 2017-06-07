//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/operators/WarpMesh.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/mesh/ApplyToMesh.h"
#include "smtk/mesh/CellField.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/PointField.h"

#include "smtk/mesh/interpolation/InverseDistanceWeighting.h"
#include "smtk/mesh/interpolation/PointCloud.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Session.h"

#include <array>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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

namespace
{
bool readCSVFile(
  const std::string& fileName, std::vector<double>& coordinates, std::vector<double>& values)
{
  std::ifstream infile(fileName.c_str());
  if (!infile.good())
  {
    return false;
  }
  std::string line;
  regex re(",");
  while (std::getline(infile, line))
  {
    // passing -1 as the submatch index parameter performs splitting
    sregex_token_iterator first{ line.begin(), line.end(), re, -1 }, last;

    // Se are looking for (x, y, z, value). So, we must have at least 4
    // components.
    if (std::distance(first, last) < 4)
    {
      return false;
    }

    coordinates.push_back(std::stod(*(first++)));
    coordinates.push_back(std::stod(*(first++)));
    coordinates.push_back(std::stod(*(first++)));
    values.push_back(std::stod(*(first++)));
  }

  infile.close();
  return true;
}
}

namespace smtk
{
namespace mesh
{

bool WarpMesh::ableToOperate()
{
  if (!this->ensureSpecification())
  {
    return false;
  }

  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");
  if (!meshItem || meshItem->numberOfValues() == 0)
  {
    return false;
  }

  return true;
}

smtk::model::OperatorResult WarpMesh::operateInternal()
{
  // Access the mesh to elevate
  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");

  // Access the interpolation points
  smtk::attribute::GroupItem::Ptr interpolationPointsItem = this->findGroup("points");

  // Access the interpolation power parameter
  smtk::attribute::DoubleItem::Ptr powerItem = this->findDouble("power");

  // Construct containers for our source points
  std::vector<double> sourceCoordinates;
  std::vector<double> sourceValues;

  // Access the points CSV file name, if it is enabled
  smtk::attribute::FileItem::Ptr ptsFileItem = this->specification()->findFile("ptsfile");
  if (ptsFileItem->isEnabled())
  {
    bool success = readCSVFile(ptsFileItem->value(0), sourceCoordinates, sourceValues);
    if (!success)
    {
      smtkErrorMacro(this->log(), "Could not read CSV file.");
      return this->createResult(smtk::model::OPERATION_FAILED);
    }
  }

  for (std::size_t i = 0; i < interpolationPointsItem->numberOfGroups(); i++)
  {
    smtk::attribute::DoubleItemPtr pointItem =
      smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(interpolationPointsItem->item(i, 0));
    for (std::size_t j = 0; j < 3; j++)
    {
      sourceCoordinates.push_back(pointItem->value(j));
    }
    sourceValues.push_back(pointItem->value(3));
  }

  // Construct an instance of our interpolator and set its parameters
  smtk::mesh::PointCloud pointcloud(sourceCoordinates, sourceValues);
  smtk::mesh::InverseDistanceWeighting interpolator(pointcloud, powerItem->value());

  // Access the attribute associated with the modified meshes
  smtk::model::OperatorResult result = this->createResult(smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::MeshItem::Ptr modifiedMeshes = result->findMesh("mesh_modified");
  modifiedMeshes->setNumberOfValues(meshItem->numberOfValues());

  // Access the attribute associated with the changed tessellation
  smtk::attribute::ModelEntityItem::Ptr modifiedEntities = result->findModelEntity("tess_changed");
  modifiedEntities->setNumberOfValues(meshItem->numberOfValues());

  std::function<std::array<double, 3>(std::array<double, 3>)> fn = [&](std::array<double, 3> x) {
    return std::array<double, 3>({ { x[0], x[1], interpolator(x) } });
  };

  // apply the interpolator to the meshes and populate the result attributes
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::MeshSet mesh = meshItem->value(i);

    smtk::mesh::applyWarp(fn, mesh, true);

    modifiedMeshes->appendValue(mesh);

    smtk::model::EntityRefArray entities;
    bool entitiesAreValid = mesh.modelEntities(entities);
    if (entitiesAreValid && !entities.empty())
    {
      smtk::model::Model model = entities[0].owningModel();
      this->addEntityToResult(result, model, MODIFIED);
      modifiedEntities->appendValue(model);
    }
  }

  return result;
}
}
}

#include "smtk/mesh/WarpMesh_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::mesh::WarpMesh, warp_mesh, "warp mesh",
  WarpMesh_xml, smtk::model::Session);
