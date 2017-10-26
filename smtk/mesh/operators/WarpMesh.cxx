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
#include "smtk/mesh/interpolation/PointCloudGenerator.h"

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

  // Read in the points file to create a point cloud
  smtk::attribute::FileItem::Ptr ptsFileItem = this->specification()->findFile("ptsfile");

  smtk::mesh::PointCloud pointcloud;

  if (ptsFileItem->isEnabled())
  {
    smtk::mesh::PointCloudGenerator pcg;
    pointcloud = pcg(ptsFileItem->value());
  }
  else
  {
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

    pointcloud = smtk::mesh::PointCloud(std::move(sourceCoordinates), std::move(sourceValues));
  }

  // Construct an instance of our interpolator and set its parameters
  smtk::mesh::InverseDistanceWeighting interpolator(pointcloud, powerItem->value());

  std::function<std::array<double, 3>(std::array<double, 3>)> fn = [&](std::array<double, 3> x) {
    return std::array<double, 3>({ { x[0], x[1], interpolator(x) } });
  };

  // Access the attribute associated with the modified meshes
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
  smtk::attribute::MeshItem::Ptr modifiedMeshes = result->findMesh("mesh_modified");
  modifiedMeshes->setNumberOfValues(meshItem->numberOfValues());

  // Access the attribute associated with the changed tessellation
  smtk::attribute::ModelEntityItem::Ptr modifiedEntities = result->findModelEntity("tess_changed");
  modifiedEntities->setNumberOfValues(meshItem->numberOfValues());

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
