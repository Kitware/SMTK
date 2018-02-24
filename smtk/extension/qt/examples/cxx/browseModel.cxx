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

#include "smtk/view/PhraseModel.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/View.h"

#include "smtk/environment/Environment.h"

#include "smtk/operation/LoadResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/model/Manager.h"

#include "smtk/extension/qt/examples/cxx/ModelBrowser.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <QApplication>
#include <QHeaderView>
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
  char* endMask;
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
  auto qmodel = new smtk::extension::qtDescriptivePhraseModel;
  auto qdelegate = new smtk::extension::qtDescriptivePhraseDelegate;
  qdelegate->setTitleFontSize(12);
  qdelegate->setTitleFontWeight(2);
  qdelegate->setSubtitleFontSize(10);
  qdelegate->setSubtitleFontWeight(1);
  ModelBrowser* qview = new ModelBrowser;
  auto operationManager = smtk::environment::OperationManager::instance();
  auto resourceManager = smtk::environment::ResourceManager::instance();
  auto view = smtk::view::View::New("ModelBrowser", "SMTK Model");
  auto phraseModel = smtk::view::ResourcePhraseModel::create(view);
  phraseModel->addSource(resourceManager, operationManager);
  qmodel->setPhraseModel(phraseModel);
  qview->setup(smtk::environment::ResourceManager::instance(), qmodel, qdelegate, nullptr);

  auto loadOp = operationManager->create<smtk::operation::LoadResource>();
  if (!loadOp)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No load operator.");
    return 1;
  }

  loadOp->parameters()->findFile("filename")->setValue(filename);
  smtk::operation::Operation::Result loadOpResult = loadOp->operate();
  if (loadOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Load operator failed for \"" << filename << "\"");
    return 1;
  }

  auto rsrc = loadOpResult->findResource("resource")->value(0);
  auto model = std::dynamic_pointer_cast<smtk::model::Manager>(rsrc);
  if (!model)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Did not load a model.");
    return 1;
  }
  model->assignDefaultNames();
  std::cout << "Loaded a " << model->uniqueName() << "\n";

  // Enable user sorting.
  qview->tree()->setSortingEnabled(true);
  qview->show();

  // FIXME: Actually test something when not in debug mode.
  int status = debug ? app.exec() : 0;

  delete qview;
  delete qmodel;
  delete qdelegate;

  return status;
}
