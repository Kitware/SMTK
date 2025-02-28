//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/moab/RandomPoint.h"

#include "smtk/mesh/core/CellTypes.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/moab/HandleRangeToRange.h"
#include "smtk/mesh/moab/Interface.h"
#include "smtk/mesh/moab/PointLocatorCache.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/AdaptiveKDTree.hpp"
#include "moab/BoundBox.hpp"
#include "moab/CartVect.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <cmath>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

namespace
{
struct MersenneTwisterCache : public smtk::resource::query::Cache
{
  MersenneTwisterCache()
    : mt(0)
  {
  }
  std::size_t seed{ 0 };
  std::mt19937 mt;
};
} // namespace

namespace smtk
{
namespace mesh
{
namespace moab
{
std::array<double, 3> RandomPoint::operator()(const smtk::resource::Component::Ptr& component) const
{
  auto meshComponent = std::dynamic_pointer_cast<smtk::mesh::Component>(component);
  if (meshComponent)
  {
    return operator()(meshComponent->mesh());
  }

  auto modelComponent = std::dynamic_pointer_cast<smtk::model::Entity>(component);
  if (modelComponent)
  {
    return operator()(modelComponent->referenceAs<smtk::model::EntityRef>().meshTessellation());
  }

  return this->Parent::operator()(component);
}

std::array<double, 3> RandomPoint::operator()(const smtk::mesh::MeshSet& meshset) const
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

  static constexpr const double nan = std::numeric_limits<double>::quiet_NaN();
  std::array<double, 3> returnValue{ { nan, nan, nan } };

  // If the entity has a mesh tessellation, and the mesh backend is moab, and
  // the tessellation has triangles...
  if (
    meshset.isValid() && meshset.resource()->interfaceName() == "moab" &&
    meshset.types().hasCell(smtk::mesh::Triangle))
  {
    //...then we can use Moab's AdaptiveKDTree to find closest points.
    const smtk::mesh::moab::InterfacePtr& interface =
      std::static_pointer_cast<smtk::mesh::moab::Interface>(meshset.resource()->interface());

    PointLocatorCache& pointLocatorCache = meshset.resource()->queries().cache<PointLocatorCache>();

    auto search = pointLocatorCache.m_caches.find(meshset.id());
    if (search == pointLocatorCache.m_caches.end())
    {
      // This option restricts the KD tree from subdividing too much
      ::moab::FileOptions treeOptions("MAX_DEPTH=13");

      search =
        pointLocatorCache.m_caches
          .emplace(
            meshset.id(),
            std::unique_ptr<PointLocatorCache::CacheForIndex>(new PointLocatorCache::CacheForIndex(
              interface->moabInterface(), smtkToMOABRange(meshset.cells().range()), &treeOptions)))
          .first;
    }

    ::moab::AdaptiveKDTree& tree = search->second->m_tree;

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
    MersenneTwisterCache& mersenneTwisterCache =
      meshset.resource()->queries().cache<MersenneTwisterCache>();
    std::mt19937& mt = mersenneTwisterCache.mt;
    if (m_seed != mersenneTwisterCache.seed)
    {
      mersenneTwisterCache.seed = m_seed;
      mersenneTwisterCache.mt.seed(static_cast<unsigned int>(m_seed));
    }

    std::uniform_real_distribution<double> dist(0., 1.0);

    // Moab's ray intersection algorithm requires a tolerance. So, here it is.
    const double tolerance = 1.e-8;

    bool computed = false;
    do
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
                                               sinThetaPlusPiOver2 * sinPhi,
                                               cosThetaPlusPiOver2 };
      const std::array<double, 3> tangent2 = { sinTheta * cosPhiPlusPiOver2,
                                               sinTheta * sinPhiPlusPiOver2,
                                               cosTheta };

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
      std::vector<::moab::EntityHandle> trianglesOut;
      std::vector<double> distanceOut;
      tree.ray_intersect_triangles(
        search->second->m_treeRootSet,
        tolerance,
        dir.data(),
        p.data(),
        trianglesOut,
        distanceOut,
        0,
        diameter);

      if (!distanceOut.empty())
      {
        // We randomly select which intersection site to use as our sample point
        std::size_t index = static_cast<std::size_t>(distanceOut.size() * dist(mt));
        for (std::size_t i = 0; i < 3; i++)
        {
          returnValue[i] = p[i] + distanceOut[index] * dir[i];
        }
        computed = true;
      }
    } while (!computed);
  }
  return returnValue;
}
} // namespace moab
} // namespace mesh
} // namespace smtk
