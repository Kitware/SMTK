
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
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/model/EntityRef.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

smtk::mesh::CollectionPtr load_mesh(smtk::mesh::ManagerPtr mngr)
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  smtk::mesh::CollectionPtr c = smtk::io::importMesh(file_path, mngr);
  test(c->isValid(), "collection should be valid");

  return c;
}

void reset(const smtk::mesh::CollectionPtr& c)
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  //reset the mesh by re-importing the file.
  smtk::io::importMesh(file_path, c);
}

void verify_remove_empty_mesh(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numMeshesBeforeRemoval = c->numberOfMeshes();

  smtk::mesh::MeshSet emptyMeshSet(c, 0, smtk::mesh::HandleRange());

  const bool result = c->removeMeshes(emptyMeshSet);

  test(result, "delete nothing is always true");
  test(!c->isModified(), "deleting nothing should not change the modify flag");

  test(numMeshesBeforeRemoval == c->numberOfMeshes(),
    "deleting no meshes shouldn't modify number of meshes");
}

void verify_remove_mesh_from_other_collection(
  smtk::mesh::ManagerPtr mngr, const smtk::mesh::CollectionPtr& c)
{
  //make another collection inside the manager
  smtk::mesh::CollectionPtr otherc = load_mesh(mngr);

  const std::size_t numMeshesBeforeRemoval = c->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = c->cells().size();

  smtk::mesh::MeshSet meshesFromOtherCollection = otherc->meshes();
  const bool result = c->removeMeshes(meshesFromOtherCollection);

  test(!result, "can't remove meshes from the incorrect collection");
  test(!c->isModified(), "shouldn't be modified after an invalid removal");
  test(numMeshesBeforeRemoval == c->numberOfMeshes());
  test(numCellsBeforeRemoval == c->cells().size());

  //unload the second collection from memory
  mngr->removeCollection(otherc);
}

void verify_remove_invalid_meshes(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numMeshesBeforeRemoval = c->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = c->cells().size();

  //roughly estimate the number of handles being used by the implemenation.
  //this is numCells + 8 * numCells + numMeshes. The 8 * numCells is a standin
  //for number of verts
  const std::size_t numHandlesUsed = (c->cells().size() * 9) + c->numberOfMeshes();
  smtk::mesh::HandleRange invalidRange;
  invalidRange.insert(numHandlesUsed + 10, numHandlesUsed + 40);
  smtk::mesh::MeshSet invalidMeshIds = smtk::mesh::MeshSet(c, 0, invalidRange);

  const bool result = c->removeMeshes(invalidMeshIds);

  test(result, "deletion of non-existent cells, is equal to deleting an empty mesh");
  test(numMeshesBeforeRemoval == c->numberOfMeshes());
  test(numCellsBeforeRemoval == c->cells().size());
}

void verify_remove_already_removed_meshes(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numMeshesBeforeRemoval = c->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = c->cells().size();

  //create a new mesh to be removed.
  smtk::mesh::CellSet allNonVolumeCells =
    smtk::mesh::set_difference(c->cells(), c->cells(smtk::mesh::Dims3));
  smtk::mesh::MeshSet newMesh = c->createMesh(allNonVolumeCells);

  //while it is a meshset it should only have a single value
  test(newMesh.size() == 1);

  const bool removedFirstTime = c->removeMeshes(newMesh);
  test(removedFirstTime, "should have no problem removing these meshes");

  const bool removedSecondTime = c->removeMeshes(newMesh);
  test(!removedSecondTime, "should not be able to remove the same mesh twice");

  test(numMeshesBeforeRemoval == c->numberOfMeshes());
  test(numCellsBeforeRemoval == c->cells().size());

  reset(c);
}

void verify_remove_single_mesh(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numMeshesBeforeRemoval = c->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = c->cells().size();

  //create a new mesh to be removed.
  smtk::mesh::CellSet allNonVolumeCells =
    smtk::mesh::set_difference(c->cells(), c->cells(smtk::mesh::Dims3));
  smtk::mesh::MeshSet newMesh = c->createMesh(allNonVolumeCells);

  //while it is a meshset it should only have a single value
  test(newMesh.size() == 1);

  const bool result = c->removeMeshes(newMesh);

  test(result, "should have no problem removing these meshes");
  test(c->isModified(), "should be modified after a valid removal");
  test(numMeshesBeforeRemoval == c->numberOfMeshes());
  test(numCellsBeforeRemoval == c->cells().size());

  reset(c);
}

