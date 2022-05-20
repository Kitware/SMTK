//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtResourceBrowser.h"

#include "smtk/extension/qt/VisibilityBadge.h"
#include "smtk/extension/qt/qtBadgeActionToggle.h"
#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"
#include "smtk/extension/qt/qtSMTKUtilities.h"
// cmake puts the .json file contents into a static string in this header
#include "smtk/extension/qt/ResourcePanelConfiguration_json.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/Manager.h"
#include "smtk/view/PhraseModelFactory.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/TwoLevelSubphraseGenerator.h"

#include "smtk/model/Entity.h"
#include "smtk/model/Resource.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/groups/DeleterGroup.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include <QAbstractProxyModel>
#include <QColorDialog>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPointer>
#include <QTreeView>

#include "smtk/extension/qt/qtResourceBrowserP.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"

using namespace smtk::extension;

std::string qtResourceBrowser::s_configurationJSON = ResourcePanelConfiguration_json;

qtBaseView* qtResourceBrowser::createViewWidget(const smtk::view::Information& info)
{
  if (qtBaseView::validateInformation(info))
  {
    auto* view = new qtResourceBrowser(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

qtResourceBrowser::qtResourceBrowser(const smtk::view::Information& info)
  : qtBaseView(info)
{
  m_p = new Internal;
  smtk::view::PhraseModelPtr phraseModel;
  std::string modelViewType;
  QAbstractItemModel* qtPhraseModel = nullptr;
  smtk::view::SelectionPtr selection;
  const auto& view = this->configuration();
  bool searchBar = false;
  if (view)
  {
    // empty Widget attribute is OK, will use default.
    view->details().attribute("Widget", modelViewType);
    smtk::view::ManagerPtr manager = this->uiManager()->viewManager();
    phraseModel = manager->phraseModelFactory().createFromConfiguration(view.get());
    qtPhraseModel = new smtk::extension::qtDescriptivePhraseModel;
    selection = this->uiManager()->selection();
    searchBar = view->details().attributeAsBool("SearchBar");
  }
  m_p->setup(
    this, phraseModel, modelViewType, qtPhraseModel, this->parentWidget(), selection, searchBar);
  this->Widget = m_p->m_container;
}

qtResourceBrowser::~qtResourceBrowser()
{
  delete m_p;
}

QTreeView* qtResourceBrowser::createDefaultView(QWidget* parent)
{
  auto* view = new QTreeView(parent);
  view->setObjectName(QStringLiteral("m_view"));
  view->setAcceptDrops(true);
  view->setDragEnabled(true);
  view->setDragDropMode(QAbstractItemView::DragDrop);
  view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  view->setSortingEnabled(true);
  view->sortByColumn(0, Qt::AscendingOrder);

  return view;
}

QTreeView* qtResourceBrowser::view() const
{
  return m_p->m_view;
}

smtk::view::PhraseModelPtr qtResourceBrowser::phraseModel() const
{
  return m_p->m_phraseModel;
}

void qtResourceBrowser::setPhraseModel(const smtk::view::PhraseModelPtr& model)
{
  if (model == m_p->m_phraseModel)
  {
    return;
  }
  m_p->m_phraseModel = model;
  // TODO: Is this all we need?
  auto* dpmodel = m_p->descriptivePhraseModel();
  if (m_p->m_phraseModel && dpmodel)
  {
    dpmodel->setPhraseModel(m_p->m_phraseModel);
    dpmodel->rebuildSubphrases(QModelIndex());
  }
}

smtk::view::SubphraseGeneratorPtr qtResourceBrowser::phraseGenerator() const
{
  auto root = m_p->m_phraseModel ? m_p->m_phraseModel->root() : nullptr;
  return root ? root->findDelegate() : nullptr;
}

void qtResourceBrowser::setPhraseGenerator(smtk::view::SubphraseGeneratorPtr spg)
{
  auto root = m_p->m_phraseModel ? m_p->m_phraseModel->root() : nullptr;
  if (spg)
  {
    spg->setModel(m_p->m_phraseModel);
  }
  if (root)
  {
    root->setDelegate(spg);
  }
}

smtk::extension::qtDescriptivePhraseModel* qtResourceBrowser::descriptivePhraseModel() const
{
  return m_p->descriptivePhraseModel();
}

void qtResourceBrowser::setDescriptivePhraseModel(QAbstractItemModel* qmodel)
{
  m_p->setDescriptivePhraseModel(qmodel);
}

bool qtResourceBrowser::highlightOnHover() const
{
  return m_p->m_delegate->highlightOnHover();
}

void qtResourceBrowser::setHighlightOnHover(bool highlight)
{
  if (highlight == this->highlightOnHover())
  {
    return;
  }

  if (highlight)
  {
    QObject::connect(
      m_p->m_view, SIGNAL(entered(const QModelIndex&)), this, SLOT(hoverRow(const QModelIndex&)));
  }
  else
  {
    QObject::disconnect(
      m_p->m_view, SIGNAL(entered(const QModelIndex&)), this, SLOT(hoverRow(const QModelIndex&)));
    this->resetHover();
  }

  m_p->m_delegate->setHighlightOnHover(highlight);
}

void qtResourceBrowser::sendPanelSelectionToSMTK(
  const QItemSelection& /*unused*/,
  const QItemSelection& /*unused*/)
{
  if (!m_p->m_seln)
  {
    // No SMTK selection exists.
    return;
  }
  if (m_p->m_updatingPanelSelectionFromSMTK)
  {
    // Derp. Updating the SMTK selection the moment the SMTK
    // selection is sent to us could cause problems even if
    // the recursion was not infinite.
    return;
  }

  std::set<smtk::resource::PersistentObject::Ptr> selnSet;
  auto selected = m_p->m_view->selectionModel()->selection();
  for (auto qslist : selected.indexes())
  {
    auto phrase = qslist.data(qtDescriptivePhraseModel::PhrasePtrRole)
                    .value<smtk::view::DescriptivePhrasePtr>();
    smtk::resource::Component::Ptr comp;
    smtk::resource::Resource::Ptr rsrc;
    if (phrase && (comp = phrase->relatedComponent()))
    {
      selnSet.insert(comp);
    }
    else if (phrase && (rsrc = phrase->relatedResource()))
    {
      selnSet.insert(rsrc);
    }
  }
  m_p->m_seln->modifySelection(
    selnSet, m_p->m_selnSource, m_p->m_selnValue, smtk::view::SelectionAction::UNFILTERED_REPLACE);
}

// FIXME: Doesn't most of this belong in PhraseModel and/or qtDescriptivePhraseModel?
void qtResourceBrowser::sendSMTKSelectionToPanel(
  const std::string& src,
  smtk::view::SelectionPtr seln)
{
  if (src == m_p->m_selnSource)
  {
    // Ignore selections generated from this panel.
    return;
  }
  auto* qview = m_p->m_view;
  auto* qmodel = m_p->descriptivePhraseModel();
  auto root = m_p->m_phraseModel->root();
  QItemSelection qseln;
  if (root)
  {
    root->visitChildren(
      [&qmodel, &qseln, &seln](smtk::view::DescriptivePhrasePtr phrase, std::vector<int>& path) {
        smtk::resource::PersistentObjectPtr obj = phrase->relatedObject();
        if (obj)
        {
          auto it = seln->currentSelection().find(obj);
          if (it != seln->currentSelection().end() && (it->second & 0x01))
          {
            auto qidx = qmodel->indexFromPath(path);
            qseln.select(qidx, qidx);
          }
        }
        return 0;
      });
  }
  auto* smodel = dynamic_cast<QAbstractProxyModel*>(qview->selectionModel()->model());
  // If our top-level model is a proxy model, map the selected
  // indices from the descriptive phrase space into the proxy's
  // space.
  if (smodel)
  {
    qseln = smodel->mapSelectionFromSource(qseln);
  }

  // Now update the Qt selection, being careful not to trigger SMTK updates:
  m_p->m_updatingPanelSelectionFromSMTK = true;
  qview->selectionModel()->select(
    qseln, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  m_p->m_updatingPanelSelectionFromSMTK = false;
}

void qtResourceBrowser::addSource(smtk::common::TypeContainer& managers)
{
  m_p->m_seln = managers.get<smtk::view::SelectionPtr>();
  if (m_p->m_seln)
  {
    m_p->m_selnValue = m_p->m_seln->findOrCreateLabeledValue(m_p->m_selnLabel);
    m_p->m_hoverValue = m_p->m_seln->findOrCreateLabeledValue(m_p->m_hoverLabel);
    QPointer<qtResourceBrowser> self(this);
    m_p->m_seln->registerSelectionSource(m_p->m_selnSource);
    m_p->m_selnHandle = m_p->m_seln->observers().insert(
      [self](const std::string& source, smtk::view::Selection::Ptr seln) {
        if (self)
        {
          self->sendSMTKSelectionToPanel(source, seln);
        }
      },
      "qtResourceBrowser: Update from SMTK selection.");
  }
  m_p->m_phraseModel->addSource(managers);
}

void qtResourceBrowser::removeSource(smtk::common::TypeContainer& managers)
{
  if (m_p->m_seln == managers.get<smtk::view::SelectionPtr>())
  {
    m_p->m_seln->observers().erase(m_p->m_selnHandle);
  }
  m_p->m_seln = nullptr;

  m_p->m_phraseModel->removeSource(managers);
}

void qtResourceBrowser::hoverRow(const QModelIndex& idx)
{
  if (!m_p->m_seln)
  {
    return;
  }
  // Erase the current hover state.
  smtk::resource::ComponentSet csetAdd;
  smtk::resource::ComponentSet csetDel;
  this->resetHover(csetAdd, csetDel);

  // Discover what is currently hovered
  auto phr =
    idx.data(qtDescriptivePhraseModel::PhrasePtrRole).value<smtk::view::DescriptivePhrasePtr>();
  if (!phr)
  {
    return;
  }

  auto comp = phr->relatedComponent();
  if (!comp)
  {
    return;
  }

  // Add new hover state
  const auto& selnMap = m_p->m_seln->currentSelection();
  auto cvit = selnMap.find(comp);
  int sv = (cvit == selnMap.end() ? 0 : cvit->second) | m_p->m_hoverValue;
  csetAdd.clear();
  csetAdd.insert(comp);
  m_p->m_seln->modifySelection(
    csetAdd, m_p->m_selnSource, sv, smtk::view::SelectionAction::UNFILTERED_ADD);
}

void qtResourceBrowser::resetHover()
{
  smtk::resource::ComponentSet csetAdd;
  smtk::resource::ComponentSet csetDel;
  this->resetHover(csetAdd, csetDel);
}

void qtResourceBrowser::resetHover(
  smtk::resource::ComponentSet& csetAdd,
  smtk::resource::ComponentSet& csetDel)
{
  // Erase the current hover state.
  if (!m_p->m_seln)
  {
    return;
  }
  m_p->m_seln->visitSelection(
    [this, &csetAdd, &csetDel](smtk::resource::PersistentObject::Ptr obj, int sv) {
      auto cp = std::dynamic_pointer_cast<smtk::resource::Component>(obj);
      if (!cp)
      {
        return;
      }
      sv = sv & (~m_p->m_hoverValue);
      if (sv)
      {
        csetAdd.insert(cp);
      }
      else
      {
        csetDel.insert(cp);
      }
    });
  if (!csetAdd.empty())
  {
    m_p->m_seln->modifySelection(
      csetAdd, m_p->m_selnSource, m_p->m_selnValue, smtk::view::SelectionAction::UNFILTERED_ADD);
  }
  if (!csetDel.empty())
  {
    m_p->m_seln->modifySelection(
      csetDel, m_p->m_selnSource, 0, smtk::view::SelectionAction::UNFILTERED_SUBTRACT);
  }
}

bool qtResourceBrowser::eventFilter(QObject* obj, QEvent* evnt)
{
  QKeyEvent* evt;
  if (
    obj == m_p->m_view && evnt->type() == QEvent::KeyPress &&
    (evt = dynamic_cast<QKeyEvent*>(evnt)))
  {
    if (evt->key() == Qt::Key_Space)
    {
      // Iterate over the selected indices and toggle the visibility of
      // every item to either on or off (determined by examining the first
      // index's current state).
      auto selected = m_p->m_view->selectionModel()->selection();
      smtk::view::DescriptivePhrase::Ptr phrase;
      qt::VisibilityBadge* badge = nullptr;
      for (auto idx : selected.indexes())
      {
        phrase = idx.data(qtDescriptivePhraseModel::PhrasePtrRole)
                   .value<smtk::view::DescriptivePhrase::Ptr>();
        if (phrase)
        {
          badge = phrase->phraseModel()->badges().findBadgeOfType<qt::VisibilityBadge>();
          if (badge)
          {
            break;
          }
        }
      }
      if (badge)
      {
        badge->action(phrase.get(), qtBadgeActionToggle(selected));
        return true;
      }
    }
    else if (evt->key() == Qt::Key_Backspace || evt->key() == Qt::Key_Delete)
    {
      auto operationManager = m_p->m_phraseModel->operationManager();
      if (operationManager)
      {
        auto selected = m_p->m_view->selectionModel()->selection();
        std::set<smtk::resource::PersistentObjectPtr> objects;
        for (const auto& idx : selected.indexes())
        {
          auto phrase = idx.data(qtDescriptivePhraseModel::PhrasePtrRole)
                          .value<smtk::view::DescriptivePhrase::Ptr>();
          if (phrase && phrase->relatedObject())
          {
            objects.insert(phrase->relatedObject());
          }
        }
        smtk::operation::DeleterGroup deleters(operationManager);
        while (!objects.empty())
        {
          std::set<smtk::resource::PersistentObjectPtr> candidates;
          smtk::operation::OperationPtr op;
          for (const auto& object : objects)
          {
            if (!op)
            {
              auto index = deleters.matchingOperation(*object);
              if (index)
              {
                op = operationManager->create(index);
              }
            }
            if (op && op->parameters()->associate(object))
            {
              candidates.insert(object);
            }
          }
          for (const auto& object : candidates)
          {
            objects.erase(object);
          }
          if (!op)
          {
            // No operation was found for anything selected and unprocessed.
            break;
          }
          if (!op->ableToOperate() && op->parameters()->findVoid("delete dependents"))
          {
            QMessageBox msgBox;
            msgBox.setText(
              QString("Unable to delete %1 selected object(s).").arg(candidates.size()));
            msgBox.setInformativeText("Delete all dependent entities as well?");
            auto buttons = QMessageBox::Yes | QMessageBox::No;
            msgBox.setStandardButtons(buttons);
            msgBox.setDefaultButton(QMessageBox::Yes);
            // QCheckBox* cb = new QCheckBox("Set default and don't show again.");
            // msgBox.setCheckBox(cb);

            int ret = msgBox.exec();
            // auto cbChecked = cb->isChecked();
            if (ret == QMessageBox::No)
            {
              return true;
            }
            op->parameters()->findVoid("delete dependents")->setIsEnabled(true);
          }
          operationManager->launchers()(op);
        }
        return true;
      }
    }
  }
  else if (
    evnt->type() == QEvent::MouseButtonPress && m_p->m_view->isVisible() &&
    obj == m_p->m_view->viewport())
  {
    if (qtDescriptivePhraseDelegate::processBadgeClick(
          static_cast<QMouseEvent*>(evnt), m_p->m_view))
    {
      return true;
    }
  }
  if (obj == m_p->m_view && evnt->type() == QEvent::Leave)
  {
    this->resetHover();
  }
  return this->Superclass::eventFilter(obj, evnt);
}
