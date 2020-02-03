//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/ResourcePhraseModel.h"

#include "smtk/model/Resource.h"
#include "smtk/model/json/jsonResource.h"

#include "smtk/extension/qt/examples/cxx/ModelBrowser.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <QApplication>
#include <QHeaderView>
#include <QTimer>
#include <QTreeView>

#include <fstream>
#include <iomanip>
#include <iostream>

#include <stdlib.h>

using namespace std;
using smtk::model::testing::hexconst;

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  const char* filename = argc > 1 ? argv[1] : "smtkModel.json";
  int debug = argc > 2 ? 1 : 0;

  std::ifstream file(filename);
  if (!file.good())
  {
    cout << "Could not open file \"" << filename << "\".\n\n"
         << "Usage:\n  " << argv[0] << " [[filename] debug]\n"
         << "where\n"
         << "  filename is the path to a JSON model.\n"
         << "  debug    is any character, indicating a debug session.\n\n";
    return 1;
  }

  auto operationManager = smtk::operation::Manager::create();
  auto resourceManager = smtk::resource::Manager::create();

  std::string json_str((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
  nlohmann::json json = nlohmann::json::parse(json_str);
  auto model = smtk::model::Resource::create();
  smtk::model::from_json(json, model);

  if (!model)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Did not read a model.");
    return 1;
  }
  model->assignDefaultNames();
  resourceManager->add(model);
  std::cout << "Read a " << model->typeName() << "\n";

  auto qmodel = new smtk::extension::qtDescriptivePhraseModel;
  auto qdelegate = new smtk::extension::qtDescriptivePhraseDelegate;
  qdelegate->setTitleFontSize(12);
  qdelegate->setTitleFontWeight(2);
  qdelegate->setSubtitleFontSize(10);
  qdelegate->setSubtitleFontWeight(1);
  ModelBrowser* qview = new ModelBrowser;
  auto config = smtk::view::Configuration::New("ModelBrowser", "SMTK Model");
  auto phraseModel = smtk::view::ResourcePhraseModel::create(config);
  phraseModel->addSource(resourceManager, operationManager, nullptr, nullptr);
  qmodel->setPhraseModel(phraseModel);
  qview->setup(resourceManager, qmodel, qdelegate);

  // Enable user sorting.
  qview->tree()->setSortingEnabled(true);
  qview->show();

  // If we are in debug mode, we just want to ensure that the application
  // successfully executes.
  if (debug)
  {
    QTimer::singleShot(1000, &app, SLOT(quit()));
  }
  int status = app.exec();

  delete qview;
  delete qmodel;
  delete qdelegate;

  return status;
}
