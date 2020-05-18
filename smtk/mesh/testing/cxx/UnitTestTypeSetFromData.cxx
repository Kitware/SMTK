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
  test(types.hasCell(smtk::mesh::Vertex));

  test(types.hasCell(smtk::mesh::Line));

  test(!types.hasCell(smtk::mesh::Triangle));
  test(types.hasCell(smtk::mesh::Quad));
  test(!types.hasCell(smtk::mesh::Polygon));

  test(!types.hasCell(smtk::mesh::Tetrahedron));
  test(!types.hasCell(smtk::mesh::Pyramid));
  test(!types.hasCell(smtk::mesh::Wedge));
  test(types.hasCell(smtk::mesh::Hexahedron));

  smtk::mesh::MeshSet hexs = mr->meshes(smtk::mesh::Dims3);
  smtk::mesh::TypeSet hexTypes = hexs.types();

  test(hexTypes.hasMeshes());
  test(hexTypes.hasCell(smtk::mesh::Hexahedron));
  test(hexTypes.hasDimension(smtk::mesh::Dims3));
  test(!hexTypes.hasDimension(smtk::mesh::Dims2));
  test(!hexTypes.hasDimension(smtk::mesh::Dims1));
  test(!hexTypes.hasDimension(smtk::mesh::Dims0));
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
  test(types.hasCell(smtk::mesh::Vertex));

  test(types.hasCell(smtk::mesh::Line));

  test(types.hasCell(smtk::mesh::Triangle));
  test(!types.hasCell(smtk::mesh::Quad));
  test(!types.hasCell(smtk::mesh::Polygon));

  test(types.hasCell(smtk::mesh::Tetrahedron));
  test(!types.hasCell(smtk::mesh::Pyramid));
  test(!types.hasCell(smtk::mesh::Wedge));
  test(!types.hasCell(smtk::mesh::Hexahedron));

  //Unlike the hex mesh, the tet mesh has all the cell types mixed
  //into a single meshset, so when we ask for meshsets with cells of
  //dimension 3 it also has cells of other dimensions
  smtk::mesh::MeshSet tets = mr->meshes(smtk::mesh::Dims3);
  smtk::mesh::TypeSet tetTypes = tets.types();

  test(tetTypes.hasMeshes());
  test(tetTypes.hasCell(smtk::mesh::Tetrahedron));
  test(tetTypes.hasDimension(smtk::mesh::Dims3));
  test(tetTypes.hasDimension(smtk::mesh::Dims2));
  test(tetTypes.hasDimension(smtk::mesh::Dims1));
  test(tetTypes.hasDimension(smtk::mesh::Dims0));

  //extract only the tet cells from the mixed type meshset
  smtk::mesh::CellSet tetCells = tets.cells(smtk::mesh::Dims3);
  smtk::mesh::TypeSet tetCTypes = tetCells.types();

  test(!tetCTypes.hasMeshes());
  test(tetCTypes.hasCell(smtk::mesh::Tetrahedron));
  test(tetCTypes.hasDimension(smtk::mesh::Dims3));
  test(!tetCTypes.hasDimension(smtk::mesh::Dims2));
  test(!tetCTypes.hasDimension(smtk::mesh::Dims1));
  test(!tetCTypes.hasDimension(smtk::mesh::Dims0));
}
} // namespace

int UnitTestTypeSetFromData(int /*unused*/, char** const /*unused*/)
{
  smtk::mesh::ResourcePtr hexMeshResource = load_hex_mesh();
  verify_hex_typeset_queries(hexMeshResource);

  smtk::mesh::ResourcePtr tetMeshResource = load_tet_mesh();
  verify_tet_typeset_queries(tetMeshResource);

  return 0;
}
