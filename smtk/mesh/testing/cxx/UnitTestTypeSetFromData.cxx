//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ImportMesh.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

smtk::mesh::ResourcePtr load_hex_mesh()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  smtk::io::importMesh(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  return mr;
}

smtk::mesh::ResourcePtr load_tet_mesh()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  smtk::io::importMesh(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  return mr;
}

void verify_hex_typeset_queries(const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::TypeSet types = mr->types();

  //to begin with TypeSet will only look at the fairly basic information
  test(types.hasCells(), "This resource should have cells");
  test(types.hasMeshes(), "This resource should have meshes");

  //now lets make sure we get the correct result for the dimensions
  //that this resource has

  //correct dims
  test(types.hasDimension(smtk::mesh::Dims0));
  test(types.hasDimension(smtk::mesh::Dims1));
  test(types.hasDimension(smtk::mesh::Dims2));
  test(types.hasDimension(smtk::mesh::Dims3));

  //now lets make sure we get the correct result for the type of cells
  //that this resource holds:
  // vertex, lines, quads, hexs
  test(types.hasCell(smtk::mesh::Vertex) == true);

  test(types.hasCell(smtk::mesh::Line) == true);

  test(types.hasCell(smtk::mesh::Triangle) == false);
  test(types.hasCell(smtk::mesh::Quad) == true);
  test(types.hasCell(smtk::mesh::Polygon) == false);

  test(types.hasCell(smtk::mesh::Tetrahedron) == false);
  test(types.hasCell(smtk::mesh::Pyramid) == false);
  test(types.hasCell(smtk::mesh::Wedge) == false);
  test(types.hasCell(smtk::mesh::Hexahedron) == true);

  smtk::mesh::MeshSet hexs = mr->meshes(smtk::mesh::Dims3);
  smtk::mesh::TypeSet hexTypes = hexs.types();

  test(hexTypes.hasMeshes());
  test(hexTypes.hasCell(smtk::mesh::Hexahedron));
  test(hexTypes.hasDimension(smtk::mesh::Dims3) == true);
  test(hexTypes.hasDimension(smtk::mesh::Dims2) == false);
  test(hexTypes.hasDimension(smtk::mesh::Dims1) == false);
  test(hexTypes.hasDimension(smtk::mesh::Dims0) == false);
}

void verify_tet_typeset_queries(const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::TypeSet types = mr->types();

  //to begin with TypeSet will only look at the fairly basic information
  test(types.hasCells(), "This resource should have cells");
  test(types.hasMeshes(), "This resource should have meshes");

  //now lets make sure we get the correct result for the dimensions
  //that this resource has

  //correct dims
  test(types.hasDimension(smtk::mesh::Dims0));
  test(types.hasDimension(smtk::mesh::Dims1));
  test(types.hasDimension(smtk::mesh::Dims2));
  test(types.hasDimension(smtk::mesh::Dims3));

  //now lets make sure we get the correct result for the type of cells
  //that this resource holds:
  // vertex, lines, triangles, tets
  test(types.hasCell(smtk::mesh::Vertex) == true);

  test(types.hasCell(smtk::mesh::Line) == true);

  test(types.hasCell(smtk::mesh::Triangle) == true);
  test(types.hasCell(smtk::mesh::Quad) == false);
  test(types.hasCell(smtk::mesh::Polygon) == false);

  test(types.hasCell(smtk::mesh::Tetrahedron) == true);
  test(types.hasCell(smtk::mesh::Pyramid) == false);
  test(types.hasCell(smtk::mesh::Wedge) == false);
  test(types.hasCell(smtk::mesh::Hexahedron) == false);

  //Unlike the hex mesh, the tet mesh has all the cell types mixed
  //into a single meshset, so when we ask for meshsets with cells of
  //dimension 3 it also has cells of other dimensions
  smtk::mesh::MeshSet tets = mr->meshes(smtk::mesh::Dims3);
  smtk::mesh::TypeSet tetTypes = tets.types();

  test(tetTypes.hasMeshes());
  test(tetTypes.hasCell(smtk::mesh::Tetrahedron) == true);
  test(tetTypes.hasDimension(smtk::mesh::Dims3) == true);
  test(tetTypes.hasDimension(smtk::mesh::Dims2) == true);
  test(tetTypes.hasDimension(smtk::mesh::Dims1) == true);
  test(tetTypes.hasDimension(smtk::mesh::Dims0) == true);

  //extract only the tet cells from the mixed type meshset
  smtk::mesh::CellSet tetCells = tets.cells(smtk::mesh::Dims3);
  smtk::mesh::TypeSet tetCTypes = tetCells.types();

  test(tetCTypes.hasMeshes() == false);
  test(tetCTypes.hasCell(smtk::mesh::Tetrahedron) == true);
  test(tetCTypes.hasDimension(smtk::mesh::Dims3) == true);
  test(tetCTypes.hasDimension(smtk::mesh::Dims2) == false);
  test(tetCTypes.hasDimension(smtk::mesh::Dims1) == false);
  test(tetCTypes.hasDimension(smtk::mesh::Dims0) == false);
}
}

int UnitTestTypeSetFromData(int, char** const)
{
  smtk::mesh::ResourcePtr hexMeshResource = load_hex_mesh();
  verify_hex_typeset_queries(hexMeshResource);

  smtk::mesh::ResourcePtr tetMeshResource = load_tet_mesh();
  verify_tet_typeset_queries(tetMeshResource);

  return 0;
}