void verify_remove_multiple_meshes(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numMeshesBeforeRemoval = c->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = c->cells().size();

  //create a new mesh to be removed.
  smtk::mesh::CellSet allNonVolumeCells =
    smtk::mesh::set_difference(c->cells(), c->cells(smtk::mesh::Dims3));
  smtk::mesh::MeshSet newMeshes = c->createMesh(allNonVolumeCells);

  //create a second new mesh to be removed.
  smtk::mesh::CellSet allVolumeCells = c->cells(smtk::mesh::Dims3);
  newMeshes.append(c->createMesh(allVolumeCells));

  //the meshset that we are deleting should have two elements
  test(newMeshes.size() == 2);

  const bool result = c->removeMeshes(newMeshes);

  test(result, "should have no problem removing these meshes");
  test(c->isModified(), "should be modified after a valid removal");
  test(numMeshesBeforeRemoval == c->numberOfMeshes());
  test(numCellsBeforeRemoval == c->cells().size());

  reset(c);
}

void verify_remove_all_meshes(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::MeshSet allMeshes = c->meshes();
  const bool result = c->removeMeshes(allMeshes);

  test(result, "deleted everything is always true");

  test(0 == c->numberOfMeshes(), "deleting all meshes should result in zero meshes");
  test(0 == c->cells().size(), "deleting all meshes should result in zero cells");

  test(c->isModified(), "should be modified after a valid removal");

  reset(c);
}

void verify_remove_meshes_removes_unused_cells(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t numMeshesBeforeRemoval = c->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = c->cells().size();

  //find a grouping of meshes that are the only users of cells
  //remove those meshes, and verify that those cells are deleted.
  smtk::mesh::MeshSet meshesWithDim3 = c->meshes(smtk::mesh::Dims3);
  smtk::mesh::MeshSet otherMeshes = c->meshes(smtk::mesh::Dims2);
  otherMeshes.append(c->meshes(smtk::mesh::Dims1));

  //meshesWithOnlyDim3 will contain meshsets that are pure 3d cells
  smtk::mesh::MeshSet onlyVolumeMeshes = smtk::mesh::set_difference(meshesWithDim3, otherMeshes);

  const std::size_t numVolumeMeshes = onlyVolumeMeshes.size();
  const std::size_t numVolumeCells = onlyVolumeMeshes.cells().size();

  const bool result = c->removeMeshes(onlyVolumeMeshes);

  test(result, "deleting all volume only meshes should work");
  test(c->isModified(), "should be modified after a valid removal");

  const std::size_t expectedNumberOfMeshes = numMeshesBeforeRemoval - numVolumeMeshes;
  const std::size_t expectedNumberOfCells = numCellsBeforeRemoval - numVolumeCells;

  test(expectedNumberOfMeshes == c->numberOfMeshes());
  test(expectedNumberOfCells == c->cells().size());

  reset(c);
}

void verify_remove_verts_with_model_association(const smtk::mesh::CollectionPtr& c)
{
  //make a mesh that only holds verts
  std::size_t num_meshes = c->meshes().size();
  std::size_t num_cells = c->cells().size();

  smtk::mesh::CellSet vertCells = c->cells(smtk::mesh::Dims0);
  smtk::mesh::MeshSet vertMesh = c->createMesh(vertCells);

  test((num_meshes == c->meshes().size() - 1), "");
  test((num_cells == (c->cells().size())), "");

  //add a model association to the vert mesh
  smtk::common::UUID entity = smtk::common::UUID::random();
  smtk::model::EntityRef eref;
  eref.setEntity(entity);
  c->setAssociation(eref, vertMesh);

  //verify we have an association
  std::size_t numAssoc = c->findAssociatedMeshes(eref).size();
  test(numAssoc == 1, "");

  bool removed = c->removeMeshes(vertMesh);

  //verify the association is removed
  numAssoc = c->findAssociatedMeshes(eref).size();
  test(numAssoc == 0, "");

  test(removed, "should be able to remove mesh of just verts");
  test((num_meshes == c->meshes().size()), "");
  test((num_cells == c->cells().size()), "");

  reset(c);
}
}

int UnitTestRemoveMeshes(int, char** const)
{
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = load_mesh(mngr);

  verify_remove_empty_mesh(c);
  verify_remove_mesh_from_other_collection(mngr, c);
  verify_remove_invalid_meshes(c);
  verify_remove_already_removed_meshes(c);

  verify_remove_single_mesh(c);
  verify_remove_multiple_meshes(c);

  verify_remove_all_meshes(c);
  verify_remove_meshes_removes_unused_cells(c);

  verify_remove_verts_with_model_association(c);

  return 0;
}
