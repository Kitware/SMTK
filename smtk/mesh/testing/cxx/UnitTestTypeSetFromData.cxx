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
smtk::mesh::Collection load_mesh(smtk::mesh::ManagerPtr mngr)
{
  std::string file_path(data_root);
  file_path += "/mesh/twoassm_out.h5m";
  smtk::common::UUID entity = smtk::io::ImportMesh::intoManager(file_path, mngr);
  test( !entity.isNull(), "uuid shouldn't be invalid");

  smtk::mesh::Collection c = mngr->collection(entity);
  test( c.isValid(), "collection should be valid");

  return c;
}

//----------------------------------------------------------------------------
void verify_typeset_queries(const smtk::mesh::Collection& c)
{
  smtk::mesh::TypeSet types = c.associatedTypes();

  //to begin with TypeSet will only look at the fairly basic information
  test( types.hasCells(), "This collection should have cells");
  test( types.hasPoints(), "This collection should have points");
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

}

//----------------------------------------------------------------------------
int UnitTestTypeSetFromData(int argc, char** argv)
{
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();
  smtk::mesh::Collection c = load_mesh(mngr);

  verify_typeset_queries(c);

  return 0;
}