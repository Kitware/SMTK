
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
smtk::mesh::CollectionPtr load_mesh(smtk::mesh::ManagerPtr mngr)
{
  std::string file_path(data_root);
  file_path += "/mesh/twoassm_out.h5m";

  smtk::mesh::CollectionPtr c  = smtk::io::ImportMesh::entireFile(file_path, mngr);
  test( c->isValid(), "collection should be valid");

  return c;
}

//----------------------------------------------------------------------------
void cleanup(const smtk::mesh::CollectionPtr& c,
             smtk::mesh::MeshSet meshset)
{
  smtk::mesh::MeshSet allMeshes = c->meshes();

  const bool is_part_of_collection =
      smtk::mesh::set_intersect(allMeshes, meshset).is_empty();
  if(is_part_of_collection)
    {
    test( c->removeMeshes(meshset) == true );
    }
}

//----------------------------------------------------------------------------
void verify_create_empty_mesh(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numMeshesBeforeCreation = c->numberOfMeshes();

  smtk::mesh::CellSet emptyCellSet = smtk::mesh::CellSet(c, smtk::mesh::HandleRange() );
  smtk::mesh::MeshSet result = c->createMesh( emptyCellSet );

  test(result.is_empty(), "empty cellset should create empty meshset");
  test(numMeshesBeforeCreation == c->numberOfMeshes(),
       "the number of meshes shouldn't change when adding an empty mesh");
}

//----------------------------------------------------------------------------
void verify_create_mesh_with_cells_from_other_collection(
    smtk::mesh::ManagerPtr mngr,
    const smtk::mesh::CollectionPtr& c)
{
  //make another collection inside the manager
  smtk::mesh::CollectionPtr otherc = load_mesh(mngr);

  const std::size_t numMeshesBeforeCreation = c->numberOfMeshes();

  smtk::mesh::CellSet cellsFromOtherMesh = otherc->cells();
  smtk::mesh::MeshSet result = c->createMesh( cellsFromOtherMesh );

  test(result.is_empty(), "cellset from different collection should create empty meshset");
  test(numMeshesBeforeCreation == c->numberOfMeshes());

  //unload the second collection from memory
  mngr->removeCollection(otherc);
}

//----------------------------------------------------------------------------
void verify_create_mesh_with_invalid_cell_ids(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numMeshesBeforeCreation = c->numberOfMeshes();

  smtk::mesh::HandleRange invalidRange;
  invalidRange.insert(0,5); //insert values 0 to 5;
  smtk::mesh::CellSet invalidCellIds = smtk::mesh::CellSet(c, invalidRange);
  smtk::mesh::MeshSet result = c->createMesh( invalidCellIds );

  test(result.is_empty(), "invalid cellset should create empty meshset");
  test(numMeshesBeforeCreation == c->numberOfMeshes());
}

//----------------------------------------------------------------------------
void verify_create_mesh(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numMeshesBeforeCreation = c->numberOfMeshes();

  smtk::mesh::CellSet allNonVolumeCells =
      smtk::mesh::set_difference(c->cells(), c->cells( smtk::mesh::Dims3 ) );
  smtk::mesh::MeshSet result = c->createMesh( allNonVolumeCells );

  test( result.size() == 1, "valid cellset should create meshset with single mesh");
  test( (numMeshesBeforeCreation + 1) == c->numberOfMeshes());

  test( result.cells().size() == allNonVolumeCells.size() );
  test( result.cells() == allNonVolumeCells );

  cleanup(c, result);
}

//----------------------------------------------------------------------------
void verify_create_mesh_num_meshes(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numMeshesBeforeCreation = c->numberOfMeshes();


  //validate that for each mesh we create the numberOfMeshes is correct
  std::vector< smtk::mesh::MeshSet > results;
  for(int i=0; i < 3; ++i)
    {
    smtk::mesh::DimensionType dt = static_cast<smtk::mesh::DimensionType>(i);
    results.push_back( c->createMesh( c->cells( dt ) ) );
    }
  test( (numMeshesBeforeCreation + 3) == c->numberOfMeshes());

  for(std::size_t i=0; i < results.size(); ++i)
    {
    cleanup(c, results[i]);
    }
}

//----------------------------------------------------------------------------
void verify_create_mesh_updated_mesh_queries(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numMeshesBeforeCreation = c->numberOfMeshes();

  smtk::mesh::CellSet allNonVolumeCells =
      smtk::mesh::set_difference(c->cells(), c->cells( smtk::mesh::Dims3 ) );
  smtk::mesh::MeshSet result = c->createMesh( allNonVolumeCells );

  test( (numMeshesBeforeCreation + 1) == c->numberOfMeshes());

  //The meshset returned from create should entirely be contained within
  //all the meshes in the collection
  smtk::mesh::MeshSet intersect = smtk::mesh::set_intersect(c->meshes(), result );
  test( intersect == result );


  //The meshset returned from create should entirely be contained within
  //all the meshes that have 2d cells
  intersect = smtk::mesh::set_intersect(c->meshes(smtk::mesh::Dims2), result );
  test( intersect == result );

  cleanup(c, result);
}

//----------------------------------------------------------------------------
void verify_create_mesh_num_cells(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numCellsBeforeCreation = c->cells().size();

  smtk::mesh::CellSet allNonVolumeCells =
      smtk::mesh::set_difference(c->cells(), c->cells( smtk::mesh::Dims3 ) );
  smtk::mesh::MeshSet result = c->createMesh( allNonVolumeCells );

  test( result.size() == 1, "valid cellset should create meshset with single mesh");
  test( numCellsBeforeCreation == c->cells().size());

  cleanup(c, result);
}

//----------------------------------------------------------------------------
void verify_create_mesh_marks_modified( )
{
  //verify that a collection loaded from file is not marked as modified
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = load_mesh(mngr);
  test( c->isModified() == false, "collection loaded from disk shouldn't be modified");

  //verify a failure to create meshes doesn't mark as modify
  verify_create_mesh_with_invalid_cell_ids(c);
  test( c->isModified() == false, "collection loaded from disk shouldn't be modified");

  //verify that creating a mesh does mark update modify flag
  verify_create_mesh(c);
  test( c->isModified() == true, "collection should be marked as modified now");
}

}

//----------------------------------------------------------------------------
int UnitTestCreateMesh(int, char** const)
{
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = load_mesh(mngr);

  verify_create_empty_mesh(c);
  verify_create_mesh_with_cells_from_other_collection(mngr,c);
  verify_create_mesh_with_invalid_cell_ids(c);

  verify_create_mesh(c);

  verify_create_mesh_num_meshes(c);
  verify_create_mesh_updated_mesh_queries(c);

  verify_create_mesh_num_cells(c);

  verify_create_mesh_marks_modified();

  return 0;
}
