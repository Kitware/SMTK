//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/moab/ModelEntityPointLocator.h"

#include "smtk/AutoInit.h"

#include "smtk/mesh/core/CellTypes.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/core/TypeSet.h"

#include "smtk/mesh/moab/HandleRangeToRange.h"
#include "smtk/mesh/moab/Interface.h"

#include "smtk/model/EntityRef.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/AdaptiveKDTree.hpp"
#include "moab/BoundBox.hpp"
#include "moab/CartVect.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <array>
#include <cmath>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

namespace smtk
{
namespace mesh
{
namespace moab
{
ModelEntityPointLocator::ModelEntityPointLocator() = default;

ModelEntityPointLocator::~ModelEntityPointLocator() = default;

bool ModelEntityPointLocator::closestPointOn(const smtk::model::EntityRef& entity,
  std::vector<double>& closestPoints, const std::vector<double>& sourcePoints, bool snapToPoint)
{
  // Attempt to access the entity's mesh tessellation
  smtk::mesh::MeshSet meshTessellation = entity.meshTessellation();

  // If the entity has a mesh tessellation, and the mesh backend is moab, and
  // the tessellation has triangles...
  if (meshTessellation.isValid() && meshTessellation.resource()->interfaceName() == "moab" &&
    meshTessellation.types().hasCell(smtk::mesh::Triangle))
  {
    //...then we can use Moab's AdaptiveKDTree to find closest points.
    const smtk::mesh::moab::InterfacePtr& interface =
      std::static_pointer_cast<smtk::mesh::moab::Interface>(
        meshTessellation.resource()->interface());

    // This option restricts the KD tree from subdividing too much
    ::moab::FileOptions treeOptions("MAX_DEPTH=13");

    // Construct an AdaptiveKDTree
    ::moab::EntityHandle treeRootSet;
    ::moab::AdaptiveKDTree tree(interface->moabInterface(),
      smtkToMOABRange(meshTessellation.cells().range()), &treeRootSet, &treeOptions);

    // Prepare the output for its points
    closestPoints.resize(sourcePoints.size());

    // For each point, query the tree for the nearest point.
    ::moab::EntityHandle triangleOut;
    for (std::size_t i = 0; i < sourcePoints.size(); i += 3)
    {
      // Identify the nearest point and the associated triangle
      tree.closest_triangle(treeRootSet, &sourcePoints[i], &closestPoints[i], triangleOut);

      // If point snapping is selected...
      if (snapToPoint)
      {
        // ...access the three vertices of the nearest triangle
        ::moab::Range connectivity;
        interface->moabInterface()->get_connectivity(&triangleOut, 1, connectivity);
        std::array<double, 9> coords;
        interface->moabInterface()->get_coords(connectivity, coords.data());

        // Compute the squared distance betwen the source point and the vertex
        std::array<double, 3> dist2 = { { 0., 0., 0. } };
        for (std::size_t j = 0; j < 3; j++)
        {
          for (std::size_t k = 0; k < 3; k++)
          {
            double tmp = coords[3 * j + k] - sourcePoints[i + k];
            dist2[j] += tmp * tmp;
          }
        }

        // Assign the closest point to the coordinates of the closest vertex
        std::size_t index =
          std::distance(dist2.begin(), std::min_element(dist2.begin(), dist2.end()));
        for (std::size_t j = 0; j < 3; j++)
        {
          closestPoints[i + j] = coords[3 * index + j];
        }
      }
    }
    return true;
  }
  return false;
}

bool ModelEntityPointLocator::randomPoint(const smtk::model::EntityRef& entity,
  const std::size_t nPoints, std::vector<double>& points, const std::size_t seed)
{
  // Select random points on an entity based on the following:
  //
  // J. A. Detwiler, R. Henning, R. A. Johnson, M. G. Marino. "A Generic Surface
  // Sampler for Monte Carlo Simulations." IEEE Trans.Nucl.Sci.55:2329-2333,2008
  // https://arxiv.org/abs/0802.2960
  //
  // Here's the ansatz:
  // 1. Compute a bounding sphere of radius R and origin O around the geometry
  // 2. Randomly select a point D on the bounding sphere
  // 3. Compute a disk of radius R through point D and tangent to the bounding
  //    sphere
  // 4. Select a random point P on this disk
  // 5. Shoot a ray from P with direction (\hat{\vec{O} - \vec{D}}) through the
  //    geometry
  // 6. If there are multiple intersections along this ray, randomly select one
  //    of the intersection points

  // Attempt to access the entity's mesh tessellation
  smtk::mesh::MeshSet meshTessellation = entity.meshTessellation();

  // If the entity has a mesh tessellation, and the mesh backend is moab, and
  // the tessellation has triangles...
  if (meshTessellation.isValid() && meshTessellation.resource()->interfaceName() == "moab" &&
    meshTessellation.types().hasCell(smtk::mesh::Triangle))
  {
    //...then we can use Moab's AdaptiveKDTree to find closest points.
    const smtk::mesh::moab::InterfacePtr& interface =
      std::static_pointer_cast<smtk::mesh::moab::Interface>(
        meshTessellation.resource()->interface());

    // This option restricts the KD tree from subdividing too much
    ::moab::FileOptions treeOptions("MAX_DEPTH=13");

    // Construct an AdaptiveKDTree
    ::moab::EntityHandle treeRootSet;
    ::moab::AdaptiveKDTree tree(interface->moabInterface(),
      smtkToMOABRange(meshTessellation.cells().range()), &treeRootSet, &treeOptions);

    // Prepare the output for its points
    points.resize(3 * nPoints);

    // Get the bounding box for the tree
    ::moab::BoundBox box;
    tree.get_bounding_box(box);

    // Get the diameter and radius of the bounding sphere for the tree
    const double diameter = box.diagonal_length();
    const double radius = diameter / 2.;

    // Get the origin of the bounding sphere for the tree
    ::moab::CartVect origin;
    box.compute_center(origin);

    // Create a RNG to sample points on the unit line
    std::mt19937 mt(static_cast<unsigned int>(seed));
    std::uniform_real_distribution<double> dist(0., 1.0);

    // Moab's ray intersection algorithm requires a tolerance. So, here it is.
    const double tolerance = 1.e-8;

    std::size_t nComputed = 0;
    while (nComputed < nPoints)
    {
      // Select a random point on the surface of our bounding sphere
      double theta = M_PI * dist(mt);
      double phi = 2. * M_PI * dist(mt);

      double sinTheta = std::sin(theta);
      double cosTheta = std::cos(theta);
      double sinPhi = std::sin(phi);
      double cosPhi = std::cos(phi);

      const std::array<double, 3> dUnit = { sinTheta * cosPhi, sinTheta * sinPhi, cosTheta };
      const std::array<double, 3> d = { radius * dUnit[0], radius * dUnit[1], radius * dUnit[2] };

      // Construct a pair of orthonormal vectors tangent to the sphere at the
      // above random point
      double sinThetaPlusPiOver2 = std::sin(theta + M_PI / 2.);
      double cosThetaPlusPiOver2 = std::cos(theta + M_PI / 2.);
      double sinPhiPlusPiOver2 = std::sin(phi + M_PI / 2.);
      double cosPhiPlusPiOver2 = std::cos(phi + M_PI / 2.);

      const std::array<double, 3> tangent1 = { sinThetaPlusPiOver2 * cosPhi,
        sinThetaPlusPiOver2 * sinPhi, cosThetaPlusPiOver2 };
      const std::array<double, 3> tangent2 = { sinTheta * cosPhiPlusPiOver2,
        sinTheta * sinPhiPlusPiOver2, cosTheta };

      // Construct a random point on a disk with radius equal to the radius of
      // our bounding sphere
      double bMag = radius * std::sqrt(dist(mt));
      double theta2 = 2. * M_PI * dist(mt);

      double sinTheta2 = std::sin(theta2);
      double cosTheta2 = std::cos(theta2);

      const std::array<double, 2> b = { bMag * cosTheta2, bMag * sinTheta2 };

      // Superimpose the second random point onto the tangent plane of our
      // bounding sphere, and offset the point according to the bounding
      // sphere's origin.
      std::array<double, 3> p = d;
      for (unsigned int i = 0; i < 3; i++)
      {
        p[i] += b[0] * tangent1[i] + b[1] * tangent2[i] + origin[i];
      }

      // Finally, the ray trajectory is simply the negative unit d vector
      std::array<double, 3> dir = { -dUnit[0], -dUnit[1], -dUnit[2] };

      // Compute the intersection of our ray and the surface
      std::vector< ::moab::EntityHandle> trianglesOut;
      std::vector<double> distanceOut;
      tree.ray_intersect_triangles(
        treeRootSet, tolerance, dir.data(), p.data(), trianglesOut, distanceOut, 0, diameter);

      if (!distanceOut.empty())
      {
        // We randomly select which intersection site to use as our sample point
        std::size_t index = static_cast<std::size_t>(distanceOut.size() * dist(mt));
        for (std::size_t i = 0; i < 3; i++)
        {
          points[3 * nComputed + i] = p[i] + distanceOut[index] * dir[i];
        }
        ++nComputed;
      }
    }

    return true;
  }
  return false;
}
}
}
}

smtkDeclareExtension(
  SMTKCORE_EXPORT, moab_model_entity_point_locator, smtk::mesh::moab::ModelEntityPointLocator);

smtkComponentInitMacro(smtk_moab_model_entity_point_locator_extension);
