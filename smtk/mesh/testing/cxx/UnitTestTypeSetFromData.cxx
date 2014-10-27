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
  smtk::common::UUID entity = smtk::io::ImportMesh::intoManager(file_path, mngr);
  test( !entity.isNull(), "uuid shouldn't be invalid");

  smtk::mesh::CollectionPtr c = mngr->collection(entity);
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
  smtk::mesh::TypeSet types = c->associatedTypes();

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

}

//----------------------------------------------------------------------------
void verify_tet_typeset_queries(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::TypeSet types = c->associatedTypes();

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
}

}

//----------------------------------------------------------------------------
int UnitTestTypeSetFromData(int argc, char** argv)
{
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();

  smtk::mesh::CollectionPtr hexCollec = load_hex_mesh(mngr);
  verify_hex_typeset_queries(hexCollec);

  smtk::mesh::CollectionPtr tetCollec = load_tet_mesh(mngr);
  verify_tet_typeset_queries(tetCollec);

  return 0;
}