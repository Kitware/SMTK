//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/operators/ElevateMesh.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/interpolation/InverseDistanceWeighting.h"
#include "smtk/mesh/interpolation/PointCloud.h"
#include "smtk/mesh/interpolation/PointCloudGenerator.h"
#include "smtk/mesh/interpolation/RadialAverage.h"
#include "smtk/mesh/interpolation/StructuredGrid.h"
#include "smtk/mesh/interpolation/StructuredGridGenerator.h"

#include "smtk/mesh/operators/ElevateMesh_xml.h"
#include "smtk/mesh/utility/ApplyToMesh.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"

#include "smtk/operation/MarkGeometry.h"

#include <array>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
template<typename InputType>
std::function<double(std::array<double, 3>)> radialAverageFrom(
  const InputType& input,
  double radius,
  const std::function<bool(double)>& prefilter,
  const smtk::mesh::InterfacePtr& interface)
{
  std::function<double(std::array<double, 3>)> radialAverage;
  {
    // Let's start by trying to make a structured grid, since they can be a
    // subset of point clouds.
    smtk::mesh::StructuredGridGenerator sgg;
    smtk::mesh::StructuredGrid structuredgrid = sgg(input);
    if (structuredgrid.size() > 0)
    {
      radialAverage = smtk::mesh::RadialAverage(structuredgrid, radius, prefilter);
    }
  }

  if (!radialAverage)
  {
    // Next, we try to make a point cloud from our data.
    smtk::mesh::PointCloudGenerator pcg;
    smtk::mesh::PointCloud pointcloud = pcg(input);
    if (pointcloud.size() > 0)
    {
      radialAverage = smtk::mesh::RadialAverage(
        smtk::mesh::Resource::create(interface), pointcloud, radius, prefilter);
    }
  }

  return radialAverage;
}

template<typename InputType>
std::function<double(std::array<double, 3>)> inverseDistanceWeightingFrom(
  const InputType& input,
  double power,
  const std::function<bool(double)>& prefilter)
{
  std::function<double(std::array<double, 3>)> idw;
  {
    // Let's start by trying to make a structured grid, since they can be a
    // subset of point clouds.
    smtk::mesh::StructuredGridGenerator sgg;
    smtk::mesh::StructuredGrid structuredgrid = sgg(input);
    if (structuredgrid.size() > 0)
    {
      idw = smtk::mesh::InverseDistanceWeighting(structuredgrid, power, prefilter);
    }
  }

  if (!idw)
  {
    // Next, we try to make a point cloud from our data.
    smtk::mesh::PointCloudGenerator pcg;
    smtk::mesh::PointCloud pointcloud = pcg(input);
    if (pointcloud.size() > 0)
    {
      idw = smtk::mesh::InverseDistanceWeighting(pointcloud, power, prefilter);
    }
  }

  return idw;
}
} // namespace

