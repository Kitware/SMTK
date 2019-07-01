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

template <typename T, typename U>
int UpdateVisibilityForFootprint(pqSMTKResourceRepresentation* smap, const T& comp, int visible,
  U& visibleThings, const smtk::view::DescriptivePhrasePtr&)
{
  bool didUpdate = false;
  int rval(0);

  if (auto ment = std::dynamic_pointer_cast<smtk::model::Entity>(comp))
  {
    if (ment->isModel() || ment->isGroup())
    {
      int any = 0;
      smtk::model::EntityIterator childIt;
      smtk::model::EntityRef entRef = ment->template referenceAs<smtk::model::EntityRef>();
      childIt.traverse(entRef, smtk::model::IteratorStyle::ITERATE_CHILDREN);
      for (childIt.begin(); !childIt.isAtEnd(); ++childIt)
      {
        auto child = childIt.current().entityRecord();
        int ok = smap->setVisibility(child, visible);
        any |= ok;
        visibleThings[child->id()] = visible;
      }
      rval = any;
      if (any)
      {
        didUpdate = true;
      }
    }
    else
    {
      // Composite auxliliary geometry condition
      int any = 0;
      smtk::model::AuxiliaryGeometry auxgeom =
        ment->template referenceAs<smtk::model::AuxiliaryGeometry>();
      auto auxgeomChildren = auxgeom.auxiliaryGeometries();
      if (auxgeom && !auxgeomChildren.empty())
      {
        for (const auto& child : auxgeomChildren)
        {
          int ok = smap->setVisibility(child.component(), visible ? true : false);
          any |= ok;
          visibleThings[child.entity()] = visible;
        }
      }
      rval |= any;

      rval |= smap->setVisibility(comp, visible ? true : false) ? 1 : 0;
      if (rval)
      {
        visibleThings[comp->id()] =
          visible; // Should we set here or wait until we hear back from smap?
        didUpdate = true;
      }
    }
  }
  else if (auto meshComponent = std::dynamic_pointer_cast<smtk::mesh::Component>(comp))
  {
    rval |= smap->setVisibility(comp, visible ? true : false) ? 1 : 0;
    if (rval)
    {
      visibleThings[comp->id()] = visible;
      didUpdate = true;
    }
  }

  if (didUpdate)
  {
    smap->renderViewEventually();
  }
  return rval;
}

pqSMTKResourceBrowser::pqSMTKResourceBrowser(const smtk::view::PhraseModelPtr& phraseModel,
  const std::string& modelViewName, QAbstractItemModel* model, QWidget* parent)
  : Superclass(phraseModel, modelViewName, model, parent)
{
  // The superclass has initialized m_p;
  // now we must add ParaView-specific configuration:

  // I. Decorate phrases with visibility related to representations in the active view.
  m_p->m_phraseModel->setDecorator([this](smtk::view::DescriptivePhrasePtr phr) {
    smtk::view::VisibilityContent::decoratePhrase(
      phr, [this](smtk::view::VisibilityContent::Query qq, int val,
             smtk::view::ConstPhraseContentPtr data) {
        return pqSMTKResourceBrowser::panelPhraseDecorator(qq, val, data, m_p->m_visibleThings);
      });
  });

  // II. Prepare the subphrase generator.
  //     This is important since otherwise m_p->m_phraseModel will
  //     not be set properly and the decorator will not be invoked.
  this->updateSettings();

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
}

pqSMTKResourceBrowser::~pqSMTKResourceBrowser()
{
}

int pqSMTKResourceBrowser::panelPhraseDecorator(smtk::view::VisibilityContent::Query qq, int val,
  smtk::view::ConstPhraseContentPtr data, std::map<smtk::common::UUID, int>& visibleThings)
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

  // If we are trying to get the value of a resource that has no pipeline
  // source, we create one.
  auto pvrc = smtkBehavior->getPVResource(rsrc);
  if (pvrc == nullptr && rsrc)
  {
    pvrc = pqSMTKRenderResourceBehavior::instance()->createPipelineSource(rsrc);
  }

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
        auto valIt = visibleThings.find(comp->id());
        if (valIt != visibleThings.end())
        {
          return valIt->second;
        }
        return 1; // visibility is assumed if there is no entry.
      }
      else if (modelRsrc || meshRsrc)
      {
        auto view = pqActiveObjects::instance().activeView();
        auto mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
        return mapr ? mapr->isVisible() : 0;
      }
      return 0; // visibility is false if the component is not a model entity or NULL.
    case smtk::view::VisibilityContent::SET_VALUE:
      if (ent || msh)
      { // Find the mapper in the active view for the related resource, then set the visibility.
        auto view = pqActiveObjects::instance().activeView();
        auto mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
        auto smap = dynamic_cast<pqSMTKResourceRepresentation*>(mapr);
        if (smap)
        {
          int rval = UpdateVisibilityForFootprint(smap, comp, val, visibleThings, data->location());
          return rval;
        }
      }
      else if (modelRsrc || meshRsrc)
      { // A resource, not a component, is being modified. Change the pipeline object's visibility.
        auto view = pqActiveObjects::instance().activeView();
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
  this->addSource(
    wrapper->smtkResourceManager(), wrapper->smtkOperationManager(), wrapper->smtkSelection());
}

void pqSMTKResourceBrowser::resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }

  this->removeSource(mgr->smtkResourceManager(), mgr->smtkOperationManager(), mgr->smtkSelection());
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

void pqSMTKResourceBrowser::updateSettings()
{
  auto smtkSettings = vtkSMTKSettings::GetInstance();
  this->setHighlightOnHover(smtkSettings->GetHighlightOnHover());

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
      this->setPhraseGenerator(spg);
    }
  }
}
