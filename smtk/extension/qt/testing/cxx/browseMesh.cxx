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

  smtk::extension::QEntityItemModel* qmodel = new smtk::extension::QEntityItemModel;
  smtk::extension::QEntityItemDelegate* qdelegate = new smtk::extension::QEntityItemDelegate;
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
