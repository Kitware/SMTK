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
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Volume.h"
#include "smtk/model/EntityIterator.h"

#include "smtk/model/testing/cxx/helpers.h"
#include "smtk/mesh/testing/cxx/helpers.h"

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
  void operator()(int numPts,
                  const smtk::mesh::Handle* const pointIds,
                  const double* const coords)
  {
  this->numCellsVisited++;
  this->numPointsSeen += numPts;
  this->pointsSeen.insert( pointIds, pointIds+numPts);
  }

  int numberOCellsVisited() const { return numCellsVisited; }
  int numberOPointsSeen() const { return numPointsSeen; }

  smtk::mesh::HandleRange points() const { return pointsSeen; }
};


std::size_t numTetsInModel = 32;

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
  test( tri_cells.size() == (numTetsInModel * 11) );

  smtk::mesh::CellSet edge_cells = c->cells( smtk::mesh::Dims1 );
  test( edge_cells.size() == 0);

  smtk::mesh::CellSet vert_cells = c->cells( smtk::mesh::Dims0 );
  test( vert_cells.size() == 0);

  // verify that we get a non-empty mesh set association back for some cells
  smtk::model::EntityIterator it;
  smtk::model::EntityRefs models =
    modelManager->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::MODEL_ENTITY);
  it.traverse(models.begin(), models.end(), smtk::model::ITERATE_MODELS);
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    smtk::mesh::MeshSet entMesh = c->findAssociatedMeshes(*it);
    std::cout
      << "  " << it->entity().toString()
      << "  " << entMesh.size() << " sets " << entMesh.cells().size() << " cells"
      << "  " << it->flagSummary(0)
      << "\n";
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

  test( functor.numberOCellsVisited() == triMeshes.cells().size() );
  test( functor.numberOPointsSeen() == triMeshes.pointConnectivity().size() );

  //currently no two cells are sharing vertices.
  test( functor.numberOCellsVisited() == (numTetsInModel * 11)  );
  test( functor.numberOPointsSeen() == (numTetsInModel * 11 * 3) );
}

}

//----------------------------------------------------------------------------
int UnitTestModelToMesh(int argc, char** argv)
{

  verify_null_managers();

  verify_empty_model();

  verify_cell_conversion();

  verify_vertex_conversion();

  verify_cell_have_points();

  return 0;
}
