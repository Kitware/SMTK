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

// #include "ui_qtResourceBrowser.h"

#include <QColorDialog>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QPointer>
#include <QTreeView>

using namespace smtk::extension;

using qtDescriptivePhraseModel = smtk::extension::qtDescriptivePhraseModel;

class qtResourceBrowser::Internal // : public Ui::qtResourceBrowser
{
public:
  Internal()
    : m_selnSource("resource panel")
    , m_selnLabel("selected")
    , m_hoverLabel("hovered")
    , m_resourceTreeStyle(-1)
    , m_updatingPanelSelectionFromSMTK(false)
  {
  }

  ~Internal()
  {
    // Unregister our decorator before we become invalid.
    m_phraseModel->setDecorator([](smtk::view::DescriptivePhrasePtr) {});
  }

  void setup(::qtResourceBrowser* parent, const std::string& viewName)
  {
    parent->setWindowTitle("Resources");
    auto viewMap = qtSMTKUtilities::modelViewConstructors();
    auto vcit = viewMap.find(viewName);
    if (vcit == viewMap.end())
    {
      vcit = viewMap.find(""); // The default constructor.
    }
    qtModelViewConstructor ctor = vcit->second;
    if (!ctor)
    {
      ctor = qtResourceBrowser::createDefaultView;
    }
    m_layout = new QVBoxLayout(parent);
    m_layout->setObjectName("m_layout");
    m_view = ctor(parent);
    m_layout->addWidget(m_view);
    m_phraseModel = smtk::view::ResourcePhraseModel::create();
    m_model = new smtk::extension::qtDescriptivePhraseModel;
    m_model->setPhraseModel(m_phraseModel);
    m_delegate = new smtk::extension::qtDescriptivePhraseDelegate;

    m_delegate->setTextVerticalPad(6);
    m_delegate->setTitleFontWeight(1);
    m_delegate->setDrawSubtitle(false);
    m_view->setModel(m_model);
    m_view->setItemDelegate(m_delegate);
    m_view->setMouseTracking(true); // Needed to receive hover events.

    QObject::connect(m_delegate, SIGNAL(requestVisibilityChange(const QModelIndex&)), m_model,
      SLOT(toggleVisibility(const QModelIndex&)));
    QObject::connect(m_delegate, SIGNAL(requestColorChange(const QModelIndex&)), parent,
      SLOT(editObjectColor(const QModelIndex&)));

    QObject::connect(m_view->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), parent,
      SLOT(sendPanelSelectionToSMTK(const QItemSelection&, const QItemSelection&)));
  }

  QVBoxLayout* m_layout;
  QTreeView* m_view;
  QPointer<smtk::extension::qtDescriptivePhraseModel> m_model;
  QPointer<smtk::extension::qtDescriptivePhraseDelegate> m_delegate;
  std::map<smtk::resource::ManagerPtr, int> m_observers;
  smtk::view::PhraseModel::Ptr m_phraseModel;
  smtk::view::Selection::Ptr m_seln; // TODO: This assumes there is only 1 server connection
  int m_selnHandle;                  // TODO: Same assumption as m_seln
  int m_selnValue;
  int m_hoverValue;
  std::string m_selnSource; // TODO: This assumes there is only 1 panel (or that all should share)
  std::string m_selnLabel;
  std::string m_hoverLabel;
  std::map<smtk::common::UUID, int> m_visibleThings;
  int m_resourceTreeStyle; // Which subphrase generator should be used?

  // Set to true when inside sendSMTKSelectionToPanel.
  // Used to avoid updating the SMTK selection from the panel while
  // the panel is being updated from SMTK:
  bool m_updatingPanelSelectionFromSMTK;
};

qtResourceBrowser::qtResourceBrowser(const std::string& viewName, QWidget* parent)
  : Superclass(parent)
{
  m_p = new Internal;
  m_p->setup(this, viewName);
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
  if (m_p->m_phraseModel && m_p->m_model)
  {
    m_p->m_model->setPhraseModel(m_p->m_phraseModel);
    m_p->m_model->rebuildSubphrases(QModelIndex());
  }
}

smtk::view::SubphraseGeneratorPtr qtResourceBrowser::phraseGenerator() const
{
  auto root = m_p->m_model->getItem(QModelIndex());
  return root ? root->findDelegate() : nullptr;
}

void qtResourceBrowser::setPhraseGenerator(smtk::view::SubphraseGeneratorPtr spg)
{
  auto root = m_p->m_model->getItem(QModelIndex());
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
    auto phrase = m_p->m_model->getItem(qslist);
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
  auto qmodel = m_p->m_model;
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
    m_p->m_selnHandle =
      m_p->m_seln->observe([self](const std::string& source, smtk::view::Selection::Ptr seln) {
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
    m_p->m_seln->unobserve(m_p->m_selnHandle);
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
  auto phr = m_p->m_model->getItem(idx);
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
  auto phrase = m_p->m_model->getItem(idx);
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
