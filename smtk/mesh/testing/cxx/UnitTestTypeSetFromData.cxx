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
#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;


//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr load_hex_mesh(smtk::mesh::ManagerPtr mngr)
{
  std::string file_path(data_root);
  file_path += "/mesh/twoassm_out.h5m";

  smtk::mesh::CollectionPtr c  = smtk::io::ImportMesh::entireFile(file_path, mngr);
  test( c->isValid(), "collection should be valid");

  return c;
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr load_tet_mesh(smtk::mesh::ManagerPtr mngr)
{
  std::string file_path(data_root);
  file_path += "/mesh/64bricks_12ktet.h5m";

  smtk::mesh::CollectionPtr c  = smtk::io::ImportMesh::entireFile(file_path, mngr);
  test( c->isValid(), "collection should be valid");

  return c;
}

//----------------------------------------------------------------------------
void verify_hex_typeset_queries(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::TypeSet types = c->types();

  //to begin with TypeSet will only look at the fairly basic information
  test( types.hasCells(), "This collection should have cells");
  test( types.hasMeshes(), "This collection should have meshes");

  //now lets make sure we get the correct result for the dimensions
  //that this collection has

  //correct dims
  test( types.hasDimension( smtk::mesh::Dims0 ) );
  test( types.hasDimension( smtk::mesh::Dims1 ) );
  test( types.hasDimension( smtk::mesh::Dims2 ) );
  test( types.hasDimension( smtk::mesh::Dims3 ) );

  //now lets make sure we get the correct result for the type of cells
  //that this collection holds:
  // vertex, lines, quads, hexs
  test( types.hasCell(smtk::mesh::Vertex) == true );

  test( types.hasCell(smtk::mesh::Line) == true );

  test( types.hasCell(smtk::mesh::Triangle) == false );
  test( types.hasCell(smtk::mesh::Quad) == true );
  test( types.hasCell(smtk::mesh::Polygon) == false );

  test( types.hasCell(smtk::mesh::Tetrahedron) == false );
  test( types.hasCell(smtk::mesh::Pyramid) == false );
  test( types.hasCell(smtk::mesh::Wedge) == false );
  test( types.hasCell(smtk::mesh::Hexahedron) == true );

  smtk::mesh::MeshSet hexs = c->meshes( smtk::mesh::Dims3 );
  smtk::mesh::TypeSet hexTypes = hexs.types();

  test( hexTypes.hasMeshes() );
  test( hexTypes.hasCell( smtk::mesh::Hexahedron ) );
  test( hexTypes.hasDimension( smtk::mesh::Dims3 ) == true);
  test( hexTypes.hasDimension( smtk::mesh::Dims2 ) == false);
  test( hexTypes.hasDimension( smtk::mesh::Dims1 ) == false);
  test( hexTypes.hasDimension( smtk::mesh::Dims0 ) == false);

}

//----------------------------------------------------------------------------
void verify_tet_typeset_queries(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::TypeSet types = c->types();

  //to begin with TypeSet will only look at the fairly basic information
  test( types.hasCells(), "This collection should have cells");
  test( types.hasMeshes(), "This collection should have meshes");

  //now lets make sure we get the correct result for the dimensions
  //that this collection has

  //correct dims
  test( types.hasDimension( smtk::mesh::Dims0 ) );
  test( types.hasDimension( smtk::mesh::Dims1 ) );
  test( types.hasDimension( smtk::mesh::Dims2 ) );
  test( types.hasDimension( smtk::mesh::Dims3 ) );

  //now lets make sure we get the correct result for the type of cells
  //that this collection holds:
  // vertex, lines, triangles, tets
  test( types.hasCell(smtk::mesh::Vertex) == true );

  test( types.hasCell(smtk::mesh::Line) == true );

  test( types.hasCell(smtk::mesh::Triangle) == true );
  test( types.hasCell(smtk::mesh::Quad) == false );
  test( types.hasCell(smtk::mesh::Polygon) == false );

  test( types.hasCell(smtk::mesh::Tetrahedron) == true );
  test( types.hasCell(smtk::mesh::Pyramid) == false );
  test( types.hasCell(smtk::mesh::Wedge) == false );
  test( types.hasCell(smtk::mesh::Hexahedron) == false );

  //Unlike the hex mesh, the tet mesh has all the cell types mixed
  //into a single meshset, so when we ask for meshsets with cells of
  //dimension 3 it also has cells of other dimensions
  smtk::mesh::MeshSet tets = c->meshes( smtk::mesh::Dims3 );
  smtk::mesh::TypeSet tetTypes = tets.types();

  test( tetTypes.hasMeshes() );
  test( tetTypes.hasCell( smtk::mesh::Tetrahedron ) == true );
  test( tetTypes.hasDimension( smtk::mesh::Dims3 )  == true);
  test( tetTypes.hasDimension( smtk::mesh::Dims2 )  == true);
  test( tetTypes.hasDimension( smtk::mesh::Dims1 )  == true);
  test( tetTypes.hasDimension( smtk::mesh::Dims0 )  == true);

  //extract only the tet cells from the mixed type meshset
  smtk::mesh::CellSet tetCells = tets.cells(  smtk::mesh::Dims3 );
  smtk::mesh::TypeSet tetCTypes = tetCells.types();

  test( tetCTypes.hasMeshes() == false );
  test( tetCTypes.hasCell( smtk::mesh::Tetrahedron ) == true);
  test( tetCTypes.hasDimension( smtk::mesh::Dims3 )  == true);
  test( tetCTypes.hasDimension( smtk::mesh::Dims2 )  == false);
  test( tetCTypes.hasDimension( smtk::mesh::Dims1 )  == false);
  test( tetCTypes.hasDimension( smtk::mesh::Dims0 )  == false);

}

}

//----------------------------------------------------------------------------
int UnitTestTypeSetFromData(int, char** const)
{
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();

  smtk::mesh::CollectionPtr hexCollec = load_hex_mesh(mngr);
  verify_hex_typeset_queries(hexCollec);

  smtk::mesh::CollectionPtr tetCollec = load_tet_mesh(mngr);
  verify_tet_typeset_queries(tetCollec);

  return 0;
}
