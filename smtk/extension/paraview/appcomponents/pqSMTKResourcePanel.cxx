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
#include "smtk/extension/qt/qtResourceBrowser.h"

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

#include "pqActiveObjects.h"
#include "pqCoreUtilities.h"
#include "vtkCommand.h"
#include "vtkCompositeRepresentation.h"

#include <QColorDialog>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QPointer>

using qtDescriptivePhraseModel = smtk::extension::qtDescriptivePhraseModel;

class pqSMTKResourcePanel::Internal
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

  void setup(::pqSMTKResourcePanel* parent)
  {
    // QWidget* ww = new QWidget(parent);
    m_browser = new smtk::extension::qtResourceBrowser("", parent);
    parent->setWindowTitle("Resources");
    // this->setupUi(ww);
    parent->setWidget(m_browser);
    m_phraseModel = smtk::view::ResourcePhraseModel::create();
    m_phraseModel->setDecorator([this](smtk::view::DescriptivePhrasePtr phr) {
      smtk::view::VisibilityContent::decoratePhrase(
        phr, [this](smtk::view::VisibilityContent::Query qq, int val,
               smtk::view::ConstPhraseContentPtr data) {
          return this->panelPhraseDecorator(qq, val, data);
        });
    });
    m_browser->setPhraseModel(m_phraseModel);

    m_model = new smtk::extension::qtDescriptivePhraseModel;
    m_model->setPhraseModel(m_phraseModel);
    m_delegate = new smtk::extension::qtDescriptivePhraseDelegate;

    m_delegate->setTextVerticalPad(6);
    m_delegate->setTitleFontWeight(1);
    m_delegate->setDrawSubtitle(false);

    QObject::connect(m_delegate, SIGNAL(requestVisibilityChange(const QModelIndex&)), m_model,
      SLOT(toggleVisibility(const QModelIndex&)));
    /*
    QObject::connect(m_delegate, SIGNAL(requestColorChange(const QModelIndex&)), parent,
      SLOT(editObjectColor(const QModelIndex&)));

    QObject::connect(m_searchText, SIGNAL(textChanged(const QString&)), parent,
      SLOT(searchTextChanged(const QString&)));
      */

    auto smtkSettings = vtkSMTKSettings::GetInstance();
    pqCoreUtilities::connect(
      smtkSettings, vtkCommand::ModifiedEvent, parent, SLOT(updateSettings()));
    // Now initialize the highlight state and signal-connections:
    parent->updateSettings();
  }

  int panelPhraseDecorator(
    smtk::view::VisibilityContent::Query qq, int val, smtk::view::ConstPhraseContentPtr data)
  {
    auto comp = data->relatedComponent();
    auto rsrc = data->relatedResource();

    smtk::model::EntityPtr ent =
      data ? std::dynamic_pointer_cast<smtk::model::Entity>(comp) : nullptr;
    smtk::model::ResourcePtr modelRsrc = ent
      ? ent->modelResource()
      : (data ? std::dynamic_pointer_cast<smtk::model::Resource>(rsrc) : nullptr);

    smtk::mesh::ComponentPtr msh =
      data ? std::dynamic_pointer_cast<smtk::mesh::Component>(comp) : nullptr;
    smtk::mesh::ResourcePtr meshRsrc = msh
      ? std::dynamic_pointer_cast<smtk::mesh::Resource>(msh->resource())
      : (data ? std::dynamic_pointer_cast<smtk::mesh::Resource>(rsrc) : nullptr);

    auto smtkBehavior = pqSMTKBehavior::instance();

    // TODO: We could check more than just that the view is non-null.
    //       For instance, does the resource have a representation in the active view?
    //       However, that gets expensive.
    bool validView = pqActiveObjects::instance().activeView() ? true : false;

    switch (qq)
    {
      case smtk::view::VisibilityContent::DISPLAYABLE:
        return validView && (ent || (!ent && modelRsrc) || (msh || (!ent && meshRsrc))) ? 1 : 0;
      case smtk::view::VisibilityContent::EDITABLE:
        return validView && (ent || (!ent && modelRsrc) || (msh || (!ent && meshRsrc))) ? 1 : 0;
      case smtk::view::VisibilityContent::GET_VALUE:
        if (ent || msh)
        {
          auto valIt = m_visibleThings.find(comp->id());
          if (valIt != m_visibleThings.end())
          {
            return valIt->second;
          }
          return 1; // visibility is assumed if there is no entry.
        }
        else if (modelRsrc || meshRsrc)
        {
          auto view = pqActiveObjects::instance().activeView();
          auto pvrc = smtkBehavior->getPVResource(rsrc);
          // If we are trying to get the value of a resource that has no
          // pipeline source, we create one.
          if (pvrc == nullptr)
          {
            pvrc = pqSMTKRenderResourceBehavior::instance()->createPipelineSource(rsrc);
          }
          auto mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
          return mapr ? mapr->isVisible() : 0;
        }
        return 0; // visibility is false if the component is not a model entity or NULL.
      case smtk::view::VisibilityContent::SET_VALUE:
        if (ent || msh)
        { // Find the mapper in the active view for the related resource, then set the visibility.
          auto view = pqActiveObjects::instance().activeView();
          auto pvrc = smtkBehavior->getPVResource(data->relatedResource());
          auto mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
          auto smap = dynamic_cast<pqSMTKModelRepresentation*>(mapr);
          if (smap)
          {
            int rval = smap->setVisibility(comp, val ? true : false) ? 1 : 0;
            smap->renderViewEventually();
            return rval;
          }
        }
        else if (modelRsrc || meshRsrc)
        { // A resource, not a component, is being modified. Change the pipeline object's visibility.
          auto view = pqActiveObjects::instance().activeView();
          auto pvrc = smtkBehavior->getPVResource(rsrc);
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
  }

  QPointer<smtk::extension::qtResourceBrowser> m_browser;
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
  int m_resourceTreeStyle; // Which subphrase generator should be used?

  // Set to true when inside sendSMTKSelectionToPanel.
  // Used to avoid updating the SMTK selection from the panel while
  // the panel is being updated from SMTK:
  bool m_updatingPanelSelectionFromSMTK;
};

pqSMTKResourcePanel::pqSMTKResourcePanel(QWidget* parent)
  : Superclass(parent)
{
  m_p = new Internal;
  m_p->setup(this);

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
  m_p->m_browser->addSource(
    wrapper->smtkResourceManager(), wrapper->smtkOperationManager(), wrapper->smtkSelection());
}

void pqSMTKResourcePanel::resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }

  m_p->m_browser->removeSource(
    mgr->smtkResourceManager(), mgr->smtkOperationManager(), mgr->smtkSelection());
}

