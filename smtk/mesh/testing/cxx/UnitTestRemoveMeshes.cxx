
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
#include "smtk/model/EntityRef.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

smtk::mesh::ResourcePtr load_mesh()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  smtk::io::importMesh(file_path, mr);
  test(mr->isValid(), "mesh resource should be valid");

  return mr;
}

void reset(const smtk::mesh::ResourcePtr& mr)
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  //reset the mesh by re-importing the file.
  smtk::io::importMesh(file_path, mr);
}

void verify_remove_empty_mesh(const smtk::mesh::ResourcePtr& mr)
{
  const std::size_t numMeshesBeforeRemoval = mr->numberOfMeshes();

  smtk::mesh::MeshSet emptyMeshSet(mr, 0, smtk::mesh::HandleRange());

  const bool result = mr->removeMeshes(emptyMeshSet);

  test(result, "delete nothing is always true");
  test(!mr->isModified(), "deleting nothing should not change the modify flag");

  test(numMeshesBeforeRemoval == mr->numberOfMeshes(),
    "deleting no meshes shouldn't modify number of meshes");
}

void verify_remove_mesh_from_other_resource(const smtk::mesh::ResourcePtr& mr)
{
  //make another mesh resource
  smtk::mesh::ResourcePtr othermr = load_mesh();

  const std::size_t numMeshesBeforeRemoval = mr->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = mr->cells().size();

  smtk::mesh::MeshSet meshesFromOtherResource = othermr->meshes();
  const bool result = mr->removeMeshes(meshesFromOtherResource);

  test(!result, "can't remove meshes from the incorrect mesh resource");
  test(!mr->isModified(), "shouldn't be modified after an invalid removal");
  test(numMeshesBeforeRemoval == mr->numberOfMeshes());
  test(numCellsBeforeRemoval == mr->cells().size());
}

void verify_remove_invalid_meshes(const smtk::mesh::ResourcePtr& mr)
{
  const std::size_t numMeshesBeforeRemoval = mr->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = mr->cells().size();

  //roughly estimate the number of handles being used by the implemenation.
  //this is numCells + 8 * numCells + numMeshes. The 8 * numCells is a standin
  //for number of verts
  const std::size_t numHandlesUsed = (mr->cells().size() * 9) + mr->numberOfMeshes();
  smtk::mesh::HandleRange invalidRange;
  invalidRange.insert(smtk::mesh::HandleInterval(numHandlesUsed + 10, numHandlesUsed + 40));
  smtk::mesh::MeshSet invalidMeshIds = smtk::mesh::MeshSet(mr, 0, invalidRange);

  const bool result = mr->removeMeshes(invalidMeshIds);

  test(result, "deletion of non-existent cells, is equal to deleting an empty mesh");
  test(numMeshesBeforeRemoval == mr->numberOfMeshes());
  test(numCellsBeforeRemoval == mr->cells().size());
}

void verify_remove_already_removed_meshes(const smtk::mesh::ResourcePtr& mr)
{
  const std::size_t numMeshesBeforeRemoval = mr->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = mr->cells().size();

  //create a new mesh to be removed.
  smtk::mesh::CellSet allNonVolumeCells =
    smtk::mesh::set_difference(mr->cells(), mr->cells(smtk::mesh::Dims3));
  smtk::mesh::MeshSet newMesh = mr->createMesh(allNonVolumeCells);

  //while it is a meshset it should only have a single value
  test(newMesh.size() == 1);

  const bool removedFirstTime = mr->removeMeshes(newMesh);
  test(removedFirstTime, "should have no problem removing these meshes");

  const bool removedSecondTime = mr->removeMeshes(newMesh);
  test(!removedSecondTime, "should not be able to remove the same mesh twice");

  test(numMeshesBeforeRemoval == mr->numberOfMeshes());
  test(numCellsBeforeRemoval == mr->cells().size());

  reset(mr);
}

void verify_remove_single_mesh(const smtk::mesh::ResourcePtr& mr)
{
  const std::size_t numMeshesBeforeRemoval = mr->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = mr->cells().size();

  //create a new mesh to be removed.
  smtk::mesh::CellSet allNonVolumeCells =
    smtk::mesh::set_difference(mr->cells(), mr->cells(smtk::mesh::Dims3));
  smtk::mesh::MeshSet newMesh = mr->createMesh(allNonVolumeCells);

  //while it is a meshset it should only have a single value
  test(newMesh.size() == 1);

  const bool result = mr->removeMeshes(newMesh);

  test(result, "should have no problem removing these meshes");
  test(mr->isModified(), "should be modified after a valid removal");
  test(numMeshesBeforeRemoval == mr->numberOfMeshes());
  test(numCellsBeforeRemoval == mr->cells().size());

  reset(mr);
}

