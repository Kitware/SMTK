//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/mesh/json/Interface.h"
#include "smtk/mesh/moab/Interface.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{
  double pts[9][3] = {{0.,0.,0.}, {1.,0.,0.}, {1.,1.,0.}, {0.,1.,0.},
                      {0.,0.,1.}, {1.,0.,1.}, {1.,1.,1.}, {0.,1.,1.},
                      {.5,-.5,0.}};

  double* vertex[1]      = {pts[0]};
  double* line[2]        = {pts[0], pts[1]};
  double* triangle[3]    = {pts[0], pts[1], pts[2]};
  double* quad[4]        = {pts[0], pts[1], pts[2], pts[3]};
  double* polygon[5]     = {pts[0], pts[1], pts[2], pts[3], pts[8]};
  double* tetrahedron[4] = {pts[0], pts[1], pts[2], pts[4]};
  double* pyramid[5]     = {pts[0], pts[1], pts[2], pts[3], pts[4]};
  double* wedge[6]       = {pts[0], pts[1], pts[2], pts[4], pts[5], pts[6]};
  double* hexahedron[8]  = {pts[0], pts[1], pts[2], pts[3], pts[4], pts[5],
                            pts[6], pts[7]};

  double** cellPoints[9]   = {vertex, line, triangle, quad, polygon,
                              tetrahedron, pyramid, wedge, hexahedron};

//----------------------------------------------------------------------------
void verify_moab_bufferred_cell_allocator_creation()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::InterfacePtr iface = smtk::mesh::moab::make_interface();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection(iface);

  test( collection->isValid(), "collection should be valid");
  test( !collection->isModified(), "collection shouldn't be modified");

  //at this point extract the allocator from json and verify that it
  //is NOT null
  smtk::mesh::BufferedCellAllocatorPtr allocator =
    collection->interface()->bufferedCellAllocator();
  test( !!allocator, "moab buffered cell allocator should be valid");

  //verify that is modified is true
  test( collection->isModified(), "collection should be modified once the buffered cell allocator is accessed");
}

//----------------------------------------------------------------------------
void verify_json_bufferred_cell_allocator_creation()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::InterfacePtr iface = smtk::mesh::json::make_interface();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection(iface);

  test( collection->isValid(), "collection should be valid");
  test( !collection->isModified(), "collection shouldn't be modified");

  //at this point extract the allocator from json and verify that it
  //is null
  smtk::mesh::BufferedCellAllocatorPtr allocator =
    collection->interface()->bufferedCellAllocator();
  test( !allocator, "json incremental allocator should be NULL");

  //verify that is modified is true
  test( !collection->isModified(), "collection shouldn't be modified");

}

//----------------------------------------------------------------------------
void verify_moab_bufferred_cell_allocator_cell(smtk::mesh::CellType cellType)
{
  // Allocate a cell of type <cellType>.

  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::InterfacePtr iface = smtk::mesh::moab::make_interface();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection(iface);

  test( collection->isValid(), "collection should be valid");
  test( !collection->isModified(), "collection shouldn't be modified");

  smtk::mesh::BufferedCellAllocatorPtr allocator =
    collection->interface()->bufferedCellAllocator();

  // Grab the number of vertices for the cell type being tested
  std::size_t nVerticesPerCell = (cellType == smtk::mesh::Polygon ? 5 :
                                  smtk::mesh::verticesPerCell(cellType));

  test(allocator->isValid() == false);

  // Reserve the vertices
  test(allocator->reserveNumberOfCoordinates(nVerticesPerCell));

  test(allocator->isValid() == true);

  std::vector<int> connectivity(nVerticesPerCell);
  // fill the vertices and remember the connectivity
  for (std::size_t i = 0; i < nVerticesPerCell; i++)
    {
    test(allocator->addCoordinate(i, cellPoints[cellType][i]));
    connectivity[i] = i;
    }

  // Add a cell using the cell type and connectivity
  test(allocator->addCell(cellType, &connectivity[0], nVerticesPerCell));

  // Finalize the addition of cells
  test(allocator->flush());

  test(allocator->cells().size() == 1);

  smtk::mesh::MeshSet mesh =
    collection->createMesh(smtk::mesh::CellSet(collection, allocator->cells()));

  test(mesh.points().size() == nVerticesPerCell);
}

