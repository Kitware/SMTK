//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/model/operators/InterpolateMeshElevation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/MeshItem.h"

#include "smtk/mesh/Displace.h"
#include "smtk/mesh/MeshSet.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Session.h"

#include <array>
#include <cmath>

namespace
{

// We use inverse distance weighting via Shepard's method, implmented below.
typedef std::array<double, 2> Point;

static const double EPSILON = 1.e-10;

double euclideanDistance(const Point& p1, const Point& p2)
{
  Point diff = { { p1[0] - p2[0], p1[1] - p2[1] } };
  return std::sqrt(diff[0] * diff[0] + diff[1] * diff[1]);
}

class ShepardInterpolator
{
public:
  // Return the interpolated value at <p> as a weighted sum of the sources
  double operator()(const Point& p) const
  {
    double d = 0., w = 0., num = 0., denom = 0.;
    for (auto& source : this->m_sources)
    {
      d = euclideanDistance(p, source.first);
      // If d is zero, then return the value associated with the source point.
      if (d < EPSILON)
      {
        return source.second;
      }
      // Otherwise, sum the contribution from each point.
      w = std::pow(d, -1. * this->m_power);
      num += w * source.second;
      denom += w;
    }

    return num / denom;
  }

  double operator()(double x, double y) const
  {
    const Point p = { { x, y } };
    return this->operator()(p);
  }

  void setPower(double power) { this->m_power = power; }
  void addSourcePoint(const Point& p, double value)
  {
    this->m_sources.push_back(std::make_pair(p, value));
  }

private:
  std::vector<std::pair<Point, double> > m_sources;
  double m_power = 1.;
};
}

namespace smtk
{
namespace model
{

bool InterpolateMeshElevation::ableToOperate()
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

smtk::model::OperatorResult InterpolateMeshElevation::operateInternal()
{
  // Access the mesh to elevate
  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");

  // Access the interpolation points
  smtk::attribute::GroupItem::Ptr interpolationPointsItem = this->findGroup("points");

  // Access the interpolation power parameter
  smtk::attribute::DoubleItem::Ptr powerItem = this->findDouble("power");

  // Construct an instance of our interpolator and set its parameters
  ShepardInterpolator interpolator;
  interpolator.setPower(powerItem->value());

  for (std::size_t i = 0; i < interpolationPointsItem->numberOfGroups(); i++)
  {
    smtk::attribute::DoubleItemPtr pointItem =
      smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(interpolationPointsItem->item(i, 0));

    Point p = { { pointItem->value(0), pointItem->value(1) } };
    interpolator.addSourcePoint(p, pointItem->value(2));
  }

  // Access the attribute associated with the modified meshes
  OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
  smtk::attribute::MeshItem::Ptr modifiedMeshes = result->findMesh("mesh_modified");
  modifiedMeshes->setNumberOfValues(meshItem->numberOfValues());

  // Access the attribute associated with the changed tessellation
  smtk::attribute::ModelEntityItem::Ptr modifiedEntities = result->findModelEntity("tess_changed");
  modifiedEntities->setNumberOfValues(meshItem->numberOfValues());

  // Elevate the meshes and populate the result attributes
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::MeshSet mesh = meshItem->value(i);
    smtk::mesh::elevate(interpolator, mesh);

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

#include "smtk/model/InterpolateMeshElevation_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::InterpolateMeshElevation,
  interpolate_mesh_elevation, "interpolate mesh elevation", InterpolateMeshElevation_xml,
  smtk::model::Session);
