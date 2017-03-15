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

#include "smtk/extension/qt/examples/cxx/ModelBrowser.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <QApplication>
#include <QTreeView>
#include <QHeaderView>

#include <iomanip>
#include <iostream>
#include <fstream>

#include <stdlib.h>

using namespace std;
using smtk::model::testing::hexconst;

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  const char* filename = argc > 1 ? argv[1] : "smtkModel.json";
  char* endMask;
  long mask = strtol(argc > 2 ? argv[2] : "0xffffffff", &endMask, 16);
  int debug = argc > 3 ? 1 : 0;

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


  smtk::model::ManagerPtr model = smtk::model::Manager::create();
  smtk::io::ImportJSON::intoModelManager(json.c_str(), model);
  model->assignDefaultNames();

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
  smtk::model::EntityRefs entityrefs;
  smtk::model::EntityRef::EntityRefsFromUUIDs(
    entityrefs, model, model->entitiesMatchingFlags(mask, false));
  std::cout << std::setbase(10) << "Found " << entityrefs.size() << " entries\n";
  view->setup(
    model,
    qmodel,
    qdelegate,
    smtk::model::EntityListPhrase::create()
      ->setup(entityrefs)
      ->setDelegate( // set the subphrase generator:
        smtk::model::SimpleModelSubphrases::create()));
  test(entityrefs.empty() || qmodel->manager() == model,
    "Failed to obtain Manager from QEntityItemModel.");

  // Enable user sorting.
  view->tree()->setSortingEnabled(true);

  view->show();

  // FIXME: Actually test something when not in debug mode.
  int status = debug ? app.exec() : 0;
  if (argc > 4)
    {
    std::ofstream result(argv[4]);
    result << smtk::io::ExportJSON::fromModelManager(model).c_str() << "\n";
    result.close();
    }

  delete view;
  delete qmodel;
  delete qdelegate;

  return status;
}
