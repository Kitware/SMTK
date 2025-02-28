//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ModelToMesh.h"
#include "smtk/io/WriteMesh.h"

#include "smtk/mesh/core/Resource.h"

#include "smtk/model/EntityIterator.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Volume.h"

#include "smtk/mesh/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <sstream>

namespace
{

class CountCells : public smtk::mesh::CellForEach
{
  //keep the range of points we have seen so we can verify that we
  //seen all the cells that we expect to be given
  smtk::mesh::HandleRange pointsSeen;

  //keep a physical count of number of cells so that we can verify we
  //don't iterate over a cell more than once
  int numCellsVisited{ 0 };

  //total number of points seen, relates to the total size of the connectivity
  //array for this cellset
  int numPointsSeen{ 0 };

public:
  CountCells() = default;

  void forCell(const smtk::mesh::Handle& cellId, smtk::mesh::CellType cellType, int numPts) override
  {
    (void)cellId;
    (void)cellType;
    this->numCellsVisited++;
    this->numPointsSeen += numPts;
    this->pointsSeen.insert(
      smtk::mesh::HandleInterval(*this->pointIds(), *(this->pointIds() + numPts - 1)));
  }

  [[nodiscard]] int numberOCellsVisited() const { return numCellsVisited; }
  [[nodiscard]] int numberOPointsSeen() const { return numPointsSeen; }

  [[nodiscard]] smtk::mesh::HandleRange points() const { return pointsSeen; }
};

std::size_t numTetsInModel = 4;

void create_simple_model(smtk::model::ResourcePtr resource)
{
  using namespace smtk::model::testing;

  smtk::model::SessionPtr session = smtk::model::Session::create();
  smtk::model::SessionRef sess(resource, session);
  smtk::model::Model model = resource->addModel();

  for (std::size_t i = 0; i < numTetsInModel; ++i)
  {
    smtk::common::UUIDArray uids = createTet(resource);
    model.addCell(smtk::model::Volume(resource, uids[21]));
  }
  model.setSession(sess);
  resource->assignDefaultNames();
}

void verify_empty_model()
{
  smtk::model::ResourcePtr modelResource = smtk::model::Resource::create();

  smtk::io::ModelToMesh convert;
  smtk::mesh::ResourcePtr mr = convert(modelResource);
  test(!mr, "mesh resource should be invalid for an empty model");
}

void verify_model_association()
{
  smtk::model::ResourcePtr modelResource = smtk::model::Resource::create();

  create_simple_model(modelResource);

  smtk::io::ModelToMesh convert;
  smtk::mesh::ResourcePtr mr = convert(modelResource);

  //we need to verify that the mesh resource is now has an associated model
  test(mr->hasAssociations(), "mesh resource should have associations");
  test(
    (mr->associatedModel() != smtk::common::UUID()),
    "mesh resource should be associated to a real model");
  test((mr->isAssociatedToModel()), "mesh resource should be associated to a real model");
  test(mr->isModified(), "A mesh created in memory with no file is considered modified");

  //verify the MODEL_ENTITY is correct
  smtk::model::EntityRefs currentModels =
    modelResource->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::MODEL_ENTITY);
  if (!currentModels.empty())
  { //presuming only a single model in the model resource
    test(
      (mr->associatedModel() == currentModels.begin()->entity()),
      "mesh resource associated model should match model resource");
  }
}

template<int Dim>
void testFindAssociations(
  smtk::mesh::ResourcePtr mr,
  smtk::model::EntityIterator& it,
  std::size_t correct)
{
  std::size_t numNonEmpty = 0;
  numNonEmpty = 0;
  for (it.begin(); !it.isAtEnd(); ++it)
  {
    smtk::mesh::MeshSet entMesh =
      mr->findAssociatedMeshes(*it, static_cast<smtk::mesh::DimensionType>(Dim));
    if (entMesh.size())
    {
      ++numNonEmpty;
      std::cout << "  " << it->entity().toString() << "  " << entMesh.size() << " sets "
                << entMesh.cells().size() << " cells"
                << "  " << it->flagSummary(0) << "\n";
    }
    /*
    test(
      (tess && tess->begin() != tess->end() && entMesh.size()) ||
      ((!tess || tess->begin() == tess->end()) && !entMesh.size()),
      "Model and mesh do not agree.");
      */
  }
  std::ostringstream msg;
  msg << "Expected " << (correct ? 1 : 0) << " non-empty meshset of dimension " << Dim
      << " per test tetrahedron.";
  test(numNonEmpty == correct, msg.str());
}

