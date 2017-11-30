//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKResourcePanel.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceManager.h"

#include "smtk/extension/paraview/server/vtkSMSMTKResourceManagerProxy.h"

#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseEditor.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseList.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "ui_pqSMTKResourcePanel.h"

#include <QPointer>

class pqSMTKResourcePanel::Internal : public Ui::pqSMTKResourcePanel
{
public:
  void setup(::pqSMTKResourcePanel* parent)
  {
    QWidget* ww = new QWidget(parent);
    parent->setWindowTitle("SMTK");
    this->setupUi(ww);
    parent->setWidget(ww);
    m_phraseModel = smtk::view::ResourcePhraseModel::create();
    m_model = new smtk::extension::qtDescriptivePhraseModel;
    m_model->setPhraseModel(m_phraseModel);
    m_delegate = new smtk::extension::qtDescriptivePhraseDelegate;

    m_delegate->setTextVerticalPad(6);
    m_delegate->setTitleFontWeight(1);
    m_delegate->setDrawSubtitle(false);
    m_view->setModel(m_model);
    m_view->setItemDelegate(m_delegate);

    QObject::connect(m_searchText, SIGNAL(textChanged(const QString&)), parent,
      SLOT(searchTextChanged(const QString&)));
  }

  QPointer<smtk::extension::qtDescriptivePhraseModel> m_model;
  QPointer<smtk::extension::qtDescriptivePhraseDelegate> m_delegate;
  std::map<smtk::resource::ManagerPtr, int> m_observers;
  smtk::view::ResourcePhraseModel::Ptr m_phraseModel;
};

pqSMTKResourcePanel::pqSMTKResourcePanel(QWidget* parent)
  : Superclass(parent)
{
  m_p = new Internal;
  m_p->setup(this);
  auto spg = smtk::view::SubphraseGenerator::create();
  this->setPhraseGenerator(spg);

  auto smtkBehavior = pqSMTKBehavior::instance();
  // Listen for resources on current connections:
  smtkBehavior->visitResourceManagersOnServers(
    [this](pqSMTKResourceManager* r, pqServer* s) { this->resourceManagerAdded(r, s); });
  // Now listen for future connections.
  QObject::connect(smtkBehavior, SIGNAL(addedManagerOnServer(pqSMTKResourceManager*, pqServer*)),
    this, SLOT(resourceManagerAdded(pqSMTKResourceManager*, pqServer*)));
  QObject::connect(smtkBehavior,
    SIGNAL(removingManagerFromServer(pqSMTKResourceManager*, pqServer*)), this,
    SLOT(resourceManagerRemoved(pqSMTKResourceManager*, pqServer*)));
}

pqSMTKResourcePanel::~pqSMTKResourcePanel()
{
  delete m_p;
}

smtk::view::PhraseModelPtr pqSMTKResourcePanel::model() const
{
  return m_p->m_phraseModel;
}

smtk::view::SubphraseGeneratorPtr pqSMTKResourcePanel::phraseGenerator() const
{
  auto root = m_p->m_model->getItem(QModelIndex());
  return root ? root->findDelegate() : nullptr;
}

void pqSMTKResourcePanel::setPhraseGenerator(smtk::view::SubphraseGeneratorPtr spg)
{
  auto root = m_p->m_model->getItem(QModelIndex());
  root->setDelegate(spg);
  m_p->m_model->rebuildSubphrases(QModelIndex());
}

void pqSMTKResourcePanel::updateTopLevel()
{
  m_p->m_model->rebuildSubphrases(QModelIndex());
}

void pqSMTKResourcePanel::searchTextChanged(const QString& searchText)
{ // For now, just rebuild.
  (void)searchText;
  m_p->m_model->rebuildSubphrases(QModelIndex());
}

void pqSMTKResourcePanel::resourceManagerAdded(pqSMTKResourceManager* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }

  // mgr->smtkProxy()->UpdateVTKObjects();
  smtk::resource::ManagerPtr rsrcMgr = mgr->smtkResourceManager();
  std::cout << "Panel should watch " << rsrcMgr << " for resources\n";
  if (!rsrcMgr)
  {
    return;
  }
  m_p->m_phraseModel->addSource(mgr->smtkResourceManager(), mgr->smtkOperationManager());
}

void pqSMTKResourcePanel::resourceManagerRemoved(pqSMTKResourceManager* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }

  smtk::resource::ManagerPtr rsrcMgr = mgr->smtkResourceManager();
  if (!rsrcMgr)
  {
    return;
  }
  m_p->m_phraseModel->removeSource(mgr->smtkResourceManager(), mgr->smtkOperationManager());
}
