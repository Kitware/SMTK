//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/operators/Transform.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/utility/ApplyToMesh.h"

#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/mesh/operators/Transform_xml.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

namespace smtk
{
namespace mesh
{

smtk::mesh::Transform::Result Transform::operateInternal()
{
  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();

  // Access the scale parameters
  smtk::attribute::DoubleItem::Ptr scaleItem = this->parameters()->findDouble("scale");
  std::array<double, 3> scale = { scaleItem->value(0), scaleItem->value(1), scaleItem->value(2) };

  // Access the rotate parameters
  smtk::attribute::DoubleItem::Ptr rotateItem = this->parameters()->findDouble("rotate");
  std::array<double, 3> rotate = { rotateItem->value(0),
                                   rotateItem->value(1),
                                   rotateItem->value(2) };
  double cosTheta = cos(rotate[0] * M_PI / 180.);
  double sinTheta = sin(rotate[0] * M_PI / 180.);
  double cosPhi = cos(rotate[1] * M_PI / 180.);
  double sinPhi = sin(rotate[1] * M_PI / 180.);
  double cosPsi = cos(rotate[2] * M_PI / 180.);
  double sinPsi = sin(rotate[2] * M_PI / 180.);

  // Access the translate parameters
  smtk::attribute::DoubleItem::Ptr translateItem = this->parameters()->findDouble("translate");
  std::array<double, 3> translate = { translateItem->value(0),
                                      translateItem->value(1),
                                      translateItem->value(2) };

  auto transformFn = [=](std::array<double, 3> p) {
    for (std::size_t i = 0; i < 3; i++)
    {
      p[i] *= scale[i];
    }

    // From https://en.wikipedia.org/wiki/Euler_angles#Intrinsic_rotations :
    // VTK uses Tait-Bryan Y_1 X_2 Z_3 angles to store orientation;
    // This is the corresponding direction cosine matrix (DCM) for
    // theta = X, phi = Y, psi = Z:

    std::array<double, 3> tmp = { 0., 0., 0. };

    std::array<std::array<double, 3>, 3> R_x = {
      { { 1., 0., 0. }, { 0., cosTheta, -sinTheta }, { 0., sinTheta, cosTheta } }
    };
    for (std::size_t i = 0; i < 3; i++)
    {
      for (std::size_t j = 0; j < 3; j++)
      {
        tmp[i] += R_x[i][j] * p[j];
      }
    }
    p = tmp;
    tmp = { 0., 0., 0. };

    std::array<std::array<double, 3>, 3> R_y = {
      { { cosPhi, 0., sinPhi }, { 0., 1., 0. }, { -sinPhi, 0., cosPhi } }
    };
    for (std::size_t i = 0; i < 3; i++)
    {
      for (std::size_t j = 0; j < 3; j++)
      {
        tmp[i] += R_y[i][j] * p[j];
      }
    }
    p = tmp;
    tmp = { 0., 0., 0. };

    std::array<std::array<double, 3>, 3> R_z = {
      { { cosPsi, -sinPsi, 0. }, { sinPsi, cosPsi, 0. }, { 0., 0., 1. } }
    };
    for (std::size_t i = 0; i < 3; i++)
    {
      for (std::size_t j = 0; j < 3; j++)
      {
        tmp[i] += R_z[i][j] * p[j];
      }
    }
    p = tmp;
    tmp = { 0., 0., 0. };

    for (std::size_t i = 0; i < 3; i++)
    {
      p[i] += translate[i];
    }

    return p;
  };

  // Access the attribute associated with the modified meshes
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Access the attribute associated with the modified model
  smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");

  // apply the interpolator to the meshes and populate the result attributes
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::Component::Ptr meshComponent = meshItem->valueAs<smtk::mesh::Component>(i);
    smtk::mesh::MeshSet mesh = meshComponent->mesh();

    bool success = smtk::mesh::utility::applyWarp(transformFn, mesh);

    if (!success)
    {
      smtkErrorMacro(this->log(), "Transform failed.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }

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

const char* Transform::xmlDescription() const
{
  return Transform_xml;
}

} //namespace mesh
} // namespace smtk