void verify_remove_multiple_meshes(const smtk::mesh::ResourcePtr& mr)
{
  const std::size_t numMeshesBeforeRemoval = mr->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = mr->cells().size();

  //create a new mesh to be removed.
  smtk::mesh::CellSet allNonVolumeCells =
    smtk::mesh::set_difference(mr->cells(), mr->cells(smtk::mesh::Dims3));
  smtk::mesh::MeshSet newMeshes = mr->createMesh(allNonVolumeCells);

  //create a second new mesh to be removed.
  smtk::mesh::CellSet allVolumeCells = mr->cells(smtk::mesh::Dims3);
  newMeshes.append(mr->createMesh(allVolumeCells));

  //the meshset that we are deleting should have two elements
  test(newMeshes.size() == 2);

  const bool result = mr->removeMeshes(newMeshes);

  test(result, "should have no problem removing these meshes");
  test(mr->isModified(), "should be modified after a valid removal");
  test(numMeshesBeforeRemoval == mr->numberOfMeshes());
  test(numCellsBeforeRemoval == mr->cells().size());

  reset(mr);
}

void verify_remove_all_meshes(const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::MeshSet allMeshes = mr->meshes();
  const bool result = mr->removeMeshes(allMeshes);

  test(result, "deleted everything is always true");

  test(0 == mr->numberOfMeshes(), "deleting all meshes should result in zero meshes");
  test(0 == mr->cells().size(), "deleting all meshes should result in zero cells");

  test(mr->isModified(), "should be modified after a valid removal");

  reset(mr);
}

void verify_remove_meshes_removes_unused_cells(const smtk::mesh::ResourcePtr& mr)
{
  const std::size_t numMeshesBeforeRemoval = mr->numberOfMeshes();
  const std::size_t numCellsBeforeRemoval = mr->cells().size();

  //find a grouping of meshes that are the only users of cells
  //remove those meshes, and verify that those cells are deleted.
  smtk::mesh::MeshSet meshesWithDim3 = mr->meshes(smtk::mesh::Dims3);
  smtk::mesh::MeshSet otherMeshes = mr->meshes(smtk::mesh::Dims2);
  otherMeshes.append(mr->meshes(smtk::mesh::Dims1));

  //meshesWithOnlyDim3 will contain meshsets that are pure 3d cells
  smtk::mesh::MeshSet onlyVolumeMeshes = smtk::mesh::set_difference(meshesWithDim3, otherMeshes);

  const std::size_t numVolumeMeshes = onlyVolumeMeshes.size();
  const std::size_t numVolumeCells = onlyVolumeMeshes.cells().size();

  const bool result = mr->removeMeshes(onlyVolumeMeshes);

  test(result, "deleting all volume only meshes should work");
  test(mr->isModified(), "should be modified after a valid removal");

  const std::size_t expectedNumberOfMeshes = numMeshesBeforeRemoval - numVolumeMeshes;
  const std::size_t expectedNumberOfCells = numCellsBeforeRemoval - numVolumeCells;

  test(expectedNumberOfMeshes == mr->numberOfMeshes());
  test(expectedNumberOfCells == mr->cells().size());

  reset(mr);
}

void verify_remove_verts_with_model_association(const smtk::mesh::ResourcePtr& mr)
{
  //make a mesh that only holds verts
  std::size_t num_meshes = mr->meshes().size();
  std::size_t num_cells = mr->cells().size();

  smtk::mesh::CellSet vertCells = mr->cells(smtk::mesh::Dims0);
  smtk::mesh::MeshSet vertMesh = mr->createMesh(vertCells);

  test((num_meshes == mr->meshes().size() - 1), "");
  test((num_cells == (mr->cells().size())), "");

  //add a model association to the vert mesh
  smtk::common::UUID entity = smtk::common::UUID::random();
  smtk::model::EntityRef eref;
  eref.setEntity(entity);
  mr->setAssociation(eref, vertMesh);

  //verify we have an association
  std::size_t numAssoc = mr->findAssociatedMeshes(eref).size();
  test(numAssoc == 1, "");

  bool removed = mr->removeMeshes(vertMesh);

  //verify the association is removed
  numAssoc = mr->findAssociatedMeshes(eref).size();
  test(numAssoc == 0, "");

  test(removed, "should be able to remove mesh of just verts");
  test((num_meshes == mr->meshes().size()), "");
  test((num_cells == mr->cells().size()), "");

  reset(mr);
}
}

int UnitTestRemoveMeshes(int /*unused*/, char** const /*unused*/)
{
  smtk::mesh::ResourcePtr mr = load_mesh();

  verify_remove_empty_mesh(mr);
  verify_remove_mesh_from_other_resource(mr);
  verify_remove_invalid_meshes(mr);
  verify_remove_already_removed_meshes(mr);

  verify_remove_single_mesh(mr);
  verify_remove_multiple_meshes(mr);

  verify_remove_all_meshes(mr);
  verify_remove_meshes_removes_unused_cells(mr);

  verify_remove_verts_with_model_association(mr);

  return 0;
}
