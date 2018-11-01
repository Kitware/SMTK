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
#include "smtk/extension/paraview/appcomponents/pqSMTKModelRepresentation.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"
#include "smtk/extension/paraview/server/vtkSMTKSettings.h"

#include "smtk/extension/paraview/server/vtkSMTKModelRepresentation.h" // FIXME: Remove the need for me

#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseEditor.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/VisibilityContent.h"

#include "smtk/model/Entity.h"
#include "smtk/model/Resource.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "ui_pqSMTKResourcePanel.h"

#include "pqActiveObjects.h"
#include "pqCoreUtilities.h"
#include "vtkCommand.h"
#include "vtkCompositeRepresentation.h"

#include <QColorDialog>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QPointer>

using qtDescriptivePhraseModel = smtk::extension::qtDescriptivePhraseModel;

class pqSMTKResourcePanel::Internal : public Ui::pqSMTKResourcePanel
{
public:
  Internal()
    : m_selnSource("resource panel")
    , m_selnLabel("selected")
    , m_hoverLabel("hovered")
  {
  }

  ~Internal()
  {
    // Unregister our decorator before we become invalid.
    m_phraseModel->setDecorator([](smtk::view::DescriptivePhrasePtr) {});
  }

  void setup(::pqSMTKResourcePanel* parent)
  {
    QWidget* ww = new QWidget(parent);
    parent->setWindowTitle("Resources");
    this->setupUi(ww);
    parent->setWidget(ww);
    m_phraseModel = smtk::view::ResourcePhraseModel::create();
    m_phraseModel->setDecorator([this](smtk::view::DescriptivePhrasePtr phr) {
      smtk::view::VisibilityContent::decoratePhrase(phr, [this](
                                                           smtk::view::VisibilityContent::Query qq,
                                                           int val,
                                                           smtk::view::ConstPhraseContentPtr data) {
        smtk::model::EntityPtr ent =
          data ? std::dynamic_pointer_cast<smtk::model::Entity>(data->relatedComponent()) : nullptr;
        smtk::model::ResourcePtr mResource = ent
          ? ent->modelResource()
          : (data ? std::dynamic_pointer_cast<smtk::model::Resource>(data->relatedResource())
                  : nullptr);
        auto smtkBehavior = pqSMTKBehavior::instance();

        // TODO: We could check more than just that the view is non-null.
        //       For instance, does the resource have a representation in the active view?
        //       However, that gets expensive.
        bool validView = pqActiveObjects::instance().activeView() ? true : false;

        switch (qq)
        {
          case smtk::view::VisibilityContent::DISPLAYABLE:
            return validView && (ent || (!ent && mResource)) ? 1 : 0;
          case smtk::view::VisibilityContent::EDITABLE:
            return validView && (ent || (!ent && mResource)) ? 1 : 0;
          case smtk::view::VisibilityContent::GET_VALUE:
            if (ent)
            {
              auto valIt = m_visibleThings.find(ent->id());
              if (valIt != m_visibleThings.end())
              {
                return valIt->second;
              }
              return 1; // visibility is assumed if there is no entry.
            }
            else if (mResource)
            {
              auto view = pqActiveObjects::instance().activeView();
              auto pvrc = smtkBehavior->getPVResource(mResource);
              // If we are trying to get the value of a resource that has no
              // pipeline source, we create one.
              if (pvrc == nullptr)
              {
                pvrc = pqSMTKRenderResourceBehavior::instance()->createPipelineSource(mResource);
              }
              auto mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
              return mapr ? mapr->isVisible() : 0;
            }
            return 0; // visibility is false if the component is not a model entity or NULL.
          case smtk::view::VisibilityContent::SET_VALUE:
            if (ent)
            { // Find the mapper in the active view for the related resource, then set the visibility.
              auto view = pqActiveObjects::instance().activeView();
              auto pvrc = smtkBehavior->getPVResource(data->relatedResource());
              auto mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
              auto smap = dynamic_cast<pqSMTKModelRepresentation*>(mapr);
              if (smap)
              {
                int rval = smap->setVisibility(ent, val ? true : false) ? 1 : 0;
                smap->renderViewEventually();
                return rval;
              }
            }
            else if (mResource)
            { // A resource, not a component, is being modified. Change the pipeline object's visibility.
              auto view = pqActiveObjects::instance().activeView();
              auto pvrc = smtkBehavior->getPVResource(mResource);
              auto mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
              if (mapr)
              {
                mapr->setVisible(!mapr->isVisible());
                pqActiveObjects::instance().setActiveSource(pvrc);
                mapr->renderViewEventually();
                return 1;
              }
            }
            return 0;
        }
        return 0;
      });
    });
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

    QObject::connect(m_searchText, SIGNAL(textChanged(const QString&)), parent,
      SLOT(searchTextChanged(const QString&)));
    QObject::connect(m_view->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), parent,
      SLOT(sendPanelSelectionToSMTK(const QItemSelection&, const QItemSelection&)));

