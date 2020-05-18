
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

#include "smtk/mesh/moab/Interface.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

smtk::mesh::ResourcePtr load_mesh(const smtk::mesh::InterfacePtr& interface)
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  smtk::mesh::ResourcePtr mr = smtk::io::importMesh(file_path, interface);
  test(mr->isValid(), "resource should be valid");

  return mr;
}

void cleanup(const smtk::mesh::ResourcePtr& mr, smtk::mesh::MeshSet meshset)
{
  smtk::mesh::MeshSet allMeshes = mr->meshes();

  const bool is_part_of_resource = smtk::mesh::set_intersect(allMeshes, meshset).is_empty();
  if (is_part_of_resource)
  {
    test(mr->removeMeshes(meshset));
  }
}

void verify_create_empty_mesh(const smtk::mesh::ResourcePtr& mr)
{
  const std::size_t numMeshesBeforeCreation = mr->numberOfMeshes();

  smtk::mesh::CellSet emptyCellSet = smtk::mesh::CellSet(mr, smtk::mesh::HandleRange());
  smtk::mesh::MeshSet result = mr->createMesh(emptyCellSet);

  test(result.is_empty(), "empty cellset should create empty meshset");
  test(
    numMeshesBeforeCreation == mr->numberOfMeshes(),
    "the number of meshes shouldn't change when adding an empty mesh");
}

void verify_create_mesh_with_cells_from_other_resource(const smtk::mesh::ResourcePtr& mr)
{
  //make another resource inside the manager
  smtk::mesh::ResourcePtr othermr = load_mesh(mr->interface());

  const std::size_t numMeshesBeforeCreation = mr->numberOfMeshes();

  smtk::mesh::CellSet cellsFromOtherMesh = othermr->cells();
  smtk::mesh::MeshSet result = mr->createMesh(cellsFromOtherMesh);

  test(result.is_empty(), "cellset from different resource should create empty meshset");
  test(numMeshesBeforeCreation == mr->numberOfMeshes());
}

void verify_create_mesh(const smtk::mesh::ResourcePtr& mr)
{
  const std::size_t numMeshesBeforeCreation = mr->numberOfMeshes();

  smtk::mesh::CellSet allNonVolumeCells =
    smtk::mesh::set_difference(mr->cells(), mr->cells(smtk::mesh::Dims3));
  smtk::mesh::MeshSet result = mr->createMesh(allNonVolumeCells);

  test(result.size() == 1, "valid cellset should create meshset with single mesh");
  test((numMeshesBeforeCreation + 1) == mr->numberOfMeshes());

  test(result.cells().size() == allNonVolumeCells.size());
  test(result.cells() == allNonVolumeCells);

  cleanup(mr, result);
}

void verify_create_mesh_num_meshes(const smtk::mesh::ResourcePtr& mr)
{
  const std::size_t numMeshesBeforeCreation = mr->numberOfMeshes();

  //validate that for each mesh we create the numberOfMeshes is correct
  std::vector<smtk::mesh::MeshSet> results;
  for (int i = 0; i < 3; ++i)
  {
    smtk::mesh::DimensionType dt = static_cast<smtk::mesh::DimensionType>(i);
    results.push_back(mr->createMesh(mr->cells(dt)));
  }
  test((numMeshesBeforeCreation + 3) == mr->numberOfMeshes());

  for (std::size_t i = 0; i < results.size(); ++i)
  {
    cleanup(mr, results[i]);
  }
}

void verify_create_mesh_updated_mesh_queries(const smtk::mesh::ResourcePtr& mr)
{
  const std::size_t numMeshesBeforeCreation = mr->numberOfMeshes();

  smtk::mesh::CellSet allNonVolumeCells =
    smtk::mesh::set_difference(mr->cells(), mr->cells(smtk::mesh::Dims3));
  smtk::mesh::MeshSet result = mr->createMesh(allNonVolumeCells);

  test((numMeshesBeforeCreation + 1) == mr->numberOfMeshes());

  //The meshset returned from create should entirely be contained within
  //all the meshes in the resource
  smtk::mesh::MeshSet intersect = smtk::mesh::set_intersect(mr->meshes(), result);
  test(intersect == result);

  //The meshset returned from create should entirely be contained within
  //all the meshes that have 2d cells
  intersect = smtk::mesh::set_intersect(mr->meshes(smtk::mesh::Dims2), result);
  test(intersect == result);

  cleanup(mr, result);
}

void verify_create_mesh_num_cells(const smtk::mesh::ResourcePtr& mr)
{
  const std::size_t numCellsBeforeCreation = mr->cells().size();

  smtk::mesh::CellSet allNonVolumeCells =
    smtk::mesh::set_difference(mr->cells(), mr->cells(smtk::mesh::Dims3));
  smtk::mesh::MeshSet result = mr->createMesh(allNonVolumeCells);

  test(result.size() == 1, "valid cellset should create meshset with single mesh");
  test(numCellsBeforeCreation == mr->cells().size());

  cleanup(mr, result);
}

void verify_create_mesh_marks_modified()
{
  //verify that a resource loaded from file is not marked as modified
  smtk::mesh::ResourcePtr mr = load_mesh(smtk::mesh::moab::make_interface());
  test(!mr->isModified(), "resource loaded from disk shouldn't be modified");

  //verify that creating a mesh does mark update modify flag
  verify_create_mesh(mr);
  test(mr->isModified(), "resource should be marked as modified now");
}
} // namespace

int UnitTestCreateMesh(int /*unused*/, char** const /*unused*/)
{
  smtk::mesh::ResourcePtr mr = load_mesh(smtk::mesh::moab::make_interface());

  verify_create_empty_mesh(mr);
  verify_create_mesh_with_cells_from_other_resource(mr);

  verify_create_mesh(mr);

  verify_create_mesh_num_meshes(mr);
  verify_create_mesh_updated_mesh_queries(mr);

  verify_create_mesh_num_cells(mr);

  verify_create_mesh_marks_modified();

  return 0;
}
