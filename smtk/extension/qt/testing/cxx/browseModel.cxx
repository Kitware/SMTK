#include "smtk/extension/qt/qtEntityItemDelegate.h"
#include "smtk/extension/qt/qtEntityItemModel.h"

#include "smtk/model/ImportJSON.h"
#include "smtk/model/ExportJSON.h"
#include "smtk/model/Manager.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/util/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/extension/qt/testing/cxx/ModelBrowser.h"

#include <QtGui/QApplication>
#include <QtGui/QTreeView>
#include <QtGui/QHeaderView>

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
  smtk::model::ImportJSON::intoModel(json.c_str(), model);
  model->assignDefaultNames();

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
  smtk::model::Cursors cursors;
  smtk::model::Cursor::CursorsFromUUIDs(
    cursors, model, model->entitiesMatchingFlags(mask, false));
  std::cout << std::setbase(10) << "Found " << cursors.size() << " entries\n";
  view->setup(
    model,
    qmodel,
    qdelegate,
    smtk::model::EntityListPhrase::create()
      ->setup(cursors)
      ->setDelegate( // set the subphrase generator:
        smtk::model::SimpleModelSubphrases::create()));
  test(cursors.empty() || qmodel->manager() == model,
    "Failed to obtain Manager from QEntityItemModel.");

  // Enable user sorting.
  view->tree()->setSortingEnabled(true);

  view->show();

  // FIXME: Actually test something when not in debug mode.
  int status = debug ? app.exec() : 0;
  if (argc > 4)
    {
    std::ofstream result(argv[4]);
    result << smtk::model::ExportJSON::fromModel(model).c_str() << "\n";
    result.close();
    }

  delete view;
  delete qmodel;
  delete qdelegate;

  return status;
}
