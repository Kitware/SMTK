//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/operators/InterpolateOntoMesh.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/mesh/ApplyToMesh.h"
#include "smtk/mesh/CellField.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/PointField.h"

#include "smtk/mesh/interpolation/InverseDistanceWeighting.h"
#include "smtk/mesh/interpolation/PointCloud.h"
#include "smtk/mesh/interpolation/PointCloudGenerator.h"
#include "smtk/mesh/interpolation/RadialAverage.h"
#include "smtk/mesh/interpolation/StructuredGrid.h"
#include "smtk/mesh/interpolation/StructuredGridGenerator.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Session.h"

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

template <typename InputType>
std::function<double(std::array<double, 3>)> radialAverageFrom(
  const InputType& input, double radius, smtk::mesh::Manager& meshManager)
{
  std::function<double(std::array<double, 3>)> radialAverage;
  {
    // Let's start by trying to make a structured grid, since they can be a
    // subset of point clouds.
    smtk::mesh::StructuredGridGenerator sgg;
    smtk::mesh::StructuredGrid structuredgrid = sgg(input);
    if (structuredgrid.size() > 0)
    {
      radialAverage = smtk::mesh::RadialAverage(structuredgrid, radius);
    }
  }

  if (!radialAverage)
  {
    // Next, we try to make a point cloud from our data.
    smtk::mesh::PointCloudGenerator pcg;
    smtk::mesh::PointCloud pointcloud = pcg(input);
    if (pointcloud.size() > 0)
    {
      radialAverage = smtk::mesh::RadialAverage(meshManager.makeCollection(), pointcloud, radius);
    }
  }

  return radialAverage;
}

template <typename InputType>
std::function<double(std::array<double, 3>)> inverseDistanceWeightingFrom(
  const InputType& input, double power)
{
  std::function<double(std::array<double, 3>)> idw;
  {
    // Let's start by trying to make a structured grid, since they can be a
    // subset of point clouds.
    smtk::mesh::StructuredGridGenerator sgg;
    smtk::mesh::StructuredGrid structuredgrid = sgg(input);
    if (structuredgrid.size() > 0)
    {
      idw = smtk::mesh::InverseDistanceWeighting(structuredgrid, power);
    }
  }

  if (!idw)
  {
    // Next, we try to make a point cloud from our data.
    smtk::mesh::PointCloudGenerator pcg;
    smtk::mesh::PointCloud pointcloud = pcg(input);
    if (pointcloud.size() > 0)
    {
      idw = smtk::mesh::InverseDistanceWeighting(pointcloud, power);
    }
  }

  return idw;
}
}

