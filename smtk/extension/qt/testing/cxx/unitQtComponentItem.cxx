//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseList.txx"
#include "smtk/view/ResourcePhrase.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/testing/cxx/helpers.h"

#include "smtk/operation/Manager.h"

#include "smtk/model/SessionRef.h"

#include "smtk/io/LoadJSON.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/AutoInit.h"

#include <QApplication>
#include <QComboBox>
#include <QVBoxLayout>
#include <QWidget>

#include <fstream>
#include <iostream>
#include <string>

#include <stdlib.h>
#include <string.h>

using namespace smtk::extension;

smtkComponentInitMacro(smtk_polygon_session);

static std::vector<char*> dataArgs;

int unitQtComponentItem(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::string testFile;
    testFile = SMTK_DATA_DIR;
    testFile += "/model/2d/smtk/epic-trex-drummer.smtk";
    dataArgs.push_back(argv[0]);
    dataArgs.push_back(strdup(testFile.c_str()));
    dataArgs.push_back(nullptr);
    argc = 2;
    argv = &dataArgs[0];
  }
  auto rsrcMgr = smtk::resource::Manager::create();
  auto operMgr = smtk::operation::Manager::create();
  auto phraseModel = smtk::view::ResourcePhraseModel::create();
  phraseModel->addSource(rsrcMgr, operMgr);
  auto rsrcs = smtk::resource::testing::loadTestResources(rsrcMgr, argc, argv);
  auto qmodel = new qtDescriptivePhraseModel;

  QApplication app(argc, argv);

  auto dlog = new QWidget;
  auto combo = new QComboBox(dlog);
  auto layout = new QVBoxLayout(dlog);
  layout->addWidget(combo);
  qmodel->setPhraseModel(phraseModel);
  combo->setModel(qmodel);
  QModelIndex comboRoot =
    qmodel->index(1, 0, qmodel->index(0, 0, qmodel->index(0, 0, QModelIndex())));
  std::cout << " root is " << qmodel->data(comboRoot).toString().toStdString() << "\n";
  combo->setRootModelIndex(comboRoot);

  dlog->show();

  return app.exec();
}