    auto smtkSettings = vtkSMTKSettings::GetInstance();
    pqCoreUtilities::connect(
      smtkSettings, vtkCommand::ModifiedEvent, parent, SLOT(updateSettings()));
    // Now initialize the highlight state and signal-connections:
    parent->updateSettings();
  }

  QPointer<smtk::extension::qtDescriptivePhraseModel> m_model;
  QPointer<smtk::extension::qtDescriptivePhraseDelegate> m_delegate;
  std::map<smtk::resource::ManagerPtr, int> m_observers;
  smtk::view::ResourcePhraseModel::Ptr m_phraseModel;
  smtk::view::Selection::Ptr m_seln; // TODO: This assumes there is only 1 server connection
  int m_selnHandle;                  // TODO: Same assumption as m_seln
  int m_selnValue;
  int m_hoverValue;
  std::string m_selnSource; // TODO: This assumes there is only 1 panel (or that all should share)
  std::string m_selnLabel;
  std::string m_hoverLabel;
  std::map<smtk::common::UUID, int> m_visibleThings;
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
  smtkBehavior->visitResourceManagersOnServers([this](pqSMTKWrapper* r, pqServer* s) {
    this->resourceManagerAdded(r, s);
    return false;
  });
  // Now listen for future connections.
  QObject::connect(smtkBehavior, SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)), this,
    SLOT(resourceManagerAdded(pqSMTKWrapper*, pqServer*)));
  QObject::connect(smtkBehavior, SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)), this,
    SLOT(resourceManagerRemoved(pqSMTKWrapper*, pqServer*)));

  pqActiveObjects& act(pqActiveObjects::instance());
  QObject::connect(&act, SIGNAL(viewChanged(pqView*)), this, SLOT(activeViewChanged(pqView*)));
  // Now call immediately, since in at least some circumstances, a view may already be active.
  this->activeViewChanged(act.activeView());
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
  if (spg)
  {
    spg->setModel(m_p->m_phraseModel);
  }
  root->setDelegate(spg);
  m_p->m_model->rebuildSubphrases(QModelIndex());
}

void pqSMTKResourcePanel::leaveEvent(QEvent* evt)
{
  this->resetHover();
  // Now let the superclass do what it wants:
  Superclass::leaveEvent(evt);
}

void pqSMTKResourcePanel::sendPanelSelectionToSMTK(const QItemSelection&, const QItemSelection&)
{
  if (!m_p->m_seln)
  {
    return;
  } // No SMTK selection exists.

  //smtk::view::Selection::SelectionMap selnMap;
  std::set<smtk::resource::Component::Ptr> selnSet;
  auto selected = m_p->m_view->selectionModel()->selection();
  smtk::resource::Resource::Ptr selectedResource;
  for (auto qslist : selected.indexes())
  {
    auto phrase = m_p->m_model->getItem(qslist);
    smtk::resource::Component::Ptr comp;
    smtk::resource::Resource::Ptr rsrc;
    if (phrase && (comp = phrase->relatedComponent()))
    {
      selnSet.insert(comp);
    }
    else if (phrase && (rsrc = phrase->relatedResource()) && !selectedResource)
    { // Pick only the first resource selected
      selectedResource = rsrc;
    }
  }
  m_p->m_seln->modifySelection(
    selnSet, m_p->m_selnSource, m_p->m_selnValue, smtk::view::SelectionAction::UNFILTERED_REPLACE);
  if (selectedResource)
  {
    // Make the reader owning the first selected resource the active PV pipeline source:
    auto behavior = pqSMTKBehavior::instance();
    auto rsrcSrc = behavior->getPVResource(selectedResource);
    if (rsrcSrc)
    {
      pqActiveObjects::instance().setActiveSource(rsrcSrc);
    }
  }
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

void pqSMTKResourcePanel::resourceManagerAdded(pqSMTKWrapper* wrapper, pqServer* server)
{
  if (!wrapper || !server)
  {
    return;
  }

  // wrapper->smtkProxy()->UpdateVTKObjects();
  smtk::resource::ManagerPtr rsrcMgr = wrapper->smtkResourceManager();
  if (!rsrcMgr)
  {
    return;
  }
  m_p->m_seln = wrapper->smtkSelection();
  if (m_p->m_seln)
  {
    m_p->m_selnValue = m_p->m_seln->findOrCreateLabeledValue(m_p->m_selnLabel);
    m_p->m_hoverValue = m_p->m_seln->findOrCreateLabeledValue(m_p->m_hoverLabel);
    QPointer<pqSMTKResourcePanel> self(this);
    m_p->m_seln->registerSelectionSource(m_p->m_selnSource);
    m_p->m_selnHandle =
      m_p->m_seln->observe([self](const std::string& source, smtk::view::Selection::Ptr seln) {
        if (self)
        {
          self->sendSMTKSelectionToPanel(source, seln);
        }
      });
  }
  m_p->m_phraseModel->addSource(wrapper->smtkResourceManager(), wrapper->smtkOperationManager());
}

void pqSMTKResourcePanel::resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server)
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

  if (m_p->m_seln)
  {
    m_p->m_seln->unobserve(m_p->m_selnHandle);
  }

  m_p->m_seln = nullptr;
  m_p->m_phraseModel->removeSource(mgr->smtkResourceManager(), mgr->smtkOperationManager());
}