//----------------------------------------------------------------------------
void verify_moab_bufferred_cell_allocator_validity(smtk::mesh::CellType cellType)
{
  // Allocate a cell of type <cellType>, ensuring that the allocator returns the
  // proper success and validity variables along the way.

  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::InterfacePtr iface = smtk::mesh::moab::make_interface();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection(iface);

  test( collection->isValid(), "collection should be valid");
  test( !collection->isModified(), "collection shouldn't be modified");

  smtk::mesh::BufferedCellAllocatorPtr allocator =
    collection->interface()->bufferedCellAllocator();
  test(allocator->isValid() == false);
  test(allocator->cells().size() == 0);

  // Try to add a coordinate before allocating for it (should fail)
  test(allocator->addCoordinate(0, cellPoints[cellType][0]) == false);
  test(allocator->isValid() == false);

  // Flush before initializing anything (should fail)
  test(allocator->flush() == false);
  test(allocator->isValid() == false);

  // Grab the number of vertices for the cell type being tested
  std::size_t nVerticesPerCell = (cellType == smtk::mesh::Polygon ? 5 :
                                  smtk::mesh::verticesPerCell(cellType));

  // Try to add a cell before allocating vertices (should fail)
  std::vector<int> connectivity(nVerticesPerCell);
  for (std::size_t i = 0; i < nVerticesPerCell; i++)
    {
    connectivity[i] = i;
    }
  test(allocator->addCell(cellType, &connectivity[0],
                          nVerticesPerCell) == false);
  test(allocator->isValid() == false);

  // Reserve the coordinates (should succeed)
  test(allocator->reserveNumberOfCoordinates(nVerticesPerCell) == true);
  test(allocator->isValid() == true);

  // Reserve coordinates again (should fail but not alter the validity of the
  // allocator)
  test(allocator->reserveNumberOfCoordinates(nVerticesPerCell + 1) == false);
  test(allocator->isValid() == true);

  // Flush with coordinates allocated but no cells added (should succeed)
  test(allocator->flush() == true);
  test(allocator->isValid() == true);

  // Add a cell before defining vertices (should succeed but not add anything
  // until the allocator is flushed)
  test(allocator->addCell(cellType, &connectivity[0], nVerticesPerCell));
  test(allocator->cells().size() == 0);

  // fill the vertices and remember the connectivity (should succeed)
  for (std::size_t i = 0; i < nVerticesPerCell; i++)
    {
    test(allocator->addCoordinate(i, cellPoints[cellType][i]));
    }

  // Finalize the addition of cells (should succeed)
  test(allocator->flush());
  test(allocator->isValid() == true);

  test(allocator->cells().size() == 1);

  smtk::mesh::MeshSet mesh =
    collection->createMesh(smtk::mesh::CellSet(collection, allocator->cells()));

  test(mesh.points().size() == nVerticesPerCell);
}

//----------------------------------------------------------------------------
void verify_moab_bufferred_cell_allocator_cells()
{
  // Allocate one of each type of cell.

  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::InterfacePtr iface = smtk::mesh::moab::make_interface();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection(iface);

  test( collection->isValid(), "collection should be valid");
  test( !collection->isModified(), "collection shouldn't be modified");

  smtk::mesh::BufferedCellAllocatorPtr allocator =
    collection->interface()->bufferedCellAllocator();

  // First, we must allocate the number of points needed for all of the cells.
  std::size_t nVertices = 0;

  for (int cellType = smtk::mesh::Vertex; cellType != smtk::mesh::CellType_MAX;
       ++cellType)
    {
    std::size_t nVerticesPerCell =
      (cellType == smtk::mesh::Polygon ? 5 :
       smtk::mesh::verticesPerCell(smtk::mesh::CellType(cellType)));

    nVertices += nVerticesPerCell;
    }

  test(allocator->reserveNumberOfCoordinates(nVertices));

  // Now that all of the points are allocated, we can incrementally add cells
  // and fill the point values in any order we want. Note that it is definitely
  // preferable to allocate like cells in sequence, however.

  std::size_t pointCounter = 0;

  for (int cellType = smtk::mesh::Vertex; cellType != smtk::mesh::CellType_MAX;
       ++cellType)
    {
    std::size_t nVerticesPerCell =
      (cellType == smtk::mesh::Polygon ? 5 :
       smtk::mesh::verticesPerCell(smtk::mesh::CellType(cellType)));
    std::vector<int> connectivity(nVerticesPerCell);

    // Fill the vertices for each cell and record its connectivity. To prevent
    // overlapping, we offset cell vertices by 2 * unit cell length.
    for (std::size_t i = 0; i < nVerticesPerCell; i++)
      {
      double xyz[3];
      for (int j = 0; j < 3; j++)
        {
        xyz[j] = cellPoints[cellType][i][j];
        }
      xyz[0] += 2.*cellType;
      test(allocator->addCoordinate(i, xyz));
      connectivity[i] = pointCounter++;
      }
    test(allocator->addCell(smtk::mesh::CellType(cellType), &connectivity[0],
                            nVerticesPerCell));
    }

  test(allocator->flush());

  test(allocator->cells().size() == 9);

  smtk::mesh::MeshSet mesh =
    collection->createMesh(smtk::mesh::CellSet(collection, allocator->cells()));

  test(mesh.points().size() == nVertices);
}

}

//----------------------------------------------------------------------------
int UnitTestBufferedCellAllocator(int, char** const)
{
  verify_moab_bufferred_cell_allocator_creation();
  verify_json_bufferred_cell_allocator_creation();

  for (int cellType = smtk::mesh::Vertex; cellType != smtk::mesh::CellType_MAX;
       ++cellType)
    {
    smtk::mesh::CellType ct = smtk::mesh::CellType(cellType);
    verify_moab_bufferred_cell_allocator_validity(ct);
    verify_moab_bufferred_cell_allocator_cell(ct);
    }

  verify_moab_bufferred_cell_allocator_cells();

  return 0;
}