template<>
void testFindAssociations<-1>(
  smtk::mesh::ResourcePtr mr,
  smtk::model::EntityIterator& it,
  std::size_t correct)
{
  std::size_t numNonEmpty = 0;
  for (it.begin(); !it.isAtEnd(); ++it)
  {
    smtk::mesh::MeshSet entMesh = mr->findAssociatedMeshes(*it);
    const smtk::model::Tessellation* tess = it->hasTessellation();
    if (entMesh.size())
      std::cout << "  " << it->entity().toString() << "  " << entMesh.size() << " sets "
                << entMesh.cells().size() << " cells"
                << "  " << it->flagSummary(0) << "\n";
    /*
      */
    test(
      (tess && tess->begin() != tess->end() && entMesh.size()) ||
        ((!tess || tess->begin() == tess->end()) && !entMesh.size()),
      "Model and mesh do not agree.");
    if (entMesh.size())
      ++numNonEmpty;
  }
  test(numNonEmpty == correct, "Expected a non-empty meshset per test tetrahedron.");
}

template<int Dim>
void testFindAssociationsByRef(
  smtk::mesh::ResourcePtr mr,
  smtk::model::EntityIterator& it,
  std::size_t correct)
{
  smtk::mesh::MeshSet entMesh =
    mr->findAssociatedMeshes(it, static_cast<smtk::mesh::DimensionType>(Dim));
  smtk::mesh::CellSet entCells =
    mr->findAssociatedCells(it, static_cast<smtk::mesh::DimensionType>(Dim));

  test(entMesh.cells() == entCells, "Expected mesh cellset to be the same as queried cellset.");

  std::ostringstream msg;
  msg << "Expected " << !entMesh.is_empty() << " non-empty meshset of dimension " << Dim
      << " for all tetrahedra.";
  test(entMesh.size() == correct, msg.str());
}

template<>
void testFindAssociationsByRef<-1>(
  smtk::mesh::ResourcePtr mr,
  smtk::model::EntityIterator& it,
  std::size_t correct)
{
  smtk::mesh::MeshSet entMesh = mr->findAssociatedMeshes(it);
  smtk::mesh::CellSet entCells = mr->findAssociatedCells(it);
  smtk::mesh::TypeSet entTypes = mr->findAssociatedTypes(it);
  const smtk::model::Tessellation* tess = it->hasTessellation();
  (void)tess;

  test(entMesh.cells() == entCells, "Expected mesh cellset to be the same as queried cellset.");

  test(entMesh.types() == entTypes, "Expected mesh typeset to be the same as queried typeset.");

  test(entMesh.size() == correct, "Expected a non-empty meshset for all tetrahedra.");
}

