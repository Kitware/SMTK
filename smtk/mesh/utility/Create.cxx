//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/utility/Create.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/Interface.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/PointSet.h"
#include "smtk/mesh/core/Resource.h"

namespace smtk
{
namespace mesh
{
namespace utility
{

std::array<smtk::mesh::MeshSet, 7> createUniformGrid(
  smtk::mesh::ResourcePtr resource,
  const std::array<std::size_t, 3>& discretization,
  const std::function<std::array<double, 3>(std::array<double, 3>)>& transform)
{
  // We start by constructing a mesh allocator object.
  smtk::mesh::AllocatorPtr allocator = resource->interface()->allocator();

  // Allocate the memory for our points. The resulting memory layout is the
  // 3 x N array "coordinates".
  std::size_t numberOfPoints =
    (discretization[0] + 1) * (discretization[1] + 1) * (discretization[2] + 1);
  smtk::mesh::Handle verticesBegin = 0;
  std::vector<double*> coordinates;
  allocator->allocatePoints(numberOfPoints, verticesBegin, coordinates);

  // Next, allocate the memory for our 3D cells.
  std::size_t numberOfCells = discretization[0] * discretization[1] * discretization[2];
  smtk::mesh::HandleRange createdCellIds;
  smtk::mesh::Handle* connectivity;
  allocator->allocateCells<smtk::mesh::Hexahedron>(numberOfCells, createdCellIds, connectivity);

  // Finally, allocate the memory for our 2D faces. We could have the mesh
  // interface construct these programmatically by accessing the shell of the
  // created 3D mesh and filtering based on the orientation of the unit
  // normals or point coordinates, but it is pretty easy to manually construct
  // the faces of a rectilinear grid. Also, this approach reduces the number of
  // computations we need to make.
  std::array<std::size_t, 6> numberOfCellsForFaces;
  std::array<smtk::mesh::HandleRange, 6> createdCellIdsForFaces;
  std::array<smtk::mesh::Handle*, 6> connectivityForFaces;
  std::array<std::size_t, 6> connectivityIndexForFaces;
  for (int i = 0; i < 6; i++)
  {
    numberOfCellsForFaces[i] = discretization[(i + 1) % 3] * discretization[(i + 2) % 3];
    allocator->allocateCells<smtk::mesh::Quad>(
      numberOfCellsForFaces[i], createdCellIdsForFaces[i], connectivityForFaces[i]);
    connectivityIndexForFaces[i] = 0;
  }

  // Map to convert hexahedral indices to quad indices for identifying the
  // faces of a hexahedron as x-min, y-min, z-min, x-max, y-max, z-max.
  static const std::size_t hexToQuad[6][4] = { { 0, 3, 2, 1 }, { 0, 1, 5, 4 }, { 0, 4, 7, 3 },
                                               { 4, 5, 6, 7 }, { 3, 7, 6, 2 }, { 1, 2, 6, 5 } };
  std::size_t coordinateCounter = 0;
  std::size_t cellCounter = 0;
  std::array<double, 3> xyz;
  std::array<double, 3> transformed;
  std::array<double, 3> step = {
    { 1. / discretization[0], 1. / discretization[1], 1. / discretization[2] }
  };
  for (std::size_t i = 0; i <= discretization[0]; i++)
  {
    xyz[0] = i * step[0];
    for (std::size_t j = 0; j <= discretization[1]; j++)
    {
      xyz[1] = j * step[1];
      for (std::size_t k = 0; k <= discretization[2]; k++)
      {
        xyz[2] = k * step[2];

        transformed = transform(xyz);

        coordinates[0][coordinateCounter] = transformed[0];
        coordinates[1][coordinateCounter] = transformed[1];
        coordinates[2][coordinateCounter] = transformed[2];

        // We are looping over the point coordinates. They run from 0 through
        // N, where N is the number of cells on a side. We can use the same
        // loop to construct our cell connectivities by skipping the final
        // iteration along each principal axis.
        if (i != discretization[0] && j != discretization[1] && k != discretization[2])
        {
          std::size_t connectivityIndex =
            smtk::mesh::CellTraits<smtk::mesh::CellHexahedron>::NUM_VERTICES * cellCounter;
          std::size_t firstVert = verticesBegin + coordinateCounter;
          connectivity[connectivityIndex + 0] = firstVert;
          connectivity[connectivityIndex + 1] = firstVert + 1;
          connectivity[connectivityIndex + 2] = firstVert + discretization[2] + 2;
          connectivity[connectivityIndex + 3] = firstVert + discretization[2] + 1;
          std::size_t tmp = (discretization[1] + 1) * (discretization[2] + 1);
          connectivity[connectivityIndex + 4] = firstVert + tmp;
          connectivity[connectivityIndex + 5] = firstVert + tmp + 1;
          connectivity[connectivityIndex + 6] = firstVert + discretization[2] + tmp + 2;
          connectivity[connectivityIndex + 7] = firstVert + discretization[2] + tmp + 1;

#define ASSIGN_CONNECTIVITY_FOR_FACE(faceId)                                                       \
  do                                                                                               \
  {                                                                                                \
    for (std::size_t m = 0; m < smtk::mesh::CellTraits<smtk::mesh::CellQuad>::NUM_VERTICES; m++)   \
    {                                                                                              \
      connectivityForFaces[faceId][connectivityIndexForFaces[faceId]++] =                          \
        connectivity[connectivityIndex + hexToQuad[faceId][m]];                                    \
    }                                                                                              \
  } while (0)

          if (i == 0)
            ASSIGN_CONNECTIVITY_FOR_FACE(0);
          if (j == 0)
            ASSIGN_CONNECTIVITY_FOR_FACE(1);
          if (k == 0)
            ASSIGN_CONNECTIVITY_FOR_FACE(2);
          if (i == (discretization[0] - 1))
            ASSIGN_CONNECTIVITY_FOR_FACE(3);
          if (j == (discretization[1] - 1))
            ASSIGN_CONNECTIVITY_FOR_FACE(4);
          if (k == (discretization[2] - 1))
            ASSIGN_CONNECTIVITY_FOR_FACE(5);
#undef ASSIGN_CONNECTIVITY_FOR_FACE

          ++cellCounter;
        }
        ++coordinateCounter;
      }
    }
  }

  std::array<smtk::mesh::MeshSet, 7> constructedMeshSets;

  // We have filled in the connectivity array, so we must now notify the
  // allocator that we have modified the connectivity array.
  allocator->connectivityModified(
    createdCellIds, smtk::mesh::CellTraits<smtk::mesh::CellHexahedron>::NUM_VERTICES, connectivity);

  // Construct a cell set corresponding to the hexahedral cells identified by
  // the connectivity array we just created.
  smtk::mesh::CellSet cellsForMesh(resource, createdCellIds);

  // Construct a mesh set corresponding to the cell set we just created.
  constructedMeshSets[0] = resource->createMesh(cellsForMesh);

  for (std::size_t i = 0; i < 6; i++)
  {
    // We have filled in the connectivity array, so we must now notify the
    // allocator that we have modified the connectivity array.
    allocator->connectivityModified(
      createdCellIdsForFaces[i],
      smtk::mesh::CellTraits<smtk::mesh::CellQuad>::NUM_VERTICES,
      connectivityForFaces[i]);

    // Construct a cell set corresponding to the quadrilateral cells identified
    // by the connectivity array we just created.
    smtk::mesh::CellSet cellsForMeshFace(resource, createdCellIdsForFaces[i]);

    // Construct a mesh set corresponding to the cell set we just created.
    constructedMeshSets[i + 1] = resource->createMesh(cellsForMeshFace);
  }

  return constructedMeshSets;
}

std::array<smtk::mesh::MeshSet, 5> createUniformGrid(
  smtk::mesh::ResourcePtr resource,
  const std::array<std::size_t, 2>& discretization,
  const std::function<std::array<double, 3>(std::array<double, 3>)>& transform)
{
  // We start by constructing a mesh allocator object.
  smtk::mesh::AllocatorPtr allocator = resource->interface()->allocator();

  // Allocate the memory for our points. The resulting memory layout is the
  // 3 x N array "coordinates".
  std::size_t numberOfPoints = (discretization[0] + 1) * (discretization[1] + 1);
  smtk::mesh::Handle verticesBegin = 0;
  std::vector<double*> coordinates;
  allocator->allocatePoints(numberOfPoints, verticesBegin, coordinates);

  // Next, allocate the memory for our 2D cells.
  std::size_t numberOfCells = discretization[0] * discretization[1];
  smtk::mesh::HandleRange createdCellIds;
  smtk::mesh::Handle* connectivity;
  allocator->allocateCells<smtk::mesh::Quad>(numberOfCells, createdCellIds, connectivity);

  // Finally, allocate the memory for our 1D edges. We could have the mesh
  // interface construct these programmatically by accessing the shell of the
  // created 2D mesh and filtering based on the orientation of the unit
  // normals or point coordinates, but it is pretty easy to manually construct
  // the faces of a rectilinear grid. Also, this approach reduces the number of
  // computations we need to make.
  std::array<std::size_t, 4> numberOfCellsForEdges;
  std::array<smtk::mesh::HandleRange, 4> createdCellIdsForEdges;
  std::array<smtk::mesh::Handle*, 4> connectivityForEdges;
  std::array<std::size_t, 4> connectivityIndexForEdges;
  for (std::size_t i = 0; i < 4; i++)
  {
    numberOfCellsForEdges[i] = discretization[(i + 1) % 2];
    allocator->allocateCells<smtk::mesh::Line>(
      numberOfCellsForEdges[i], createdCellIdsForEdges[i], connectivityForEdges[i]);
    connectivityIndexForEdges[i] = 0;
  }

  // Map to convert quadrilateral indices to line indices for identifying the
  // edges of a quadrilateral as x-min, y-min, x-max, y-max.
  static const std::size_t quadToLine[4][2] = { { 0, 1 }, { 3, 0 }, { 2, 3 }, { 1, 2 } };
  std::size_t coordinateCounter = 0;
  std::size_t cellCounter = 0;
  std::array<double, 3> xyz = { { 0., 0., 0. } };
  std::array<double, 3> transformed;
  std::array<double, 2> step = { { 1. / discretization[0], 1. / discretization[1] } };
  for (std::size_t i = 0; i <= discretization[0]; i++)
  {
    xyz[0] = i * step[0];
    for (std::size_t j = 0; j <= discretization[1]; j++)
    {
      xyz[1] = j * step[1];

      transformed = transform(xyz);

      coordinates[0][coordinateCounter] = transformed[0];
      coordinates[1][coordinateCounter] = transformed[1];
      coordinates[2][coordinateCounter] = transformed[2];

      // We are looping over the point coordinates. They run from 0 through
      // N, where N is the number of cells on a side. We can use the same
      // loop to construct our cell connectivities by skipping the final
      // iteration along each principal axis.
      if (i != discretization[0] && j != discretization[1])
      {
        std::size_t connectivityIndex =
          smtk::mesh::CellTraits<smtk::mesh::CellQuad>::NUM_VERTICES * cellCounter;
        std::size_t firstVert = verticesBegin + coordinateCounter;
        connectivity[connectivityIndex + 0] = firstVert;
        connectivity[connectivityIndex + 1] = firstVert + 1;
        connectivity[connectivityIndex + 2] = firstVert + discretization[1] + 2;
        connectivity[connectivityIndex + 3] = firstVert + discretization[1] + 1;

#define ASSIGN_CONNECTIVITY_FOR_EDGE(edgeId)                                                       \
  do                                                                                               \
  {                                                                                                \
    for (std::size_t m = 0; m < smtk::mesh::CellTraits<smtk::mesh::CellLine>::NUM_VERTICES; m++)   \
    {                                                                                              \
      connectivityForEdges[edgeId][connectivityIndexForEdges[edgeId]++] =                          \
        connectivity[connectivityIndex + quadToLine[edgeId][m]];                                   \
    }                                                                                              \
  } while (0)

        if (i == 0)
          ASSIGN_CONNECTIVITY_FOR_EDGE(0);
        if (j == 0)
          ASSIGN_CONNECTIVITY_FOR_EDGE(1);
        if (i == (discretization[0] - 1))
          ASSIGN_CONNECTIVITY_FOR_EDGE(2);
        if (j == (discretization[1] - 1))
          ASSIGN_CONNECTIVITY_FOR_EDGE(3);
#undef ASSIGN_CONNECTIVITY_FOR_EDGE

        ++cellCounter;
      }
      ++coordinateCounter;
    }
  }

  std::array<smtk::mesh::MeshSet, 5> constructedMeshSets;

  // We have filled in the connectivity array, so we must now notify the
  // allocator that we have modified the connectivity array.
  allocator->connectivityModified(
    createdCellIds, smtk::mesh::CellTraits<smtk::mesh::CellQuad>::NUM_VERTICES, connectivity);

  // Construct a cell set corresponding to the quadrilateral cells identified by
  // the connectivity array we just created.
  smtk::mesh::CellSet cellsForMesh(resource, createdCellIds);

  // Construct a mesh set corresponding to the cell set we just created.
  constructedMeshSets[0] = resource->createMesh(cellsForMesh);

  for (std::size_t i = 0; i < 4; i++)
  {
    // We have filled in the connectivity array, so we must now notify the
    // allocator that we have modified the connectivity array.
    allocator->connectivityModified(
      createdCellIdsForEdges[i],
      smtk::mesh::CellTraits<smtk::mesh::CellLine>::NUM_VERTICES,
      connectivityForEdges[i]);

    // Construct a cell set corresponding to the line cells identified by the
    // connectivity array we just created.
    smtk::mesh::CellSet cellsForMeshFace(resource, createdCellIdsForEdges[i]);

    // Construct a mesh set corresponding to the cell set we just created.
    constructedMeshSets[i + 1] = resource->createMesh(cellsForMeshFace);
  }

  return constructedMeshSets;
}
} // namespace utility
} // namespace mesh
} // namespace smtk