void pqSMTKResourcePanel::activeViewChanged(pqView* view)
{
  m_p->m_visibleThings.clear();
  if (view)
  {
    // Disconnect old representations, connect new ones.
    QObject::disconnect(this, SLOT(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)));
    foreach (pqRepresentation* rep, view->getRepresentations())
    {
      this->representationAddedToActiveView(rep);
    }
    QObject::connect(view, SIGNAL(representationAdded(pqRepresentation*)), this,
      SLOT(representationAddedToActiveView(pqRepresentation*)));
    QObject::connect(view, SIGNAL(representationRemoved(pqRepresentation*)), this,
      SLOT(representationRemovedFromActiveView(pqRepresentation*)));
  }
  auto rsrcPhrases = m_p->m_phraseModel->root()->subphrases();
  auto behavior = pqSMTKBehavior::instance();
  for (auto rsrcPhrase : rsrcPhrases)
  {
    auto rsrc = rsrcPhrase->relatedResource();
    auto pvr = behavior->getPVResource(rsrc);
    auto rep = pvr ? pvr->getRepresentation(view) : nullptr;
    // TODO: At a minimum, we can update the representation's visibility now
    //       since if rep is null it is invisible and if not null, we can ask
    //       for its visibility.
    if (rep)
    {
      m_p->m_visibleThings[rsrc->id()] = rep->isVisible() ? 1 : 0;
      auto thingy = rep->getProxy()->GetClientSideObject();
      auto thingy2 = vtkCompositeRepresentation::SafeDownCast(thingy);
      auto srvrep = vtkSMTKModelRepresentation::SafeDownCast(
        thingy2 ? thingy2->GetActiveRepresentation() : nullptr);
      if (srvrep)
      {
        // TODO: This assumes we are running in built-in mode.
        srvrep->GetEntityVisibilities(m_p->m_visibleThings);
      }
    }
    else
    {
      // This is a sign that things are going poorly.
      // The representation should already have been created either when
      // the view was created or the resource loaded.
      m_p->m_visibleThings[rsrc->id()] = behavior->createRepresentation(pvr, view) ? 1 : 0;
    }
  }
  m_p->m_phraseModel->triggerDataChanged();
}

void pqSMTKResourcePanel::representationAddedToActiveView(pqRepresentation* rep)
{
  auto modelRep = dynamic_cast<pqSMTKModelRepresentation*>(rep);
  if (modelRep)
  {
    QObject::connect(modelRep,
      SIGNAL(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)), this,
      SLOT(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)));
  }
}

void pqSMTKResourcePanel::representationRemovedFromActiveView(pqRepresentation* rep)
{
  auto modelRep = dynamic_cast<pqSMTKModelRepresentation*>(rep);
  if (modelRep)
  {
    QObject::disconnect(modelRep,
      SIGNAL(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)), this,
      SLOT(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)));
  }
}

void pqSMTKResourcePanel::componentVisibilityChanged(
  smtk::resource::ComponentPtr comp, bool visible)
{
  // The visibilty should change for every row displaying the same \a ent:
  m_p->m_visibleThings[comp->id()] = visible;
  m_p->m_phraseModel->triggerDataChangedFor(comp);
}

void pqSMTKResourcePanel::hoverRow(const QModelIndex& idx)
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

void pqSMTKResourcePanel::resetHover()
{
  smtk::resource::ComponentSet csetAdd;
  smtk::resource::ComponentSet csetDel;
  this->resetHover(csetAdd, csetDel);
}

void pqSMTKResourcePanel::resetHover(
  smtk::resource::ComponentSet& csetAdd, smtk::resource::ComponentSet& csetDel)
{
  // Erase the current hover state.
  if (!m_p->m_seln)
  {
    return;
  }
  m_p->m_seln->visitSelection(
    [this, &csetAdd, &csetDel](smtk::resource::Component::Ptr cp, int sv) {
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

void pqSMTKResourcePanel::updateSettings()
{
  auto smtkSettings = vtkSMTKSettings::GetInstance();
  if (smtkSettings->GetHighlightOnHover())
  {
    m_p->m_delegate->setHighlightOnHover(true);
    QObject::connect(
      m_p->m_view, SIGNAL(entered(const QModelIndex&)), this, SLOT(hoverRow(const QModelIndex&)));
  }
  else
  {
    m_p->m_delegate->setHighlightOnHover(false);
    QObject::disconnect(
      m_p->m_view, SIGNAL(entered(const QModelIndex&)), this, SLOT(hoverRow(const QModelIndex&)));
    this->resetHover();
  }
}

void pqSMTKResourcePanel::editObjectColor(const QModelIndex& idx)
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