namespace smtk
{
namespace mesh
{

bool InterpolateOntoMesh::ableToOperate()
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

smtk::model::OperatorResult InterpolateOntoMesh::operateInternal()
{
  // Access the string describing the input data type
  smtk::attribute::StringItem::Ptr inputDataItem = this->specification()->findString("input data");

  // Access the mesh to elevate
  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");

  // Access the string describing the interpolation scheme
  smtk::attribute::StringItem::Ptr interpolationSchemeItem =
    this->specification()->findString("interpolation scheme");

  // Access the radius parameter
  smtk::attribute::DoubleItem::Ptr radiusItem = this->findDouble("radius");

  // Access the power parameter
  smtk::attribute::DoubleItem::Ptr powerItem = this->findDouble("power");

  // Access the data set name
  smtk::attribute::StringItem::Ptr nameItem = this->specification()->findString("dsname");

  // Access the output interpolation Field type
  smtk::attribute::IntItem::Ptr modeItem = this->specification()->findInt("interpmode");

  // Construct a function that takes an input point and returns a value
  // according to the average of the locus of points in the external data that,
  // when projected onto the x-y plane, are within a radius of the input
  std::function<double(std::array<double, 3>)> interpolation;

  if (inputDataItem->value() == "auxiliary geometry")
  {
    // Access the external data to use in determining elevation values
    smtk::attribute::ModelEntityItem::Ptr auxGeoItem =
      this->specification()->findModelEntity("auxiliary geometry");

    // Get the auxiliary geometry
    smtk::model::AuxiliaryGeometry auxGeo = auxGeoItem->value();

    if (interpolationSchemeItem->value() == "radial average")
    {
      // Compute the radial average function
      interpolation = radialAverageFrom<smtk::model::AuxiliaryGeometry>(
        auxGeo, radiusItem->value(), *this->manager()->meshes());
    }
    else if (interpolationSchemeItem->value() == "inverse distance weighting")
    {
      // Compute the inverse distance weighting function
      interpolation =
        inverseDistanceWeightingFrom<smtk::model::AuxiliaryGeometry>(auxGeo, powerItem->value());
    }

    if (!interpolation)
    {
      smtkErrorMacro(this->log(), "Could not convert auxiliary geometry.");
      return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
    }
  }
  else if (inputDataItem->value() == "ptsfile")
  {
    // Get the file name
    std::string fileName = this->specification()->findFile("ptsfile")->value();

    if (interpolationSchemeItem->value() == "radial average")
    {
      // Compute the radial average function
      interpolation =
        radialAverageFrom<std::string>(fileName, radiusItem->value(), *this->manager()->meshes());
    }
    else if (interpolationSchemeItem->value() == "inverse distance weighting")
    {
      // Compute the inverse distance weighting function
      interpolation = inverseDistanceWeightingFrom<std::string>(fileName, powerItem->value());
    }

    if (!interpolation)
    {
      smtkErrorMacro(this->log(), "Could not read file.");
      return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
    }
  }
  else if (inputDataItem->value() == "points")
  {
    // Access the interpolation points
    smtk::attribute::GroupItem::Ptr interpolationPointsItem = this->findGroup("points");

    // Construct containers for our source points
    std::vector<double> sourceCoordinates;
    std::vector<double> sourceValues;

    for (std::size_t i = 0; i < interpolationPointsItem->numberOfGroups(); i++)
    {
      smtk::attribute::DoubleItemPtr pointItem =
        smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(
          interpolationPointsItem->item(i, 0));
      for (std::size_t j = 0; j < 3; j++)
      {
        sourceCoordinates.push_back(pointItem->value(j));
      }
      sourceValues.push_back(pointItem->value(3));
    }

    smtk::mesh::PointCloud pointcloud(std::move(sourceCoordinates), std::move(sourceValues));

    if (interpolationSchemeItem->value() == "radial average")
    {
      interpolation = smtk::mesh::RadialAverage(
        this->manager()->meshes()->makeCollection(), pointcloud, radiusItem->value());
    }
    else if (interpolationSchemeItem->value() == "inverse distance weighting")
    {
      // Compute the inverse distance weighting function
      interpolation = smtk::mesh::InverseDistanceWeighting(pointcloud, powerItem->value());
    }

    if (!interpolation)
    {
      smtkErrorMacro(this->log(), "Could not read points.");
      return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
    }
  }
  else
  {
    smtkErrorMacro(this->log(), "Unrecognized input type.");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  // Access the attribute associated with the modified meshes
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
  smtk::attribute::MeshItem::Ptr modifiedMeshes = result->findMesh("mesh_modified");
  modifiedMeshes->setNumberOfValues(meshItem->numberOfValues());

  // Access the attribute associated with the changed tessellation
  smtk::attribute::ModelEntityItem::Ptr modifiedEntities = result->findModelEntity("tess_changed");
  modifiedEntities->setNumberOfValues(meshItem->numberOfValues());

  std::function<double(std::array<double, 3>)> fn = [&](
    std::array<double, 3> x) { return interpolation(x); };

  // apply the interpolator to the meshes and populate the result attributes
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::MeshSet mesh = meshItem->value(i);

    if (modeItem->value(0) == CELL_FIELD)
    {
      smtk::mesh::applyScalarCellField(fn, nameItem->value(), mesh);
    }
    else
    {
      smtk::mesh::applyScalarPointField(fn, nameItem->value(), mesh);
    }

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

#include "smtk/mesh/InterpolateOntoMesh_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::mesh::InterpolateOntoMesh, interpolate_onto_mesh,
  "interpolate onto mesh", InterpolateOntoMesh_xml, smtk::model::Session);