void verify_cell_conversion()
{
  smtk::model::ResourcePtr modelResource = smtk::model::Resource::create();

  create_simple_model(modelResource);

  smtk::io::ModelToMesh convert;
  smtk::mesh::ResourcePtr mr = convert(modelResource);
  test(mr->isValid(), "mesh resource should be valid");
  test(mr->numberOfMeshes() == numTetsInModel, "mesh resource should have a mesh per tet");

  //confirm that we have the proper number of volume cells
  smtk::mesh::CellSet tri_cells = mr->cells(smtk::mesh::Dims2);
  test(tri_cells.size() == (numTetsInModel * 10));

  smtk::mesh::CellSet edge_cells = mr->cells(smtk::mesh::Dims1);
  test(edge_cells.size() == 0);

  smtk::mesh::CellSet vert_cells = mr->cells(smtk::mesh::Dims0);
  test(vert_cells.size() == 0);

  // verify that we get a non-empty mesh set association back for some cells
  smtk::model::EntityIterator it;
  smtk::model::EntityRefs models =
    modelResource->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::MODEL_ENTITY);
  it.traverse(models.begin(), models.end(), smtk::model::ITERATE_MODELS);
  std::cout << "All associations:\n";
  testFindAssociations<-1>(mr, it, numTetsInModel);
  std::cout << "Dim 0 associations:\n";
  testFindAssociations<0>(mr, it, 0);
  std::cout << "Dim 1 associations:\n";
  testFindAssociations<1>(mr, it, 0);
  std::cout << "Dim 2 associations:\n";
  testFindAssociations<2>(mr, it, numTetsInModel);
  std::cout << "Dim 3 associations:\n";
  testFindAssociations<3>(mr, it, 0);

  {
    it.traverse(models.begin(), models.end(), smtk::model::ITERATE_MODELS);
    std::cout << "All associations:\n";
    testFindAssociationsByRef<-1>(mr, it, numTetsInModel);
    std::cout << "Dim 0 associations:\n";
    testFindAssociationsByRef<0>(mr, it, 0);
    std::cout << "Dim 1 associations:\n";
    testFindAssociationsByRef<1>(mr, it, 0);
    std::cout << "Dim 2 associations:\n";
    testFindAssociationsByRef<2>(mr, it, numTetsInModel);
    std::cout << "Dim 3 associations:\n";
    testFindAssociationsByRef<3>(mr, it, 0);
  }

  {
    std::cout << "Querying an empty mesh resource:\n";
    it.traverse(models.begin(), models.end(), smtk::model::ITERATE_MODELS);
    std::cout << "All associations:\n";
    smtk::mesh::ResourcePtr emptyResource = smtk::mesh::Resource::create();
    testFindAssociationsByRef<-1>(emptyResource, it, 0);
  }

  std::cout << "Find type info of first cell:\n";
  if (!models.empty())
  {
    smtk::model::CellEntities cells = models.begin()->as<smtk::model::Model>().cells();
    if (!cells.empty())
    {
      smtk::mesh::TypeSet meshedTypes = mr->findAssociatedTypes(cells[0]);
      smtk::mesh::CellTypes cellTypes = meshedTypes.cellTypes();

      //the model we have been given only has triangles, and while those
      //triangles has physical coordinates we don't model each of those coordinates
      //as an explicit cell vertex. That is why we only expect to find triangle
      //cells in the following check
      std::cout << "  Cell " << cells[0].name() << "\n";
      static bool expected[smtk::mesh::CellType_MAX] = { false, false, true,  false, false,
                                                         false, false, false, false };
      for (int i = 0; i < smtk::mesh::CellType_MAX; ++i)
      {
        smtk::mesh::CellType ct = static_cast<smtk::mesh::CellType>(i);
        std::cout << "    Has mesh items of type " << cellTypeSummary(ct) << "? "
                  << (cellTypes[ct] ? "Y" : "N") << "\n";
        if (cellTypes[ct] != expected[ct])
        {
          std::ostringstream msg;
          msg << (expected[i] ? "Expected" : "Did not expect") << " this cell to have mesh "
              << cellTypeSummary(ct, 1);
          std::cout << "    " << msg.str() << "\n";
          test(cellTypes[ct] == expected[ct], msg.str());
        }
      }
    }
  }

  std::cout << "Entity lookup via reverse classification\n";
  smtk::model::EntityRefArray ents;
  bool entsAreValid = mr->meshes().modelEntities(ents);
  test(entsAreValid, "Expected valid entity refs.");
  test(ents.size() == numTetsInModel, "Expected 1 tetrahedron per model.");
  for (smtk::model::EntityRefArray::iterator eit = ents.begin(); eit != ents.end(); ++eit)
  {
    std::cout << "  " << eit->name() << " (" << eit->flagSummary(0) << ")\n";
  }
}

void verify_vertex_conversion()
{
  smtk::model::ResourcePtr modelResource = smtk::model::Resource::create();

  create_simple_model(modelResource);

  smtk::io::ModelToMesh convert;
  smtk::mesh::ResourcePtr mr = convert(modelResource);
  test(mr->isValid(), "mesh resource should be valid");
  test(mr->numberOfMeshes() == numTetsInModel, "mesh resource should have a mesh per tet");

  smtk::mesh::PointSet points = mr->points();
  test(points.size() == 7, "After merging of identical points we should have 7");
}

void verify_cell_have_points()
{
  smtk::model::ResourcePtr modelResource = smtk::model::Resource::create();

  create_simple_model(modelResource);

  smtk::io::ModelToMesh convert;
  smtk::mesh::ResourcePtr mr = convert(modelResource);
  smtk::mesh::MeshSet triMeshes = mr->meshes(smtk::mesh::Dims2);

  CountCells functor;
  smtk::mesh::for_each(triMeshes.cells(), functor);

  test(
    functor.numberOCellsVisited() == static_cast<int>(triMeshes.cells().size()),
    "Mismatch in number of cells visited.");
  test(
    functor.numberOPointsSeen() == static_cast<int>(triMeshes.pointConnectivity().size()),
    "Mismatch in number of points seen.");

  //currently no two cells are sharing vertices.
  test(
    functor.numberOCellsVisited() == static_cast<int>(numTetsInModel * 10),
    "Number of cells not proportional to number of tets.");
  test(
    functor.numberOPointsSeen() == static_cast<int>(numTetsInModel * 10 * 3),
    "Number of points not proportional to number of tets.");
}
} // namespace

int UnitTestModelToMesh3D(int /*unused*/, char** const /*unused*/)
{
  verify_empty_model();
  verify_model_association();
  verify_cell_conversion();
  verify_vertex_conversion();
  verify_cell_have_points();

  return 0;
}