void pqSMTKResourcePanel::activeViewChanged(pqView* view)
{
  // Disconnect old representations, clear local visibility map.
  QObject::disconnect(this, SLOT(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)));
  m_p->m_visibleThings.clear();
  // Connect new representations, initialize visibility map..
  if (view)
  {
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
        // TODO: This assumes we are running in built-in mode. Remove the need for me.
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
  // Indicate to the Qt model that it needs to refresh every row,
  // since visibility may be altered on each one:
  // m_p->m_phraseModel->triggerDataChanged();
  m_p->m_browser->phraseModel()->triggerDataChanged();
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

void pqSMTKResourcePanel::updateSettings()
{
  auto smtkSettings = vtkSMTKSettings::GetInstance();
  m_p->m_browser->setHighlightOnHover(smtkSettings->GetHighlightOnHover());

  int resourceTreeStyle = smtkSettings->GetResourceTreeStyle();
  if (resourceTreeStyle != m_p->m_resourceTreeStyle)
  {
    smtk::view::SubphraseGenerator::Ptr spg = nullptr;
    switch (resourceTreeStyle)
    {
      case vtkSMTKSettings::HierarchicalStyle:
        spg = smtk::view::SubphraseGenerator::create();
        break;
      case vtkSMTKSettings::TwoLevelStyle:
        spg = smtk::view::TwoLevelSubphraseGenerator::create();
        break;
      default:
        smtkWarningMacro(
          smtk::io::Logger::instance(), "Unsupported resource tree style. Resetting to default.");
        spg = smtk::view::SubphraseGenerator::create();
        break;
    }
    if (spg)
    {
      m_p->m_resourceTreeStyle = resourceTreeStyle;
      m_p->m_browser->setPhraseGenerator(spg);
    }
  }
}
