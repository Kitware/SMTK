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

#include "smtk/operation/operators/ReadResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/model/Resource.h"

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
  auto operationManager = smtk::operation::Manager::create();
  auto resourceManager = smtk::resource::Manager::create();
  auto view = smtk::view::View::New("ModelBrowser", "SMTK Model");
  auto phraseModel = smtk::view::ResourcePhraseModel::create(view);
  phraseModel->addSource(resourceManager, operationManager);
  qmodel->setPhraseModel(phraseModel);
  qview->setup(resourceManager, qmodel, qdelegate, nullptr);

  auto readOp = operationManager->create<smtk::operation::ReadResource>();
  if (!readOp)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No read operator.");
    return 1;
  }

  readOp->parameters()->findFile("filename")->setValue(filename);
  smtk::operation::Operation::Result readOpResult = readOp->operate();
  if (readOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Read operator failed for \"" << filename << "\"");
    return 1;
  }

  auto rsrc = readOpResult->findResource("resource")->value(0);
  auto model = std::dynamic_pointer_cast<smtk::model::Resource>(rsrc);
  if (!model)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Did not read a model.");
    return 1;
  }
  model->assignDefaultNames();
  std::cout << "Read a " << model->typeName() << "\n";

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
