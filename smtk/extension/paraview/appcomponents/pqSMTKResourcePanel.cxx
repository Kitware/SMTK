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

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QPointer>

class pqSMTKResourcePanel::Internal : public Ui::pqSMTKResourcePanel
{
public:
  Internal()
    : m_selnSource("resource panel")
  {
  }

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
    QObject::connect(m_view->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), parent,
      SLOT(sendPanelSelectionToSMTK(const QItemSelection&, const QItemSelection&)));
  }

  QPointer<smtk::extension::qtDescriptivePhraseModel> m_model;
  QPointer<smtk::extension::qtDescriptivePhraseDelegate> m_delegate;
  std::map<smtk::resource::ManagerPtr, int> m_observers;
  smtk::view::ResourcePhraseModel::Ptr m_phraseModel;
  smtk::view::Selection::Ptr m_seln; // TODO: This assumes there is only 1 server connection
  int m_selnHandle;                  // TODO: Same assumption as m_seln
  std::string m_selnLabel;
  std::string
    m_selnSource; // TODO: This assumes there is only 1 panel (or that all panels should share)
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

void pqSMTKResourcePanel::sendPanelSelectionToSMTK(const QItemSelection&, const QItemSelection&)
{
  if (!m_p->m_seln)
  {
    return;
  } // No SMTK selection exists.

  int selnValue = m_p->m_seln->findOrCreateLabeledValue(m_p->m_selnLabel);
  //smtk::view::Selection::SelectionMap selnMap;
  std::set<smtk::resource::Component::Ptr> selnSet;
  auto selected = m_p->m_view->selectionModel()->selection();
  for (auto qslist : selected.indexes())
  {
    auto phrase = m_p->m_model->getItem(qslist);
    smtk::resource::Component::Ptr comp;
    if (phrase && (comp = phrase->relatedComponent()))
    {
      selnSet.insert(comp);
    }
  }
  m_p->m_seln->modifySelection(
    selnSet, m_p->m_selnSource, selnValue, smtk::view::SelectionAction::UNFILTERED_REPLACE);
}

// FIXME: Doesn't most of this belong in PhraseModel and/or qtDescriptivePhraseModel?
void pqSMTKResourcePanel::sendSMTKSelectionToPanel(
  const std::string& src, smtk::view::SelectionPtr seln)
{
  if (src == m_p->m_selnSource)
  {
    return;
  } // Ignore selections generated from this panel.
  auto qview = m_p->m_view;
  auto qmodel = m_p->m_model;
  auto root = m_p->m_phraseModel->root();
  QItemSelection qseln;
  root->visitChildren(
    [&qmodel, &qseln, &seln](smtk::view::DescriptivePhrasePtr phrase, std::vector<int>& path) {
      auto comp = phrase->relatedComponent();
      if (comp)
      {
        if (seln->currentSelection().find(comp) != seln->currentSelection().end())
        {
          auto qidx = qmodel->indexFromPath(path);
          qseln.select(qidx, qidx);
        }
      }
      return 0;
    });
  qview->selectionModel()->select(
    qseln, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
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
  m_p->m_seln = mgr->smtkSelection();
  if (m_p->m_seln)
  {
    m_p->m_seln->registerSelectionSource(m_p->m_selnSource);
    m_p->m_selnHandle = m_p->m_seln->observe([this](const std::string& source,
      smtk::view::Selection::Ptr seln) { this->sendSMTKSelectionToPanel(source, seln); });
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
  m_p->m_seln = nullptr;
  m_p->m_phraseModel->removeSource(mgr->smtkResourceManager(), mgr->smtkOperationManager());
}
