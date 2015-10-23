//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtEntityItemDelegate.h"
#include "smtk/extension/qt/qtEntityItemModel.h"

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/model/Manager.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/SimpleModelSubphrases.h"

#include "smtk/extension/qt/testing/cxx/ModelBrowser.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <QtGui/QApplication>
#include <QtGui/QTreeView>
#include <QtGui/QHeaderView>

// Mesh related includes
#include "smtk/io/ModelToMesh.h"
#include "smtk/io/WriteMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/Volume.h"
#include "smtk/model/EntityIterator.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <stdlib.h>

using namespace std;
using smtk::model::testing::hexconst;

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
  void forCell(smtk::mesh::CellType cellType, int numPts)
  {
  (void)cellType;
  this->numCellsVisited++;
  this->numPointsSeen += numPts;
  this->pointsSeen.insert( this->pointIds(), this->pointIds()+numPts);
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
void verify_cell_conversion(
  const smtk::model::ManagerPtr& modelManager,
  const smtk::mesh::CollectionPtr& c)
{
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
void verify_vertex_conversion(
  const smtk::model::ManagerPtr& modelManager,
  const smtk::mesh::CollectionPtr& c)
{
  test( c->isValid(), "collection should be valid");
  test( c->numberOfMeshes() == numTetsInModel, "collection should have a mesh per tet");

  //make sure we have the proper number of meshsets.

  //Some bug in our point logic as 1 / 10 times the results are incorrect
  //need to figure out why
  smtk::mesh::PointSet points = c->points( );
  test( points.size() == 28, "Should be exactly 28 points in the original mesh");

  c->meshes().mergeCoincidentContactPoints();

  points = c->points( );
  test( points.size() == 7, "After merging of identical points we should have 7");
}

//----------------------------------------------------------------------------
void verify_cell_have_points(
  const smtk::model::ManagerPtr& modelManager,
  const smtk::mesh::CollectionPtr& c)
{
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

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  smtk::model::ManagerPtr model = smtk::model::Manager::create();
  smtk::mesh::ManagerPtr meshManager = model->meshes();

  char* endMask;
  long mask = strtol(argc > 1 ? argv[1] : "0xffffffff", &endMask, 16);
  int debug = argc > 2 ? 1 : 0;

  create_simple_model(model);
  model->assignDefaultNames();
/*
  const char* filename = argc > 1 ? argv[1] : "smtkModel.json";
  std::ifstream file(filename);
  if (!file.good())
    {
    cout
      << "Could not open file \"" << filename << "\".\n\n"
      << "Usage:\n  " << argv[0] << " [[[filename] mask] debug]\n"
      << "where\n"
      << "  filename is the path to a JSON model.\n"
      << "  mask     is an integer entity mask selecting what to display.\n"
      << "  debug    is any character, indicating a debug session.\n\n"
      ;
    return 1;
    }
  std::string json(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  smtk::io::ImportJSON::intoModelManager(json.c_str(), model);
  model->assignDefaultNames();
*/

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager, model);
  test( c->isValid(), "collection should be valid");
  test( c->numberOfMeshes() == numTetsInModel, "collection should have a mesh per tet");

  smtk::model::QEntityItemModel* qmodel = new smtk::model::QEntityItemModel;
  smtk::model::QEntityItemDelegate* qdelegate = new smtk::model::QEntityItemDelegate;
  qdelegate->setTitleFontSize(12);
  qdelegate->setTitleFontWeight(2);
  qdelegate->setSubtitleFontSize(10);
  qdelegate->setSubtitleFontWeight(1);
  ModelBrowser* view = new ModelBrowser;
  //QTreeView* view = new QTreeView;
  cout << "mask " << hexconst(mask) << "\n";
  /*
  smtk::model::DescriptivePhrases plist =
    smtk::model::EntityPhrase::PhrasesFromUUIDs(
      model, model->entitiesMatchingFlags(mask, false));
  std::cout << std::setbase(10) << "Found " << plist.size() << " entries\n";
  qmodel->setPhrases(plist);
  */

  smtk::model::SimpleModelSubphrases::Ptr spg =
    smtk::model::SimpleModelSubphrases::create();
  spg->setDirectLimit(-1);
  spg->setSkipAttributes(true);
  spg->setSkipProperties(false);

  mask = smtk::model::SESSION;
  debug = 1;

  smtk::model::EntityRefs entityrefs;
  smtk::model::EntityRef::EntityRefsFromUUIDs(
    entityrefs, model, model->entitiesMatchingFlags(mask, true));
  std::cout << std::setbase(10) << "Found " << entityrefs.size() << " entries\n";
  view->setup(
    model,
    qmodel,
    qdelegate,
    smtk::model::EntityListPhrase::create()
      ->setup(entityrefs)
      ->setDelegate( spg ));
  test(entityrefs.empty() || qmodel->manager() == model,
    "Failed to obtain Manager from QEntityItemModel.");

  // Enable user sorting.
  view->tree()->setSortingEnabled(true);

  view->show();

  // FIXME: Actually test something when not in debug mode.
  int status = debug ? app.exec() : 0;
  if (argc > 3)
    {
    std::ofstream result(argv[3]);
    result << smtk::io::ExportJSON::fromModelManager(model).c_str() << "\n";
    result.close();
    }

  delete view;
  delete qmodel;
  delete qdelegate;

  return status;
}
