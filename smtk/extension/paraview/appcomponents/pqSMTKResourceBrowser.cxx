//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceBrowser.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceRepresentation.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h" // FIXME: Remove the need for me
#include "smtk/extension/paraview/server/vtkSMTKSettings.h"

#include "smtk/extension/qt/qtDescriptivePhraseModel.h"
#include "smtk/extension/qt/qtResourceBrowser.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/TwoLevelSubphraseGenerator.h"
#include "smtk/view/VisibilityContent.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "pqActiveObjects.h"
#include "vtkCompositeRepresentation.h"

#include "smtk/extension/qt/qtResourceBrowserP.h"

#include <regex>

const std::string pqSMTKResourceBrowser::getJSONConfiguration()
{
  // we want to use the base config, but use our type instead of qtResourceBrowser.
  std::string baseConfig = smtk::extension::qtResourceBrowser::getJSONConfiguration();
  return std::regex_replace(baseConfig, std::regex("qtResourceBrowser"), "pqSMTKResourceBrowser");
}

smtk::extension::qtBaseView* pqSMTKResourceBrowser::createViewWidget(
  const smtk::view::Information& info)
{
  pqSMTKResourceBrowser* view = new pqSMTKResourceBrowser(info);
  view->buildUI();
  return view;
}

pqSMTKResourceBrowser::pqSMTKResourceBrowser(const smtk::view::Information& info)
  : Superclass(info)
{
  // The superclass has initialized m_p;
  // now we must add ParaView-specific configuration:

  // I. Decorate phrases with visibility related to representations in the active view.
  // m_p->m_phraseModel->setDecorator([this](smtk::view::DescriptivePhrasePtr phr) {
  //   smtk::view::VisibilityContent::decoratePhrase(
  //     phr, [this](smtk::view::VisibilityContent::Query query, int val,
  //            smtk::view::ConstPhraseContentPtr data) {
  //       return pqSMTKResourceBrowser::panelPhraseDecorator(query, val, data, m_p->m_visibleThings);
  //     });
  // });

  // II. Prepare the subphrase generator.
  //     This is important since otherwise m_p->m_phraseModel will
  //     not be set properly and the decorator will not be invoked.
  this->initSubphraseGenerator();

  // III. Ensure the phrase model is configured to listen to the proper managers.
  // Listen for resources on current connections:
  auto smtkBehavior = pqSMTKBehavior::instance();
  smtkBehavior->visitResourceManagersOnServers([this](pqSMTKWrapper* r, pqServer* s) {
    this->resourceManagerAdded(r, s);
    return false;
  });
  // Now listen for future connections.
  QObject::connect(smtkBehavior, SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)), this,
    SLOT(resourceManagerAdded(pqSMTKWrapper*, pqServer*)));
  QObject::connect(smtkBehavior, SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)), this,
    SLOT(resourceManagerRemoved(pqSMTKWrapper*, pqServer*)));

  // IV. Reset eyeball icons when the active view changes:
  pqActiveObjects& act(pqActiveObjects::instance());
  QObject::connect(&act, SIGNAL(viewChanged(pqView*)), this, SLOT(activeViewChanged(pqView*)));
  // Now call immediately, since in at least some circumstances, a view may already be active.
  this->activeViewChanged(act.activeView());

  this->updateSettings();
}

pqSMTKResourceBrowser::~pqSMTKResourceBrowser()
{
  QObject::disconnect(this);
}

void pqSMTKResourceBrowser::searchTextChanged(const QString& searchText)
{ // For now, just rebuild.
  (void)searchText;
  // m_p->m_model->rebuildSubphrases(QModelIndex());
}

void pqSMTKResourceBrowser::resourceManagerAdded(pqSMTKWrapper* wrapper, pqServer* server)
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
  this->addSource(wrapper->smtkResourceManager(), wrapper->smtkOperationManager(),
    wrapper->smtkViewManager(), wrapper->smtkSelection());
}

