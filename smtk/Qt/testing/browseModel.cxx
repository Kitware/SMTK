#include "smtk/Qt/qtEntityItemModel.h"

#include "smtk/model/ImportJSON.h"
#include "smtk/model/Storage.h"

#include <QtGui/QApplication>
#include <QtGui/QTreeView>

#include <iostream>
#include <fstream>

#include <stdlib.h>

using namespace std;

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


  smtk::model::StoragePtr model = smtk::model::Storage::New();
  smtk::model::ImportJSON::intoModel(json.c_str(), model);

  QEntityItemModel* qmodel = new QEntityItemModel(model);
  QTreeView* view = new QTreeView;
  view->setModel(qmodel);
  qmodel->setSubset(model->entitiesMatchingFlags(mask, false));
  view->show();

  // FIXME: Actually test something when not in debug mode.
  return debug ? app.exec() : 0;
}
