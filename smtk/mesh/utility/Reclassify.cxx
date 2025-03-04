//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/utility/Reclassify.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/Interface.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/PointSet.h"
#include "smtk/mesh/core/Resource.h"

namespace
{
class ContainedInNewEdge : public smtk::mesh::CellForEach
{
public:
  ContainedInNewEdge(smtk::model::Edge edge)
    : CellForEach(true)

  {
    //Find the tessellation of the edge and store the points
    //in a search-able structure.
    //when we are iterating the cells we need to quickly determine
    //if the points are contained inside.
    //for now we are going to use a linear search :(
    const smtk::model::Tessellation* tess = edge.hasTessellation();
    if (tess)
    {
      const std::vector<double>& c = tess->coords();
      const std::size_t size = c.size();
      const std::size_t perSize = size / 3;

      m_Xpoints.reserve(perSize);
      m_Ypoints.reserve(perSize);
      m_Zpoints.reserve(perSize);

      for (std::size_t i = 0; i < size; i += 3)
      {
        m_Xpoints.push_back(c[i]);
        m_Ypoints.push_back(c[i + 1]);
        m_Zpoints.push_back(c[i + 2]);
      }
    }
  }

  bool contains(double x, double y, double z)
  {
    //determine if the given point is contained in this
    //edges tessellation
    bool found = false;
    std::size_t size = m_Xpoints.size();
    for (std::size_t i = 0; i < size && !found; ++i)
    {
      //Check on the X before doing an assignment to found
      if (m_Xpoints[i] == x)
      {
        found = (m_Ypoints[i] == y && m_Zpoints[i] == z);
      }
    }
    return found;
  }

  void forCell(
    const smtk::mesh::Handle& cellId,
    smtk::mesh::CellType /*cellType*/,
    int /*numPointIds*/) override
  {
    bool contained = true;
    //see if the coordinates for this cell are all contained
    //in the set of points. We need to build some form search
    //otherwise we are looking at a linear search
    const std::vector<double>& coords = this->coordinates();
    const std::size_t size = coords.size();
    for (std::size_t i = 0; i < size && contained; i += 3)
    {
      contained = this->contains(coords[i], coords[i + 1], coords[i + 2]);
    }

    if (contained)
    {
      m_cells.insert(cellId);
    }
  }

  [[nodiscard]] smtk::mesh::HandleRange cells() const { return m_cells; }

private:
  smtk::mesh::HandleRange m_cells;
  std::vector<double> m_Xpoints;
  std::vector<double> m_Ypoints;
  std::vector<double> m_Zpoints;
};

smtk::mesh::MeshSet make_MeshPoint(smtk::mesh::ResourcePtr resource, smtk::model::Vertex vertex)
{
  //What are the steps that are required here:
  //1. Allocate a single point
  //2. Make a HandleRange of the single point Id
  //3. Make a Mesh of that HandleRange
  //4. assign an association to that new mesh set
  smtk::mesh::InterfacePtr interface = resource->interface();
  smtk::mesh::AllocatorPtr allocator = interface->allocator();
  const smtk::model::Tessellation* tess = vertex.hasTessellation();

  smtk::mesh::MeshSet result;
  if (tess && allocator)
  {
    const std::vector<double>& c = tess->coords();

    smtk::mesh::Handle vertexHandle;
    std::vector<double*> coords;
    allocator->allocatePoints(1, vertexHandle, coords);

    coords[0][0] = c[0];
    coords[1][0] = c[1];
    coords[2][0] = c[2];

    smtk::mesh::HandleRange meshCells;
    meshCells.insert(vertexHandle);

    smtk::mesh::CellSet cellsForMesh(resource, meshCells);
    result = resource->createMesh(cellsForMesh);
    if (!result.is_empty())
    {
      resource->setAssociation(vertex, result);
    }
  }

  return result;
}
} // namespace