void pqSMTKResourceBrowser::resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }
  this->removeSource(mgr->smtkResourceManager(), mgr->smtkOperationManager(),
    mgr->smtkViewManager(), mgr->smtkSelection());
}

void pqSMTKResourceBrowser::activeViewChanged(pqView* view)
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
  for (const auto& rsrcPhrase : rsrcPhrases)
  {
    auto rsrc = rsrcPhrase->relatedResource();
    if (!rsrc)
    {
      continue;
    }
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
      auto srvrep = vtkSMTKResourceRepresentation::SafeDownCast(
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
  this->phraseModel()->triggerDataChanged();
}

void pqSMTKResourceBrowser::representationAddedToActiveView(pqRepresentation* rep)
{
  auto modelRep = dynamic_cast<pqSMTKResourceRepresentation*>(rep);
  if (modelRep)
  {
    QObject::connect(modelRep,
      SIGNAL(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)), this,
      SLOT(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)));
  }
}

void pqSMTKResourceBrowser::representationRemovedFromActiveView(pqRepresentation* rep)
{
  auto modelRep = dynamic_cast<pqSMTKResourceRepresentation*>(rep);
  if (modelRep)
  {
    QObject::disconnect(modelRep,
      SIGNAL(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)), this,
      SLOT(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)));
  }
}

void pqSMTKResourceBrowser::componentVisibilityChanged(
  smtk::resource::ComponentPtr comp, bool visible)
{
  // The visibilty should change for every row displaying the same \a ent:
  m_p->m_visibleThings[comp->id()] = visible;
  m_p->m_phraseModel->triggerDataChangedFor(comp);
}

void pqSMTKResourceBrowser::initSubphraseGenerator()
{
  std::string subphraseViewType = smtk::view::SubphraseGenerator::getType(m_viewInfo.m_view);
  auto smtkSettings = vtkSMTKSettings::GetInstance();

  int resourceTreeStyle = smtkSettings->GetResourceTreeStyle();
  std::string defaultSubphraseType;
  if (resourceTreeStyle != m_p->m_resourceTreeStyle)
  {
    switch (resourceTreeStyle)
    {
      case vtkSMTKSettings::HierarchicalStyle:
        defaultSubphraseType = "smtk::view::SubphraseGenerator";
        break;
      case vtkSMTKSettings::TwoLevelStyle:
        defaultSubphraseType = "smtk::view::TwoLevelSubphraseGenerator";
        break;
      default:
        smtkWarningMacro(
          smtk::io::Logger::instance(), "Unsupported resource tree style. Resetting to default.");
        defaultSubphraseType = "smtk::view::SubphraseGenerator";
        break;
    }
  }
  if (m_p->m_resourceTreeType.empty() || m_p->m_resourceTreeType != subphraseViewType ||
    (subphraseViewType == "default" && resourceTreeStyle != m_p->m_resourceTreeStyle))
  {
    smtk::view::ManagerPtr manager = m_viewInfo.m_UIManager->viewManager();
    smtk::view::SubphraseGenerator::Ptr spg = smtk::view::SubphraseGenerator::create(
      subphraseViewType == "default" ? defaultSubphraseType : subphraseViewType, manager);
    if (spg)
    {
      m_p->m_resourceTreeType = subphraseViewType;
      if (subphraseViewType == "default")
      {
        m_p->m_resourceTreeStyle = resourceTreeStyle;
      }
      this->setPhraseGenerator(spg);
    }
  }
}

void pqSMTKResourceBrowser::updateSettings()
{
  auto smtkSettings = vtkSMTKSettings::GetInstance();
  this->setHighlightOnHover(smtkSettings->GetHighlightOnHover());

  int resourceTreeStyle = smtkSettings->GetResourceTreeStyle();
  if (resourceTreeStyle != m_p->m_resourceTreeStyle && m_p->m_resourceTreeType == "default")
  {
    this->initSubphraseGenerator();
  }
}