namespace smtk
{
namespace mesh
{

bool ElevateMesh::ableToOperate()
{
  return this->Superclass::ableToOperate();
}

ElevateMesh::Result ElevateMesh::operateInternal()
{
  // Access the string describing the input data type
  smtk::attribute::StringItem::Ptr inputDataItem = this->parameters()->findString("input data");

  // Access the mesh to elevate
  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();
  smtk::mesh::MeshSet meshset = meshItem->valueAs<smtk::mesh::Component>()->mesh();
  const smtk::mesh::Resource::Ptr& resource = meshset.resource();

  // Access the string describing the interpolation scheme
  smtk::attribute::StringItem::Ptr interpolationSchemeItem =
    this->parameters()->findString("interpolation scheme");

  // Access the radius parameter
  smtk::attribute::DoubleItem::Ptr radiusItem = this->parameters()->findDouble("radius");

  // Access the string describing external point treatment
  smtk::attribute::StringItem::Ptr externalPointItem =
    this->parameters()->findString("external point values");

  // Access the power parameter
  smtk::attribute::DoubleItem::Ptr powerItem = this->parameters()->findDouble("power");

  // Construct a prefilter for the input data
  std::function<bool(double)> prefilter = [](double /*unused*/) { return true; };
  {
    // Access the input filter parameter group
    smtk::attribute::GroupItem::Ptr inputFilterItem = this->parameters()->findGroup("input filter");

    if (inputFilterItem)
    {
      // Access the min threshold parameter
      smtk::attribute::DoubleItem::Ptr minThresholdItem =
        inputFilterItem->findAs<smtk::attribute::DoubleItem>("min threshold");

      // Access the max threshold parameter
      smtk::attribute::DoubleItem::Ptr maxThresholdItem =
        inputFilterItem->findAs<smtk::attribute::DoubleItem>("max threshold");

      if (
        minThresholdItem && minThresholdItem->isEnabled() && maxThresholdItem &&
        maxThresholdItem->isEnabled())
      {
        double minThreshold = minThresholdItem->value();
        double maxThreshold = maxThresholdItem->value();
        prefilter = [=](double d) { return d >= minThreshold && d <= maxThreshold; };
      }
      else if (minThresholdItem && minThresholdItem->isEnabled())
      {
        double minThreshold = minThresholdItem->value();
        prefilter = [=](double d) { return d >= minThreshold; };
      }
      else if (maxThresholdItem && maxThresholdItem->isEnabled())
      {
        double maxThreshold = maxThresholdItem->value();
        prefilter = [=](double d) { return d <= maxThreshold; };
      }
    }
  }

  // Construct a function that takes an input point and returns a value
  // according to the average of the locus of points in the external data that,
  // when projected onto the x-y plane, are within a radius of the input
  std::function<double(std::array<double, 3>)> interpolation;

  if (inputDataItem->value() == "auxiliary geometry")
  {
    // Access the external data to use in determining elevation values
    auto auxGeoItem = this->parameters()->findComponent("auxiliary geometry");

    // Get the auxiliary geometry
    smtk::model::AuxiliaryGeometry auxGeo = auxGeoItem->valueAs<smtk::model::Entity>();

    if (interpolationSchemeItem->value() == "radial average")
    {
      // Compute the radial average function
      interpolation = radialAverageFrom<smtk::model::AuxiliaryGeometry>(
        auxGeo, radiusItem->value(), prefilter, resource->interface());
    }
    else if (interpolationSchemeItem->value() == "inverse distance weighting")
    {
      // Compute the inverse distance weighting function
      interpolation = inverseDistanceWeightingFrom<smtk::model::AuxiliaryGeometry>(
        auxGeo, powerItem->value(), prefilter);
    }

    if (!interpolation)
    {
      smtkErrorMacro(this->log(), "Could not convert auxiliary geometry.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }
  else if (inputDataItem->value() == "ptsfile")
  {
    // Get the file name
    std::string fileName = this->parameters()->findFile("ptsfile")->value();

    if (interpolationSchemeItem->value() == "radial average")
    {
      // Compute the radial average function
      interpolation = radialAverageFrom<std::string>(
        fileName, radiusItem->value(), prefilter, resource->interface());
    }
    else if (interpolationSchemeItem->value() == "inverse distance weighting")
    {
      // Compute the inverse distance weighting function
      interpolation =
        inverseDistanceWeightingFrom<std::string>(fileName, powerItem->value(), prefilter);
    }

    if (!interpolation)
    {
      smtkErrorMacro(this->log(), "Could not read file.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }
  else if (inputDataItem->value() == "points")
  {
    // Access the interpolation points
    smtk::attribute::GroupItem::Ptr interpolationPointsItem =
      this->parameters()->findGroup("points");

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
        smtk::mesh::Resource::create(resource->interface()), pointcloud, radiusItem->value());
    }
    else if (interpolationSchemeItem->value() == "inverse distance weighting")
    {
      // Compute the inverse distance weighting function
      interpolation = smtk::mesh::InverseDistanceWeighting(pointcloud, powerItem->value());
    }

    if (!interpolation)
    {
      smtkErrorMacro(this->log(), "Could not read points.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }
  else
  {
    smtkErrorMacro(this->log(), "Unrecognized input type.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Construct a function that clips its input according to the input parameters
  std::function<double(double)> postProcess = [](double input) { return input; };
  {
    // Access the output filter parameter group
    smtk::attribute::GroupItem::Ptr outputFilterItem =
      this->parameters()->findGroup("output filter");

    if (outputFilterItem)
    {
      // Access the min elevation parameter
      smtk::attribute::DoubleItem::Ptr minElevationItem =
        outputFilterItem->findAs<smtk::attribute::DoubleItem>("min elevation");

      // Access the max elevation parameter
      smtk::attribute::DoubleItem::Ptr maxElevationItem =
        outputFilterItem->findAs<smtk::attribute::DoubleItem>("max elevation");

      if (
        minElevationItem && minElevationItem->isEnabled() && maxElevationItem &&
        maxElevationItem->isEnabled())
      {
        double minElevation = minElevationItem->value();
        double maxElevation = maxElevationItem->value();
        postProcess = [=](double input) {
          return (
            input < minElevation ? minElevation : (input > maxElevation ? maxElevation : input));
        };
      }
      else if (minElevationItem && minElevationItem->isEnabled())
      {
        double minElevation = minElevationItem->value();
        postProcess = [=](double input) { return (input < minElevation ? minElevation : input); };
      }
      else if (maxElevationItem && maxElevationItem->isEnabled())
      {
        double maxElevation = maxElevationItem->value();
        postProcess = [=](double input) { return (input > maxElevation ? maxElevation : input); };
      }
    }
  }

  // Add a conditional function for dealing with points that were rejected by the interpolator.
  std::function<double(std::array<double, 3>)> externalDataPoint = [](std::array<double, 3> xyz) {
    return xyz[2];
  };
  if (interpolationSchemeItem->value() == "radial average")
  {
    if (externalPointItem->value() == "set to NaN")
    {
      externalDataPoint = [](std::array<double, 3> /*unused*/) {
        return std::numeric_limits<double>::quiet_NaN();
      };
    }
    else if (externalPointItem->value() == "set to value")
    {
      double externalPointValue = this->parameters()->findDouble("external point value")->value();
      externalDataPoint = [=](std::array<double, 3> /*unused*/) { return externalPointValue; };
    }
  }

  // Combine the radial average function with the elevation clipping function.
  std::function<std::array<double, 3>(std::array<double, 3>)> fn = [&](std::array<double, 3> x) {
    double z = postProcess(interpolation(x));
    if (std::isnan(z))
    {
      z = externalDataPoint(x);
    }

    return std::array<double, 3>({ { x[0], x[1], z } });
  };

  // Access the attribute associated with the modified meshes
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Access the attribute associated with the modified model
  smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");

  // Mark the modified mesh components to update their representative geometry
  smtk::operation::MarkGeometry markGeometry(resource);

  // apply the interpolator to the meshes and populate the result attributes
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    auto meshComponent = meshItem->valueAs<smtk::mesh::Component>(i);
    auto mesh = meshComponent->mesh();

    smtk::mesh::utility::applyWarp(fn, mesh, true);

    modified->appendValue(meshComponent);
    markGeometry.markModified(meshComponent);

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

const char* ElevateMesh::xmlDescription() const
{
  return ElevateMesh_xml;
}
} // namespace mesh
} // namespace smtk