namespace smtk
{
namespace mesh
{
namespace utility
{

bool split(
  smtk::mesh::ResourcePtr resource,
  smtk::model::Edge orignalEdge,
  smtk::model::Edge newEdge,
  smtk::model::Vertex promotedVertex)
{
  if (!resource)
  {
    return false;
  }

  if (!orignalEdge.hasTessellation() || !newEdge.hasTessellation())
  {
    return false;
  } //both edges need tessellation for this operator to work

  //1. find all meshes associated to original edge
  smtk::mesh::MeshSet origMesh = resource->findAssociatedMeshes(orignalEdge);
  if (origMesh.is_empty())
  {
    return false;
  }

  //1:
  //Iterate the cells of the original meshset, for each cell
  //check if it contained in the new-edge. If so that cell needs
  //to be reclassified.
  ContainedInNewEdge functor(newEdge);
  smtk::mesh::for_each(origMesh.cells(), functor);

  //2:
  //get the cells from the original set that should be reclassified
  smtk::mesh::CellSet newEdgeCells(resource, functor.cells());
  if (newEdgeCells.is_empty())
  { //the split new edge is invalid since it had nothing in common with
    //the original edge
    return false;
  }

  //3 create a new mesh set for the vertex
  smtk::mesh::MeshSet pointMesh = make_MeshPoint(resource, promotedVertex);
  if (pointMesh.is_empty())
  { //for some reason we could not allocate a new mesh
    return false;
  }

  //4 Make sure the new meshes are properly disjoint
  smtk::mesh::MeshSet newMesh = resource->createMesh(newEdgeCells);
  resource->setAssociation(newEdge, newMesh);

  bool ret = make_disjoint(resource, newMesh, origMesh, orignalEdge);
  if (!ret)
  { //if the split failed rollback the creation of the new mesh
    resource->removeMeshes(newMesh);
  }

  //5 Lastly make sure we don't have any duplicate points in the underlying
  //database
  smtk::mesh::MeshSet all = newMesh;
  all.append(origMesh);
  all.append(pointMesh);
  all.mergeCoincidentContactPoints();

  return ret;
}

bool make_disjoint(
  smtk::mesh::ResourcePtr resource,
  const smtk::mesh::MeshSet& toBeRemoved,
  smtk::mesh::MeshSet& removeFrom,
  const smtk::model::EntityRef& modelAssoc)
{

  //Add a new MeshSet which contains all the original cells, expect the ones
  //that are in the new edge mesh set.
  smtk::mesh::CellSet diff = smtk::mesh::set_difference(removeFrom.cells(), toBeRemoved.cells());

  if (!diff.is_empty())
  {
    smtk::mesh::MeshSet updatedOriginalMesh = resource->createMesh(diff);
    //Remove the original meshset so we don't have the original, and new
    //representation existing
    const bool success = resource->removeMeshes(removeFrom);
    if (!success)
    {
      //rollback if the the mesh removal fails
      resource->removeMeshes(updatedOriginalMesh);
      return false;
    }
    removeFrom = updatedOriginalMesh;
  }

  resource->setAssociation(modelAssoc, removeFrom);
  return true;
}

bool merge(
  smtk::mesh::ResourcePtr resource,
  smtk::model::Vertex toRemoveVert,
  smtk::model::Edge toRemoveEdge,
  smtk::model::Edge toAddTo)
{
  smtk::mesh::MeshSet vertexToRemoveMS = resource->findAssociatedMeshes(toRemoveVert);
  smtk::mesh::MeshSet edgeToRemoveMS = resource->findAssociatedMeshes(toRemoveEdge);
  smtk::mesh::MeshSet toAddToMS = resource->findAssociatedMeshes(toAddTo);
  const smtk::model::EntityRef& eref = toAddTo;

  //first we need to delete the vertex, that way if the delete fails, we
  //can rollback properly. If we merge the edge and vertex and delete them
  //in the fuse call, the vertex mesh items will get the edge model association
  //if we have to rollback the delete
  bool merged = resource->removeMeshes(vertexToRemoveMS);
  if (merged)
  {
    return fuse(resource, edgeToRemoveMS, toAddToMS, eref);
  }
  else
  {
    //make sure the association is restored, since the delete failed
    //it is better to be safe than sorry
    resource->setAssociation(toRemoveVert, vertexToRemoveMS);
  }
  return false;
}

bool fuse(
  smtk::mesh::ResourcePtr resource,
  smtk::mesh::MeshSet& toRemove,
  smtk::mesh::MeshSet& toAddTo,
  const smtk::model::EntityRef& toAddToAssoc)
{
  //Merge two mesh sets together and create a single meshset from that
  //
  //The logic of the fuse is slightly more complicated than expected because
  //you want to handle the ability to rollback a failed removal properly.
  //
  //Secondly you want to add the new mesh before removing the old mesh,
  //that signals to the underlying interface that the cells of the
  //deleted mesh are used by another meshset and shouldn't be deleted,
  //but just delete the meshset

  smtk::mesh::CellSet newSetCells = toAddTo.cells();
  newSetCells.append(toRemove.cells());
  if (newSetCells.is_empty())
  {
    return false;
  }

  smtk::mesh::MeshSet newSet = resource->createMesh(newSetCells);
  if (newSet.is_empty())
  {
    return false;
  }

  bool removed = resource->removeMeshes(toAddTo);
  if (!removed)
  {
    resource->removeMeshes(newSet);
    return false;
  }

  removed = removed && resource->removeMeshes(toRemove);
  if (!removed)
  {
    //rollback the deletion of the first meshset
    toAddTo = resource->createMesh(toAddTo.cells());
    resource->setAssociation(toAddToAssoc, toAddTo);

    resource->removeMeshes(newSet);
    return false;
  }

  //update what toAddTo is point too
  toAddTo = newSet;
  resource->setAssociation(toAddToAssoc, toAddTo);
  return true;
}
} // namespace utility
} // namespace mesh
} // namespace smtk
