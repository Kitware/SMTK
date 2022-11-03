//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/operators/GenerateHotStartData.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/mesh/GenerateHotStartData_xml.h"
#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/PointField.h"

#include "smtk/mesh/interpolation/InverseDistanceWeighting.h"
#include "smtk/mesh/interpolation/PointCloud.h"

#include "smtk/mesh/utility/ApplyToMesh.h"

#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/Regex.h"

#include <array>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
// A key that corresponds to the .sbt file's values for output field type.
enum
{
  CELL_FIELD = 0,
  POINT_FIELD = 1
};

bool readCSVFile(
  const std::string& fileName,
  std::vector<double>& coordinates,
  std::vector<double>& values)
{
  std::ifstream infile(fileName.c_str());
  if (!infile.good())
  {
    return false;
  }
  std::string line;
  smtk::regex re(",");
  while (std::getline(infile, line))
  {
    // passing -1 as the submatch index parameter performs splitting
    smtk::sregex_token_iterator first{ line.begin(), line.end(), re, -1 }, last;

    // Se are looking for (x, y, value). So, we must have at least 3
    // components.
    if (std::distance(first, last) < 3)
    {
      return false;
    }

    coordinates.push_back(std::stod(*(first++)));
    coordinates.push_back(std::stod(*(first++)));
    coordinates.push_back(0.);
    values.push_back(std::stod(*(first++)));
  }

  infile.close();
  return true;
}

// Compute the bounding box of a point cloud
std::array<double, 6> bounds(const smtk::mesh::PointCloud& pc)
{
  std::array<double, 6> b = { { std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::lowest(),
                                std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::lowest(),
                                std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::lowest() } };

  for (std::size_t i = 0; i < pc.size(); i++)
  {
    auto coord = pc.coordinates()(i);
    for (std::size_t dim = 0; dim < 3; dim++)
    {
      if (coord[dim] < b[2 * dim])
      {
        b[2 * dim] = coord[dim];
      }
      if (coord[dim] > b[2 * dim + 1])
      {
        b[2 * dim + 1] = coord[dim];
      }
    }
  }

  return b;
}

// Compute the bounding box of a mesh set
std::array<double, 6> bounds(smtk::mesh::MeshSet ms)
{
  std::array<double, 6> b = { { std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::lowest(),
                                std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::lowest(),
                                std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::lowest() } };

  class Extent : public smtk::mesh::PointForEach
  {
  public:
    Extent()
    {
      m_values[0] = m_values[2] = m_values[4] = std::numeric_limits<double>::max();
      m_values[1] = m_values[3] = m_values[5] = std::numeric_limits<double>::lowest();
    }

    void forPoints(
      const smtk::mesh::HandleRange& /*pointIds*/,
      std::vector<double>& xyz,
      bool& /*coordinatesModified*/) override
    {
      for (std::size_t i = 0; i < xyz.size(); i += 3)
      {
        for (std::size_t j = 0; j < 3; j++)
        {
          if (xyz[i + j] < m_values[2 * j])
          {
            m_values[2 * j] = xyz[i + j];
          }
          if (xyz[i + j] > m_values[2 * j + 1])
          {
            m_values[2 * j + 1] = xyz[i + j];
          }
        }
      }
    }

    double m_values[6];
  };

  Extent extent;
  smtk::mesh::for_each(ms.points(), extent);
  for (std::size_t i = 0; i < 6; i++)
  {
    b[i] = extent.m_values[i];
  }
  return b;
}
} // namespace

