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

#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseEditor.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"
#include "smtk/extension/qt/qtSMTKUtilities.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/TwoLevelSubphraseGenerator.h"
#include "smtk/view/VisibilityContent.h"

#include "smtk/model/Entity.h"
#include "smtk/model/Resource.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include <QAbstractProxyModel>
#include <QColorDialog>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QPointer>
#include <QTreeView>

#include "smtk/extension/qt/qtResourceBrowserP.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"

using namespace smtk::extension;

qtResourceBrowser::qtResourceBrowser(const smtk::view::PhraseModelPtr& phraseModel,
  const std::string& modelViewName, QAbstractItemModel* qmodel, QWidget* parent)
  : Superclass(parent)
{
  m_p = new Internal;
  m_p->setup(this, phraseModel, modelViewName, qmodel, parent);
}

qtResourceBrowser::~qtResourceBrowser()
{
  delete m_p;
}

QTreeView* qtResourceBrowser::createDefaultView(QWidget* parent)
{
  auto view = new QTreeView(parent);
  view->setObjectName(QStringLiteral("m_view"));
  view->setAcceptDrops(true);
  view->setDragEnabled(true);
  view->setDragDropMode(QAbstractItemView::DragDrop);
  view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  view->setSortingEnabled(true);

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
  auto dpmodel = m_p->descriptivePhraseModel();
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
  root->setDelegate(spg);
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

void qtResourceBrowser::leaveEvent(QEvent* evt)
{
  this->resetHover();
  // Now let the superclass do what it wants:
  Superclass::leaveEvent(evt);
}

void qtResourceBrowser::sendPanelSelectionToSMTK(const QItemSelection&, const QItemSelection&)
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

  //smtk::view::Selection::SelectionMap selnMap;
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
  const std::string& src, smtk::view::SelectionPtr seln)
{
  if (src == m_p->m_selnSource)
  {
    // Ignore selections generated from this panel.
    return;
  }
  auto qview = m_p->m_view;
  auto qmodel = m_p->descriptivePhraseModel();
  auto root = m_p->m_phraseModel->root();
  QItemSelection qseln;
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

  auto smodel = dynamic_cast<QAbstractProxyModel*>(qview->selectionModel()->model());
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

void qtResourceBrowser::addSource(smtk::resource::ManagerPtr rsrcMgr,
  smtk::operation::ManagerPtr operMgr, smtk::view::SelectionPtr seln)
{
  m_p->m_seln = seln;
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
      });
  }
  m_p->m_phraseModel->addSource(rsrcMgr, operMgr, seln);
}

void qtResourceBrowser::removeSource(smtk::resource::ManagerPtr rsrcMgr,
  smtk::operation::ManagerPtr operMgr, smtk::view::SelectionPtr seln)
{
  if (m_p->m_seln == seln)
  {
    m_p->m_seln->observers().erase(m_p->m_selnHandle);
  }
  m_p->m_seln = nullptr;

  m_p->m_phraseModel->removeSource(rsrcMgr, operMgr, seln);
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
  smtk::resource::ComponentSet& csetAdd, smtk::resource::ComponentSet& csetDel)
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

void qtResourceBrowser::editObjectColor(const QModelIndex& idx)
{
  auto phrase =
    idx.data(qtDescriptivePhraseModel::PhrasePtrRole).value<smtk::view::DescriptivePhrasePtr>();
  if (phrase)
  {
    std::string dialogInstructions = "Choose Color for " +
      idx.data(qtDescriptivePhraseModel::TitleTextRole).value<QString>().toStdString() +
      " (click Cancel to remove color)";
    QColor currentColor = idx.data(qtDescriptivePhraseModel::PhraseColorRole).value<QColor>();
    QColor nextColor = QColorDialog::getColor(currentColor, this, dialogInstructions.c_str(),
      QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);
    bool removeColor = !nextColor.isValid();
    if (removeColor)
    {
      smtk::model::FloatList rgba{ 0., 0., 0., -1. };
      phrase->setRelatedColor(rgba);
    }
    else
    {
      smtk::model::FloatList rgba{ nextColor.red() / 255.0, nextColor.green() / 255.0,
        nextColor.blue() / 255.0, nextColor.alpha() / 255.0 };
      phrase->setRelatedColor(rgba);
    }
  }
}

bool qtResourceBrowser::eventFilter(QObject* obj, QEvent* evnt)
{
  QKeyEvent* evt;
  if (obj == m_p->m_view && evnt->type() == QEvent::KeyPress &&
    (evt = dynamic_cast<QKeyEvent*>(evnt)))
  {
    if (evt->key() == Qt::Key_Space)
    {
      // Iterate over the selected indices and toggle the visibility of
      // every item to either on or off (determined by examining the first
      // index's current state).
      auto selected = m_p->m_view->selectionModel()->selection();
      smtk::view::DescriptivePhrase::Ptr phrase;
      bool toggleTo = false;
      bool found = false;
      for (auto idx : selected.indexes())
      {
        phrase = idx.data(qtDescriptivePhraseModel::PhrasePtrRole)
                   .value<smtk::view::DescriptivePhrase::Ptr>();
        if (!phrase)
        {
          continue;
        }
        if (phrase->displayVisibility())
        {
          toggleTo = !phrase->relatedVisibility();
          found = true;
          break;
        }
      }
      if (found)
      {
        for (auto idx : selected.indexes())
        {
          phrase = idx.data(qtDescriptivePhraseModel::PhrasePtrRole)
                     .value<smtk::view::DescriptivePhrase::Ptr>();
          if (!phrase)
          {
            continue;
          }
          phrase->setRelatedVisibility(toggleTo);
        }
        return true;
      }
    }
  }
  return this->Superclass::eventFilter(obj, evnt);
}
