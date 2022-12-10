//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/operators/ExtractByDihedralAngle.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/ForEachTypes.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/utility/Metrics.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/mesh/operators/ExtractByDihedralAngle_xml.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

namespace smtk
{
namespace mesh
{

namespace
{
typedef std::unordered_map<smtk::mesh::Handle, std::array<double, 3>> NormalsMap;

// For each cell, compute the cell's normal and add it to the map.
class ComputeNormals : public smtk::mesh::CellForEach
{
public:
  ComputeNormals(NormalsMap& normsMap)
    : smtk::mesh::CellForEach(true)
    , m_normalsMap(normsMap)
  {
  }

  std::array<double, 3> unitNormal()
  {
    const double* p0 = &(this->coordinates()[0]);
    const double* p1 = &(this->coordinates()[3]);
    const double* p2 = &(this->coordinates()[6]);

    std::array<double, 3> v1 = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
    std::array<double, 3> v2 = { p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2] };

    std::array<double, 3> n = { v1[1] * v2[2] - v1[2] * v2[1],
                                v1[2] * v2[0] - v1[0] * v2[2],
                                v1[0] * v2[1] - v1[1] * v2[0] };

    double magnitude = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    for (std::size_t i = 0; i < n.size(); i++)
    {
      n[i] /= magnitude;
    }

    return n;
  }

  void forCell(
    const smtk::mesh::Handle& cellId,
    smtk::mesh::CellType /*cellType*/,
    int /*numPointIds*/) override
  {
    m_normalsMap[cellId] = this->unitNormal();
  }

protected:
  NormalsMap& m_normalsMap;
};

// For each cell, check if the cell has a neighbor that (a) is in the normals
// map and (b) whose dihedral angle with said neighbor is less than a user-
// defined value. If so, add the cell to the list of new cells.
class ExtendByDihedralAngle : public ComputeNormals
{
public:
  ExtendByDihedralAngle(NormalsMap& normsMap, const InterfacePtr& iface, double angle)
    : ComputeNormals(normsMap)
    , m_interface(iface)
    , m_cosDihedralAngle(std::cos(M_PI * angle / 180.))
  {
  }

  smtk::mesh::HandleRange newCellHandles() const { return m_newCells; }

  void clear() { m_newCells.clear(); }

  void forCell(const smtk::mesh::Handle& cellId, smtk::mesh::CellType /*unused*/, int nPoints)
    override
  {
    (void)nPoints;
    assert(nPoints == 3);
    std::array<double, 3> normal = this->unitNormal();
    smtk::mesh::HandleRange neighborCells = m_interface->neighbors(cellId);
    for (auto i = smtk::mesh::rangeElementsBegin(neighborCells);
         i != smtk::mesh::rangeElementsEnd(neighborCells);
         ++i)
    {
      auto it = m_normalsMap.find(*i);
      if (it != m_normalsMap.end())
      {
        double dot =
          normal[0] * it->second[0] + normal[1] * it->second[1] + normal[2] * it->second[2];
        if (dot > m_cosDihedralAngle)
        {
          m_normalsMap[cellId] = normal;
          m_newCells.insert(cellId);
          break;
        }
      }
    }
  }

private:
  const InterfacePtr& m_interface;
  double m_cosDihedralAngle;
  smtk::mesh::HandleRange m_newCells;
};
} // namespace

bool ExtractByDihedralAngle::ableToOperate()
{
  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();
  smtk::mesh::Component::Ptr meshComponent = meshItem->valueAs<smtk::mesh::Component>();
  smtk::mesh::MeshSet meshset = meshComponent->mesh();

  // Ensure that we are dealing with a triangle mesh.
  CellTypes triangles;
  triangles[smtk::mesh::Triangle] = true;
  return meshset.types().cellTypes() == triangles;
}

smtk::mesh::ExtractByDihedralAngle::Result ExtractByDihedralAngle::operateInternal()
{
  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();
  smtk::mesh::Component::Ptr meshComponent = meshItem->valueAs<smtk::mesh::Component>();
  smtk::mesh::MeshSet meshset = meshComponent->mesh();

  if (!meshset.isValid())
  {
    this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  const smtk::mesh::Resource::Ptr& resource = meshset.resource();

  smtk::mesh::MeshSet surfaceMesh = resource->meshes();

  // For 2-dimensional mesh selections within a 3-dimensional mesh, we must take
  // care to restrict our algorithm to the surface mesh.
  bool shellCreated;
  if (smtk::mesh::utility::highestDimension(surfaceMesh) == smtk::mesh::Dims3)
  {
    surfaceMesh = resource->meshes().extractShell(shellCreated);
  }

  // Access the dihedral angle.
  double dihedralAngle = this->parameters()->findDouble("dihedral angle")->value();

  // The range of all cells to extract as a new meshset
  smtk::mesh::HandleRange cells = meshset.cells().range();

  // The range of cells to extract that were added during the most recent
  // iteration
  smtk::mesh::HandleRange newCells = cells;

  NormalsMap normalsMap;

  // Start by computing normals for the seed cells.
  ComputeNormals computeNormals(normalsMap);
  smtk::mesh::for_each(smtk::mesh::CellSet(resource, cells), computeNormals);

  ExtendByDihedralAngle extendByDihedralAngle(normalsMap, resource->interface(), dihedralAngle);

  do
  {
    extendByDihedralAngle.clear();

    // The cells to test for inclusion in the extraction set
    smtk::mesh::HandleRange toCheck;

    // Compute the next layer of neighbors to check for inclusion in the
    // extraction set
    for (auto i = smtk::mesh::rangeElementsBegin(newCells);
         i != smtk::mesh::rangeElementsEnd(newCells);
         ++i)
    {
      // Take the intersection of the cell's neighbors with the surface mesh.
      toCheck += (resource->interface()->neighbors(*i) & surfaceMesh.cells().range());
    }
    toCheck -= cells;

    // If there are no more cells to check, we are done
    if (toCheck.empty())
    {
      break;
    }

    // Check the next layer of neighbors
    smtk::mesh::for_each(smtk::mesh::CellSet(resource, toCheck), extendByDihedralAngle);

    // Access the newest set of cells
    newCells = extendByDihedralAngle.newCellHandles();

    // Include the newest set of cells in the overall range of cells to extract
    cells += newCells;
  } while (!newCells.empty());

  // If a shell was created to facilitate the algorithm, remove it.
  if (shellCreated)
  {
    resource->removeMeshes(surfaceMesh);
  }

  smtk::mesh::MeshSet createdMesh = resource->createMesh(smtk::mesh::CellSet(resource, cells));
  createdMesh.setName("extracted");

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  auto created = smtk::mesh::Component::create(createdMesh);
  result->findComponent("created")->appendValue(created);

  // Mark the created component as having a modified geometry so it will be
  // propertly rendered
  smtk::operation::MarkGeometry().markModified(created);

  return result;
}

const char* ExtractByDihedralAngle::xmlDescription() const
{
  return ExtractByDihedralAngle_xml;
}

} //namespace mesh
} // namespace smtk