namespace smtk
{
namespace mesh
{

bool GenerateHotStartData::ableToOperate()
{
  return this->Superclass::ableToOperate();
}

GenerateHotStartData::Result GenerateHotStartData::operateInternal()
{
  // Access the mesh to elevate
  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();

  // Access the data set type
  smtk::attribute::StringItem::Ptr typeItem = this->parameters()->findString("dstype");

  // Access the interpolation points
  smtk::attribute::GroupItem::Ptr interpolationPointsItem = this->parameters()->findGroup("points");

  // Access the interpolation power parameter
  smtk::attribute::DoubleItem::Ptr powerItem = this->parameters()->findDouble("power");

  // Construct containers for our source points
  std::vector<double> sourceCoordinates;
  std::vector<double> sourceValues;

  // Access the points CSV file name, if it is enabled
  smtk::attribute::FileItem::Ptr ptsFileItem = this->parameters()->findFile("ptsfile");
  if (ptsFileItem->isEnabled())
  {
    bool success = readCSVFile(ptsFileItem->value(0), sourceCoordinates, sourceValues);
    if (!success)
    {
      smtkErrorMacro(this->log(), "Could not read CSV file.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }

  for (std::size_t i = 0; i < interpolationPointsItem->numberOfGroups(); i++)
  {
    smtk::attribute::DoubleItemPtr pointItem =
      smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(interpolationPointsItem->item(i, 0));
    for (std::size_t j = 0; j < 2; j++)
    {
      sourceCoordinates.push_back(pointItem->value(j));
    }
    sourceCoordinates.push_back(0.);
    sourceValues.push_back(pointItem->value(2));
  }

  // Construct a point cloud from our source points
  smtk::mesh::PointCloud pointcloud(std::move(sourceCoordinates), std::move(sourceValues));

  // Check if any meshes extend beyond the bounds of our interpolation point cloud,
  // and warn if they do.
  std::array<double, 6> pc_bounds = bounds(pointcloud);
  bool meshIsContainedByPointCloud = true;
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::MeshSet mesh = meshItem->valueAs<smtk::mesh::Component>(i)->mesh();
    std::array<double, 6> mesh_bounds = bounds(mesh);

    for (std::size_t j = 0; j < 3; j++)
    {
      if (mesh_bounds[2 * j] < pc_bounds[2 * j] || mesh_bounds[2 * j + 1] > pc_bounds[2 * j + 1])
      {
        meshIsContainedByPointCloud = false;
      }
    }
  }

  if (!meshIsContainedByPointCloud)
  {
    std::stringstream s;
    s << "Input mesh extends beyond the interpolation point cloud.";
    smtkWarningMacro(this->log(), s.str());
  }

  // Construct an instance of our interpolator and set its parameters
  smtk::mesh::InverseDistanceWeighting interpolator(pointcloud, powerItem->value());

  // Construct a result object to hold our output
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Access the attribute associated with the modified model
  smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");

  std::function<double(std::array<double, 3>)> fn;
  std::string name;

  if (typeItem->value() == "Initial Depth")
  {
    name = "ioh";
    fn = [&](std::array<double, 3> x) { return interpolator(x); };
  }
  else if (typeItem->value() == "Initial Concentration")
  {
    name = "icon 1";
    fn = [&](std::array<double, 3> x) { return interpolator(x); };
  }
  else if (typeItem->value() == "Initial Water Surface Elevation")
  {
    name = "ioh";
    fn = [&](std::array<double, 3> x) { return interpolator(x) - x[2]; };
  }
  else
  {
    smtkErrorMacro(this->log(), "Unsupported data type.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // apply the interpolator to the meshes and populate the result attributes
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::Component::Ptr meshComponent = meshItem->valueAs<smtk::mesh::Component>(i);
    smtk::mesh::MeshSet mesh = meshComponent->mesh();
    smtk::mesh::utility::applyScalarPointField(fn, name, mesh);

    modified->appendValue(meshComponent);
    smtk::operation::MarkGeometry().markModified(meshComponent);

    smtk::model::EntityRefArray entities;
    bool entitiesAreValid = mesh.modelEntities(entities);
    if (entitiesAreValid && !entities.empty())
    {
      smtk::model::Model model = entities[0].owningModel();
      modified->appendValue(model.component());
      smtk::operation::MarkGeometry().markModified(model.component());
    }
  }

  return result;
}

const char* GenerateHotStartData::xmlDescription() const
{
  return GenerateHotStartData_xml;
}
} // namespace mesh
} // namespace smtk
