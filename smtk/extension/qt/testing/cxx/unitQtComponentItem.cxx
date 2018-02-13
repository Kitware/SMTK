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
#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/View.h"
#include "smtk/view/VisibilityContent.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/testing/cxx/helpers.h"

#include "smtk/environment/Environment.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/LoadResource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/model/SessionRef.h"

#include "smtk/io/LoadJSON.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/AutoInit.h"

#include <QApplication>
#include <QComboBox>
#include <QListView>
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
  auto rsrcMgr = smtk::environment::ResourceManager::instance();
  auto operMgr = smtk::environment::OperationManager::instance();
  auto view = smtk::view::View::New("ComponentItem", "stuff");
  auto phraseModel = smtk::view::ResourcePhraseModel::create(view);
  std::map<smtk::common::UUID, int> m_visibleThings;
  phraseModel->addSource(rsrcMgr, operMgr);
  phraseModel->setDecorator(
    [&m_visibleThings](smtk::view::DescriptivePhrasePtr phr)
    {
      smtk::view::VisibilityContent::decoratePhrase(phr,
        [&m_visibleThings](
          smtk::view::VisibilityContent::Query qq,
          int val,
          smtk::view::ConstPhraseContentPtr data)
        {
          smtk::model::EntityPtr ent =
            data ? std::dynamic_pointer_cast<smtk::model::Entity>(data->relatedComponent()) :
            nullptr;
          smtk::model::ManagerPtr mmgr =
            ent ?
              ent->modelResource() :
              (data ?
                std::dynamic_pointer_cast<smtk::model::Manager>(data->relatedResource()) :
                nullptr);

          switch (qq)
          {
          case smtk::view::VisibilityContent::DISPLAYABLE:
            return (ent || (!ent && mmgr)) ? 1 : 0;
          case smtk::view::VisibilityContent::EDITABLE:
            return (ent || (!ent && mmgr)) ? 1 : 0;
          case smtk::view::VisibilityContent::GET_VALUE:
            if (ent)
            {
              auto valIt = m_visibleThings.find(ent->id());
              if (valIt != m_visibleThings.end())
              {
                return valIt->second;
              }
              return 0; // membership is only assumed if there is an entry.
            }
            return 0; // membership is false if the component is not a model entity or NULL.
          case smtk::view::VisibilityContent::SET_VALUE:
            if (ent)
            {
              m_visibleThings[ent->id()] = val ? 1 : 0;
              return 1;
            }
          }
        return 0;
        });
      return 0;
    }
  );

  auto oper = operMgr->create<smtk::operation::LoadResource>();
  if (!oper)
  {
    std::cout << "No load operator\n";
    return 1;
  }

  oper->parameters()->findFile("filename")->setValue(argv[1]);
  auto result = oper->operate();
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cout << "Load operator failed\n";
    return 2;
  }

  auto rsrc = result->findResource("resource")->value(0);
  auto modelRsrc = std::dynamic_pointer_cast<smtk::model::Manager>(rsrc);
  if (!modelRsrc)
  {
    std::cout << "Load operator succeeded but had empty output\n";
    return 4;
  }
  auto qmodel = new qtDescriptivePhraseModel;

  QApplication app(argc, argv);

  auto dlog = new QWidget;
  auto combo = new QComboBox(dlog);
  auto layout = new QVBoxLayout(dlog);
  layout->addWidget(combo);
  qmodel->setPhraseModel(phraseModel);
  combo->setModel(qmodel);
  QModelIndex comboRoot =
    qmodel->index(4, 0, qmodel->index(0, 0, qmodel->index(0, 0, QModelIndex())));
  std::cout << " root is " << qmodel->data(comboRoot).toString().toStdString() << "\n";
  QListView* listView = new QListView(combo);
  auto delegate = new smtk::extension::qtDescriptivePhraseDelegate;
  delegate->setTextVerticalPad(6);
  delegate->setTitleFontWeight(1);
  delegate->setDrawSubtitle(false);

  listView->setItemDelegate(delegate);
  combo->setView(listView);
  combo->setRootModelIndex(comboRoot);
  QObject::connect(delegate, SIGNAL(requestVisibilityChange(const QModelIndex&)), qmodel,
    SLOT(toggleVisibility(const QModelIndex&)));

  dlog->show();

  return app.exec();
}
