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

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Volume.h"
#include "smtk/model/EntityIterator.h"

#include "smtk/model/testing/cxx/helpers.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include <sstream>

namespace
{


//----------------------------------------------------------------------------
class CountCells : public smtk::mesh::CellForEach
{
  //keep the range of points we have seen so we can verify that we
  //seen all the cells that we expect to be given
  smtk::mesh::HandleRange pointsSeen;

  //keep a physical count of number of cells so that we can verify we
  //don't iterate over a cell more than once
  int numCellsVisited;

  //total number of points seen, relates to the total size of the connectivity
  //array for this cellset
  int numPointsSeen;
public:
  //--------------------------------------------------------------------------
  CountCells( ):
    smtk::mesh::CellForEach(),
    pointsSeen( ),
    numCellsVisited(0),
    numPointsSeen(0)
    {
    }

  //--------------------------------------------------------------------------
  void operator()(smtk::mesh::CellType& cellType,
                  int numPts,
                  const smtk::mesh::Handle* const pointIds,
                  const double* const coords)
  {
  (void)cellType;
  (void)coords;
  this->numCellsVisited++;
  this->numPointsSeen += numPts;
  this->pointsSeen.insert( pointIds, pointIds+numPts);
  }

  int numberOCellsVisited() const { return numCellsVisited; }
  int numberOPointsSeen() const { return numPointsSeen; }

  smtk::mesh::HandleRange points() const { return pointsSeen; }
};


std::size_t numTetsInModel = 4;

//----------------------------------------------------------------------------
void create_simple_model( smtk::model::ManagerPtr mgr )
{
  using namespace smtk::model::testing;

  smtk::model::SessionRef sess = mgr->createSession("native");
  smtk::model::Model model = mgr->addModel();

  for(std::size_t i=0; i < numTetsInModel; ++i)
    {
    smtk::common::UUIDArray uids = createTet(mgr);
    model.addCell( smtk::model::Volume(mgr, uids[21]));
    }
  model.setSession(sess);
  mgr->assignDefaultNames();

}

//----------------------------------------------------------------------------
void verify_null_managers()
{
  smtk::mesh::ManagerPtr null_meshManager;
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();

  smtk::model::ManagerPtr null_modelManager;
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  smtk::io::ModelToMesh convert;

  {
  smtk::mesh::CollectionPtr c = convert(null_meshManager, null_modelManager);
  test( !c, "collection should be invalid for a NULL managers");
  }

  {
  smtk::mesh::CollectionPtr c = convert(null_meshManager, modelManager);
  test( !c, "collection should be invalid for a NULL mesh manager");
  }

  {
  smtk::mesh::CollectionPtr c = convert(meshManager, null_modelManager);
  test( !c, "collection should be invalid for a NULL model manager");
  }
}

//----------------------------------------------------------------------------
void verify_empty_model()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  test( !c, "collection should be invalid for an empty model");
}

//----------------------------------------------------------------------------
template<int Dim>
void testFindAssociations(smtk::mesh::CollectionPtr c, smtk::model::EntityIterator& it, std::size_t correct)
{
  std::size_t numNonEmpty = 0;
  numNonEmpty = 0;
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    smtk::mesh::MeshSet entMesh = c->findAssociatedMeshes(*it, static_cast<smtk::mesh::DimensionType>(Dim));
    if (entMesh.size())
      {
      ++numNonEmpty;
      std::cout
        << "  " << it->entity().toString()
        << "  " << entMesh.size() << " sets " << entMesh.cells().size() << " cells"
        << "  " << it->flagSummary(0)
        << "\n";
      }
    /*
    test(
      (tess && tess->begin() != tess->end() && entMesh.size()) ||
      ((!tess || tess->begin() == tess->end()) && !entMesh.size()),
      "Model and mesh do not agree.");
      */
    }
  std::ostringstream msg;
  msg
    << "Expected " << (correct ? 1 : 0)
    << " non-empty meshset of dimension " << Dim
    << " per test tetrahedron.";
  test(numNonEmpty == correct, msg.str());
}

