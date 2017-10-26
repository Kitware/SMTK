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
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/mesh/ApplyToMesh.h"
#include "smtk/mesh/CellField.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/PointField.h"

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

namespace smtk
{
namespace mesh
{

bool ElevateMesh::ableToOperate()
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

smtk::model::OperatorResult ElevateMesh::operateInternal()
{
  // Access the external data to use in determining elevation values
  smtk::attribute::ModelEntityItem::Ptr auxGeoItem =
    this->specification()->findModelEntity("auxiliary geometry");

  // Access the mesh to elevate
  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");

  // Access the radius parameter
  smtk::attribute::DoubleItem::Ptr radiusItem = this->findDouble("radius");

  // Access the min elevation parameter
  smtk::attribute::DoubleItem::Ptr minElevationItem = this->findDouble("min elevation");

  // Access the max elevation parameter
  smtk::attribute::DoubleItem::Ptr maxElevationItem = this->findDouble("max elevation");

  // Access the invert scalars parameter
  smtk::attribute::VoidItem::Ptr invertScalarsItem = this->findVoid("invert scalars");

  // Construct a function that inverts and clips its input, according to the input parameters
  std::function<double(double)> postProcess;
  {
    double prefactor = invertScalarsItem->isEnabled() ? -1. : 1.;

    if (minElevationItem->isEnabled() && maxElevationItem->isEnabled())
    {
      double minElevation = minElevationItem->value();
      double maxElevation = maxElevationItem->value();
      postProcess = [=](double input) {
        double output = prefactor * input;
        return (
          output < minElevation ? minElevation : (output > maxElevation ? maxElevation : output));
      };
    }
    else if (minElevationItem->isEnabled())
    {
      double minElevation = minElevationItem->value();
      postProcess = [=](double input) {
        double output = prefactor * input;
        return (output < minElevation ? minElevation : output);
      };
    }
    else if (maxElevationItem->isEnabled())
    {
      double maxElevation = maxElevationItem->value();
      postProcess = [=](double input) {
        double output = prefactor * input;
        return (output > maxElevation ? maxElevation : output);
      };
    }
    else
    {
      postProcess = [=](double input) { return prefactor * input; };
    }
  }

  // Get the auxiliary geometry and check its validity
  smtk::model::AuxiliaryGeometry auxGeo = auxGeoItem->value();
  if (!auxGeo.isValid())
  {
    smtkErrorMacro(this->log(), "Invalid auxiliary geometry.");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  // Construct a function that takes an input point and returns a value
  // according to the average of the locus of points in the external data that,
  // when projected onto the x-y plane, are within a radius of the input
  std::function<double(std::array<double, 3>)> radialAverage;

  {
    smtk::mesh::StructuredGridGenerator sgg;
    smtk::mesh::StructuredGrid structuredgrid = sgg(auxGeo);
    if (structuredgrid.size() > 0)
    {
      radialAverage = smtk::mesh::RadialAverage(structuredgrid, radiusItem->value());
    }
  }

  if (!radialAverage)
  {
    smtk::mesh::PointCloudGenerator pcg;
    smtk::mesh::PointCloud pointcloud = pcg(auxGeo);
    if (pointcloud.size() > 0)
    {
      radialAverage = smtk::mesh::RadialAverage(
        this->manager()->meshes()->makeCollection(), pointcloud, radiusItem->value());
    }
  }

  if (!radialAverage)
  {
    smtkErrorMacro(this->log(), "Could not convert auxiliary geometry.");
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

  std::function<std::array<double, 3>(std::array<double, 3>)> fn = [&](std::array<double, 3> x) {
    return std::array<double, 3>({ { x[0], x[1], postProcess(radialAverage(x)) } });
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

#include "smtk/mesh/ElevateMesh_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::mesh::ElevateMesh, elevate_mesh, "elevate mesh",
  ElevateMesh_xml, smtk::model::Session);