template<>
void testFindAssociations<-1>(smtk::mesh::CollectionPtr c, smtk::model::EntityIterator& it, std::size_t correct)
{
  std::size_t numNonEmpty = 0;
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    smtk::mesh::MeshSet entMesh = c->findAssociatedMeshes(*it);
    const smtk::model::Tessellation* tess = it->hasTessellation();
    if (entMesh.size())
      std::cout
        << "  " << it->entity().toString()
        << "  " << entMesh.size() << " sets " << entMesh.cells().size() << " cells"
        << "  " << it->flagSummary(0)
        << "\n";
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

//----------------------------------------------------------------------------
void verify_cell_conversion()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  test( c->isValid(), "collection should be valid");
  test( c->numberOfMeshes() == numTetsInModel, "collection should have a mesh per tet");

  //confirm that we have the proper number of volume cells
  smtk::mesh::CellSet tri_cells = c->cells( smtk::mesh::Dims2 );
  test( tri_cells.size() == (numTetsInModel * 10) );

  smtk::mesh::CellSet edge_cells = c->cells( smtk::mesh::Dims1 );
  test( edge_cells.size() == 0);

  smtk::mesh::CellSet vert_cells = c->cells( smtk::mesh::Dims0 );
  test( vert_cells.size() == 0);

  // verify that we get a non-empty mesh set association back for some cells
  smtk::model::EntityIterator it;
  smtk::model::EntityRefs models =
    modelManager->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::MODEL_ENTITY);
  it.traverse(models.begin(), models.end(), smtk::model::ITERATE_MODELS);
  std::cout << "All associations:\n";
  testFindAssociations<-1>(c, it, numTetsInModel);
  std::cout << "Dim 0 associations:\n";
  testFindAssociations<0>(c, it, 0);
  std::cout << "Dim 1 associations:\n";
  testFindAssociations<1>(c, it, 0);
  std::cout << "Dim 2 associations:\n";
  testFindAssociations<2>(c, it, numTetsInModel);
  std::cout << "Dim 3 associations:\n";
  testFindAssociations<3>(c, it, 0);

  std::cout << "Find type info of first cell:\n";
  if (!models.empty())
    {
    smtk::model::CellEntities cells = models.begin()->as<smtk::model::Model>().cells();
    if (!cells.empty())
      {
      smtk::mesh::TypeSet meshedTypes = c->findAssociatedTypes(cells[0]);
      smtk::mesh::CellTypes cellTypes = meshedTypes.cellTypes();

      //the model we have been given only has triangles, and while those
      //triangles has physical coordinates we don't model each of those coordinates
      //as an explicit cell vertex. That is why we only expect to find triangle
      //cells in the following check
      std::cout << "  Cell " << cells[0].name() << "\n";
      static bool expected[smtk::mesh::CellType_MAX] =
        {false, false, true, false, false, false, false, false, false};
      for (int i = 0; i < smtk::mesh::CellType_MAX; ++i)
        {
        smtk::mesh::CellType ct = static_cast<smtk::mesh::CellType>(i);
        std::cout
          << "    Has mesh items of type " << cellTypeSummary(ct) << "? "
          << (cellTypes[ct] ? "Y" : "N") << "\n";
        if (cellTypes[ct] != expected[ct])
          {
          std::ostringstream msg;
          msg
            << (expected[i] ? "Expected" : "Did not expect")
            << " this cell to have mesh " << cellTypeSummary(ct,1);
          std::cout << "    " << msg.str() << "\n";
          test(cellTypes[ct] == expected[ct], msg.str());
          //smtk::io::WriteMesh::entireCollection("/tmp/mesh.vtk", c);
          }
        }
      }
    }

  std::cout << "Entity lookup via reverse classification\n";
  smtk::model::EntityRefArray ents = c->meshes().modelEntities();
  test(ents.size() == numTetsInModel, "Expected 1 tetrahedron per model.");
  for (smtk::model::EntityRefArray::iterator eit = ents.begin(); eit != ents.end(); ++eit)
    {
    std::cout << "  " << eit->name() << " (" << eit->flagSummary(0) << ")\n";
    }
}

//----------------------------------------------------------------------------
void verify_vertex_conversion()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  test( c->isValid(), "collection should be valid");
  test( c->numberOfMeshes() == numTetsInModel, "collection should have a mesh per tet");

  //make sure we have the proper number of meshsets
  smtk::mesh::CellSet vert_cells = c->cells( smtk::mesh::Dims0 );
  // test( vert_cells.size() == 224, "Should only have 224 vertices");

}

//----------------------------------------------------------------------------
void verify_cell_have_points()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);
  smtk::mesh::MeshSet triMeshes = c->meshes( smtk::mesh::Dims2 );

  CountCells functor;
  smtk::mesh::for_each( triMeshes.cells(), functor );

  test(functor.numberOCellsVisited() == static_cast<int>(triMeshes.cells().size()), "Mismatch in number of cells visited.");
  test(functor.numberOPointsSeen() == static_cast<int>(triMeshes.pointConnectivity().size()), "Mismatch in number of points seen.");

  //currently no two cells are sharing vertices.
  test(
    functor.numberOCellsVisited() == static_cast<int>(numTetsInModel * 10),
    "Number of cells not proportional to number of tets.");
  test(
    functor.numberOPointsSeen() == static_cast<int>(numTetsInModel * 10 * 3),
    "Number of points not proportional to number of tets." );
}

}

//----------------------------------------------------------------------------
int UnitTestModelToMesh(int, char**)
{
  verify_null_managers();
  verify_empty_model();
  verify_cell_conversion();
  verify_vertex_conversion();
  verify_cell_have_points();

  return 0;
}
