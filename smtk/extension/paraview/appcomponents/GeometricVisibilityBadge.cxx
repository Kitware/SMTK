//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/appcomponents/GeometricVisibilityBadge.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceBrowser.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceRepresentation.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h" // FIXME: Remove the need for me
#include "smtk/extension/vtk/geometry/Backend.h"
#include "smtk/geometry/Geometry.h"
#include "smtk/geometry/Resource.h"
#include "smtk/geometry/queries/SelectionFootprint.h"
#include "smtk/resource/query/BadTypeError.h"

#include "smtk/view/icons/checkbox_blanked_cpp.h"
#include "smtk/view/icons/checkbox_partial_cpp.h"
#include "smtk/view/icons/checkbox_visible_cpp.h"

#include "smtk/common/Color.h"
#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/Manager.h"

#include "pqActiveObjects.h"
#include "vtkCompositeRepresentation.h"
#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMSourceProxy.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace appcomponents
{

GeometricVisibilityBadge::VisibilityState
GeometricVisibilityBadge::PhraseInfo::calculateHierarchicalVisibility(
  const smtk::view::DescriptivePhrase* phrase,
  bool decrementNumChildren,
  std::size_t offset) const
{
  std::size_t numChildren = phrase->subphrases().size();

  // Do we need to adjust the number of children count - this would be in the case of removing phrases
  if (decrementNumChildren)
  {
    if (offset <= numChildren)
    {
      numChildren -= offset;
    }
    else
    {
      numChildren = 0;
    }
  }

  // If there are no children then there are no visible children
  if (numChildren == 0)
  {
    return VisibilityState::Invisible;
  }

  if (m_numberOfVisibleChildren == numChildren)
  {
    return VisibilityState::Visible;
  }

  if (m_numberOfInvisibleChildren == numChildren)
  {
    return VisibilityState::Invisible;
  }
  return VisibilityState::Neither;
}

GeometricVisibilityBadge::VisibilityState
GeometricVisibilityBadge::PhraseInfo::calculateCombinedVisibility(
  const smtk::view::DescriptivePhrase* phrase,
  bool decrementNumChildren,
  std::size_t offset) const
{

  std::size_t numChildren = phrase->subphrases().size();
  // Do we need to adjust the number of children count - this would be in the case of removing phrases
  if (decrementNumChildren)
  {
    if (offset <= numChildren)
    {
      numChildren -= offset;
    }
    else
    {
      numChildren = 0;
    }
  }

  if (numChildren == 0)
  {
    return m_geometricVisibility;
  }

  VisibilityState hVis =
    this->calculateHierarchicalVisibility(phrase, decrementNumChildren, offset);

  if (
    (hVis == VisibilityState::Neither) || (m_geometricVisibility == VisibilityState::Neither) ||
    (hVis == m_geometricVisibility))
  {
    return hVis;
  }

  return VisibilityState::Neither;
}

void GeometricVisibilityBadge::PhraseCache::updateForRemoval(
  smtk::view::DescriptivePhrase* phrase,
  std::size_t numVisibleChildrenToBeRemoved,
  std::size_t numInvisibleChildrenToBeRemoved,
  std::size_t numNeitherChildrenToBeRemoved)
{
  auto it = m_phraseInfos.find(phrase);
  if (it == m_phraseInfos.end())
  {
    return; // the phrase doesn't exist in the cache
  }

  PhraseInfo& pinfo = it->second;

  // Get phrase's current visibility
  VisibilityState currentState = pinfo.calculateCombinedVisibility(phrase);

  // Update the counts
  pinfo.m_numberOfVisibleChildren -= numVisibleChildrenToBeRemoved;
  pinfo.m_numberOfInvisibleChildren -= numInvisibleChildrenToBeRemoved;

  VisibilityState newState = pinfo.calculateCombinedVisibility(
    phrase,
    true,
    numVisibleChildrenToBeRemoved + numInvisibleChildrenToBeRemoved +
      numNeitherChildrenToBeRemoved);

  // Do we need to update the phrase's parent?
  if (currentState != newState)
  {
    smtk::view::DescriptivePhrase* parent = phrase->parent().get();
    if (parent)
    {
      this->update(parent, currentState, newState);
    }
  }
}

void GeometricVisibilityBadge::PhraseCache::update(
  smtk::view::DescriptivePhrase* phrase,
  const VisibilityState& childPreviousVisibility,
  const VisibilityState& childNewVisibility,
  bool okToCreate)
{
  auto it = m_phraseInfos.find(phrase);
  if ((!okToCreate) && (it == m_phraseInfos.end()))
  {
    return; // the phrase doesn't exist in the cache and we were told not to create an entry
  }
  PhraseInfo& pinfo = m_phraseInfos[phrase];

  VisibilityState currentState = VisibilityState::Neither;
  VisibilityState newState;

  // If the phrase was already in the cache then get its current visibility and change it base on the previous visibility
  if (it != m_phraseInfos.end())
  {
    currentState = pinfo.calculateCombinedVisibility(phrase);
    // Decrement the previousVisibility
    if (childPreviousVisibility == VisibilityState::Visible)
    {
      pinfo.m_numberOfVisibleChildren--;
    }
    else if (childPreviousVisibility == VisibilityState::Invisible)
    {
      pinfo.m_numberOfInvisibleChildren--;
    }
  }

  // Update the cache based on the new visibility state
  if (childNewVisibility == VisibilityState::Visible)
  {
    pinfo.m_numberOfVisibleChildren++;
  }
  else if (childNewVisibility == VisibilityState::Invisible)
  {
    pinfo.m_numberOfInvisibleChildren++;
  }

  newState = pinfo.calculateCombinedVisibility(phrase);

  // Do we need to update the phrase's parent?
  if (currentState != newState)
  {
    smtk::view::DescriptivePhrase* parent = phrase->parent().get();
    if (parent)
    {
      this->update(parent, currentState, newState);
    }
  }
}

void GeometricVisibilityBadge::PhraseCache::update(
  smtk::view::DescriptivePhrase* phrase,
  const VisibilityState& geometricVisibility,
  bool isResource)
{
  auto it = m_phraseInfos.find(phrase);
  PhraseInfo& pinfo = m_phraseInfos[phrase];

  VisibilityState currentState = VisibilityState::Neither;
  VisibilityState newState;

  // If the phrase was already in the cache then get its current visibility
  if (it != m_phraseInfos.end())
  {
    currentState = pinfo.calculateCombinedVisibility(phrase);
  }

  // Update the cache based on the new visibility state
  pinfo.m_geometricVisibility = geometricVisibility;
  pinfo.m_isResource = isResource;

  newState = pinfo.calculateCombinedVisibility(phrase);

  // Do we need to update the phrase's parent?
  if (currentState != newState)
  {
    smtk::view::DescriptivePhrase* parent = phrase->parent().get();
    if (parent)
    {
      this->update(parent, currentState, newState);
    }
  }
}

void GeometricVisibilityBadge::PhraseCache::removeSubTreeVisibilities(
  view::DescriptivePhrase* phrase,
  std::map<smtk::common::UUID, int>& uuidToRepVisibility,
  bool isTopOfSubTree)
{

  // Get its persistent object and remove it from the representation visibility map
  auto obj = phrase->relatedObject();
  if (obj)
  {
    uuidToRepVisibility.erase(obj->id());
  }
  // find this phrase in the cache it it exists
  auto pit = m_phraseInfos.find(phrase);
  if (pit != m_phraseInfos.end())
  {
    // Is this the top of the original sub tree?  If so lets calculate its current visibility and
    // tell its parent it is changing to Neither since its going away
    if (isTopOfSubTree)
    {
      VisibilityState current = pit->second.calculateCombinedVisibility(phrase);
      if (current != VisibilityState::Neither)
      {
        // see if we can find its parent
        auto parentDp = phrase->parent();
        if (parentDp)
        {
          this->update(parentDp.get(), current, VisibilityState::Neither, false);
        }
      }
    }

    // Remove the phrase from the cache
    m_phraseInfos.erase(pit);
  }
  // Now remove all of the phrase's children from the cache
  if (phrase->areSubphrasesBuilt())
  {
    for (const auto& child : phrase->subphrases())
    {
      this->removeSubTreeVisibilities(child.get(), uuidToRepVisibility, false);
    }
  }
}

// This method removes the phrase and all of its descendants from the cache.
// NOTE - this does not update the phase's parent's visibility!  If you need to do that you
// should first call updateForRemoval on the parent phrase.
void GeometricVisibilityBadge::PhraseCache::removePhrase(smtk::view::DescriptivePhrase* phrase)
{
  auto it = m_phraseInfos.find(phrase);
  // If the phrase exists, remove it
  if (it != m_phraseInfos.end())
  {
    m_phraseInfos.erase(it);
  }

  if (!phrase->areSubphrasesBuilt())
  {
    // There is nothing else to be removed
    return;
  }

  for (const auto& childPhrase : phrase->subphrases())
  {
    this->removePhrase(childPhrase.get());
  }
  return;
}

// This method inserts a newly inserted Descriptive Phrase
void GeometricVisibilityBadge::PhraseCache::insertNewPhrase(smtk::view::DescriptivePhrase* phrase)
{
  auto rsrc = phrase->relatedResource();
  smtk::geometry::ResourcePtr geomRsrc = std::dynamic_pointer_cast<smtk::geometry::Resource>(rsrc);

  // we only need to update this phrase if it has geometry
  if (geomRsrc)
  {
    smtk::extension::vtk::geometry::Backend vtk;
    const auto& geom = geomRsrc->geometry(vtk);
    if (
      geom &&
      ((geom->generationNumber(phrase->relatedObject()) != smtk::geometry::Geometry::Invalid) ||
       (phrase->relatedObject() == geomRsrc)))
    {
      // ASSUME: The inserted geometry is visible by default - THIS MAY CHANGE IN THE FUTURE
      this->update(phrase, VisibilityState::Visible, (phrase->relatedObject() == geomRsrc));
    }
  }
  // Now process the phrase's children since they too had been newly inserted
  for (const auto& childPhrase : phrase->subphrases())
  {
    this->insertNewPhrase(childPhrase.get());
  }
}

GeometricVisibilityBadge::GeometricVisibilityBadge()
  : m_icon(checkbox_visible_svg())
  , m_iconClosed(checkbox_blanked_svg())
{
}

GeometricVisibilityBadge::GeometricVisibilityBadge(
  smtk::view::BadgeSet& parent,
  const smtk::view::Configuration::Component&)
  : m_icon(checkbox_visible_svg())
  , m_iconClosed(checkbox_blanked_svg())
  , m_parent(&parent)
{
  // Reset eyeball icons when the active view changes:
  pqActiveObjects& act(pqActiveObjects::instance());
  QObject::connect(&act, SIGNAL(viewChanged(pqView*)), this, SLOT(activeViewChanged(pqView*)));
  // Now call immediately, since in at least some circumstances, a view may already be active.
  if (this->phraseModel())
  {
    this->activeViewChanged(act.activeView());
    // Subscribe to updates to the phrase model so we can track number of children accurately.
    m_key = this->phraseModel()->observers().insert(
      [this](
        smtk::view::DescriptivePhrasePtr phrase,
        smtk::view::PhraseModelEvent event,
        const std::vector<int>& src,
        const std::vector<int>& dst,
        const std::vector<int>& range) { this->updateObserver(phrase, event, src, dst, range); },
      "Geometric visibility badge phrase tracker.");
  }
}

GeometricVisibilityBadge::~GeometricVisibilityBadge()
{
  m_parent = nullptr;
}

bool GeometricVisibilityBadge::appliesToPhrase(const DescriptivePhrase* phrase) const
{
  auto rsrc = phrase->relatedResource();
  smtk::geometry::ResourcePtr geomRsrc = std::dynamic_pointer_cast<smtk::geometry::Resource>(rsrc);

  bool validView = pqActiveObjects::instance().activeView() != nullptr;
  if (validView && geomRsrc)
  {
    // Only show the visibility badge on attributes that explicitly have renderable geometry.
    smtk::extension::vtk::geometry::Backend vtk;
    const auto& geom = geomRsrc->geometry(vtk);
    if (
      geom &&
      ((geom->generationNumber(phrase->relatedObject()) != smtk::geometry::Geometry::Invalid) ||
       (phrase->relatedObject() == geomRsrc)))
    {
      return true;
    }
  }
  return false;
}

GeometricVisibilityBadge::PhraseCache& GeometricVisibilityBadge::phraseCache()
{
  return m_parent->badgeData().get<PhraseCache>();
}

const GeometricVisibilityBadge::PhraseCache& GeometricVisibilityBadge::phraseCache() const
{
  return m_parent->badgeData().get<PhraseCache>();
}

bool GeometricVisibilityBadge::phraseVisibility(const DescriptivePhrase* phrase) const
{
  auto it = this->phraseCache().phraseInfos().find(const_cast<DescriptivePhrase*>(phrase));
  if (it == this->phraseCache().phraseInfos().end())
  {
    return !(phrase->relatedObject() == phrase->relatedResource() && !!phrase->relatedResource());
  }
  return (it->second.m_geometricVisibility == VisibilityState::Visible);
}

void GeometricVisibilityBadge::setPhraseVisibility(const DescriptivePhrase* phrase, int val)
{
  auto comp = phrase->relatedComponent();
  auto rsrc = phrase->relatedResource();

  smtk::geometry::ResourcePtr geomRsrc = std::dynamic_pointer_cast<smtk::geometry::Resource>(rsrc);

  auto* smtkBehavior = pqSMTKBehavior::instance();

  auto pvrc = smtkBehavior->getPVResource(rsrc);

  if (comp && geomRsrc)
  { // Find the mapper in the active view for the related resource, then set the visibility.
    auto* view = pqActiveObjects::instance().activeView();
    auto* mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
    auto* smap = dynamic_cast<pqSMTKResourceRepresentation*>(mapr);
    smtk::extension::vtk::geometry::Backend vtk;
    bool didUpdate = smap ? smap->setVisibility(comp, val != 0) : true;

    if (didUpdate && smap)
    {
      smap->renderViewEventually();
    }
  }
  else if (geomRsrc)
  { // A resource, not a component, is being modified. Change the pipeline object's visibility.
    auto* view = pqActiveObjects::instance().activeView();
    auto* mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
    if (mapr)
    {
      mapr->setVisible(!mapr->isVisible());
      pqActiveObjects::instance().setActiveSource(pvrc);
      mapr->renderViewEventually();
    }
    else if (pvrc)
    {
      auto* view = pqActiveObjects::instance().activeView();
      if (view)
      {
        // Create a representation as needed for the active view.
        vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;
        controller->Show(vtkSMSourceProxy::SafeDownCast(pvrc->getProxy()), 0, view->getViewProxy());
        mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
      }
    }

    // Resources don't get a signal from ParaView indicating the representation
    // proxy visibility changed; update the panel immediately. Note that even if
    // we monitor each representation for its visibilityChanged() signal, that
    // signal is fired before the server-side property is updated and a render occurs,
    // so there is no point in listening to it.
    auto& pcache = this->phraseCache();
    const auto& uuid2PhrasesMap = this->phraseModel()->uuidPhraseMap();
    // Find all of the Phrases that use this resource
    auto rsrcId = rsrc->id();
    auto rit = uuid2PhrasesMap.find(rsrcId);
    if (rit == uuid2PhrasesMap.end())
    {
      return; // There are no phrases for this ID
    }

    // Now that we know we have phrases to process lets set their visibility
    VisibilityState geomVis =
      (mapr && mapr->isVisible()) ? VisibilityState::Visible : VisibilityState::Invisible;

    for (const auto& wp : rit->second)
    {
      auto phrase = wp.lock();
      pcache.update(phrase.get(), geomVis);
    }

    this->phraseModel()->triggerDataChanged(); // FIXME: implement triggerDataChangedFor(rsrc);
  }
}

std::string GeometricVisibilityBadge::icon(
  const DescriptivePhrase* phrase,
  const std::array<float, 4>& /*background*/) const
{
  if (this->phraseVisibility(phrase))
  {
    return m_icon;
  }
  return m_iconClosed;
}

bool GeometricVisibilityBadge::action(const DescriptivePhrase* phrase, const BadgeAction& act)
{
  if (!dynamic_cast<const smtk::view::BadgeActionToggle*>(&act))
  {
    return false; // we only support toggling.
  }

  int newVal = !this->phraseVisibility(phrase) ? 1 : 0;
  bool didVisit = false;

  act.visitRelatedPhrases([this, newVal, &didVisit](const DescriptivePhrase* related) -> bool {
    didVisit = true;
    this->setPhraseVisibility(related, newVal);
    return false;
  });

  // If the UI component did not provide a set of related phrases, at least
  // toggle visibility of our primary phrase:
  if (!didVisit)
  {
    this->setPhraseVisibility(phrase, newVal);
  }

  return true;
}

void GeometricVisibilityBadge::activeViewChanged(pqView* view)
{
  // Disconnect old representations, clear visibility maps.
  QObject::disconnect(
    this, SLOT(componentVisibilityChanged(const smtk::resource::ComponentPtr&, bool)));
  this->phraseCache().clear();
  m_uuidToRepVisibility.clear();

  // Connect new view's representations, initialize visibility map..
  if (view)
  {
    Q_FOREACH (pqRepresentation* rep, view->getRepresentations())
    {
      this->representationAddedToActiveView(rep);
    }
    QObject::connect(
      view,
      SIGNAL(representationAdded(pqRepresentation*)),
      this,
      SLOT(representationAddedToActiveView(pqRepresentation*)));
    QObject::connect(
      view,
      SIGNAL(representationRemoved(pqRepresentation*)),
      this,
      SLOT(representationRemovedFromActiveView(pqRepresentation*)));
  }
  this->phraseModel()->triggerDataChanged();
}

void GeometricVisibilityBadge::representationAddedToActiveView(pqRepresentation* rep)
{
  auto* smtkRep = dynamic_cast<pqSMTKResourceRepresentation*>(rep);
  if (!smtkRep)
  {
    return;
  }

  QObject::connect(
    smtkRep,
    SIGNAL(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)),
    this,
    SLOT(componentVisibilityChanged(const smtk::resource::ComponentPtr&, bool)));

  // TODO: The following  assumes we are running in built-in mode. Remove the need for this!
  auto* thingy = rep->getProxy()->GetClientSideObject();
  auto* thingy2 = vtkCompositeRepresentation::SafeDownCast(thingy);
  auto* srvrep = vtkSMTKResourceRepresentation::SafeDownCast(
    thingy2 ? thingy2->GetActiveRepresentation() : nullptr);
  if (!srvrep)
  {
    return;
  }
  // Get the visibility information as well as the geometry resource from the representation
  srvrep->GetEntityVisibilities(m_uuidToRepVisibility);
  auto repResource = std::dynamic_pointer_cast<smtk::geometry::Resource>(srvrep->GetResource());

  if (!repResource)
  {
    return;
  }

  //Lets also get the Phrase Model uuidMap and the badge's phrase cache
  const auto& uuid2PhrasesMap = this->phraseModel()->uuidPhraseMap();
  auto& pcache = this->phraseCache();

  // Lets set the Resource's geometric visibility to that of the representation

  auto resId = repResource->id();
  auto resIt = uuid2PhrasesMap.find(resId);
  if (resIt != uuid2PhrasesMap.end())
  {
    // Get the representation's visibility

    VisibilityState resVis =
      rep->isVisible() ? VisibilityState::Visible : VisibilityState::Invisible;
    // Now set all of the corresponding DPs correctly
    for (const auto& resDp : resIt->second)
    {
      auto phrase = resDp.lock();
      pcache.update(phrase.get(), resVis, true);
    }
  }

  // Now visit the resource's geometry if it has any
  smtk::extension::vtk::geometry::Backend vtk;
  const auto& geom = repResource->geometry(vtk);
  if (!geom)
  {
    return;
  }

  geom->visit([this, &uuid2PhrasesMap, &pcache](
                const smtk::resource::PersistentObject::Ptr& obj,
                smtk::geometry::Geometry::GenerationNumber genNum) {
    if (!obj || (genNum == smtk::geometry::Geometry::Invalid))
    {
      return false; // Either there is no Object or it doesn't have geometry
    }
    // Find all of the Phrases that use this object
    auto objId = obj->id();
    auto pit = uuid2PhrasesMap.find(objId);
    if (pit == uuid2PhrasesMap.end())
    {
      return false; // There are no phrases for this ID
    }

    // Now that we know we have phrases to process lets get their visibility
    VisibilityState geomVis = VisibilityState::Visible;
    auto vit = m_uuidToRepVisibility.find(objId);
    if (vit != m_uuidToRepVisibility.end())
    {
      geomVis = (vit->second == 0) ? VisibilityState::Invisible : VisibilityState::Visible;
    }
    for (const auto& wp : pit->second)
    {
      auto phrase = wp.lock();
      pcache.update(phrase.get(), geomVis);
    }
    return false;
  });

  // Get Geometry Cache
  // Visit the cache entries
  // For cache entry
  //  1. Skip if no geometry
  //  2. Get visibility from visibleInfo (else its visible)
  //  3. Find all Phrases and call update(geomVisible)
}

void GeometricVisibilityBadge::representationRemovedFromActiveView(pqRepresentation* rep)
{
  // Get the Representation Resource (if it has one)
  auto* smtkRep = dynamic_cast<pqSMTKResourceRepresentation*>(rep);
  if (!smtkRep)
  {
    return;
  }
  auto* pipeline = dynamic_cast<pqSMTKResource*>(smtkRep->getInput());
  if (!pipeline)
  {
    return;
  }
  auto resource = pipeline->getResource();
  // Ensure that when a representation is removed due to
  // the resource being closed that we "forget" the visibility
  // state of its components — otherwise, reloading the resource
  // will result in inconsistent state.
  if (!(resource || this->phraseModel()->root()))
  {
    return;
  }

  auto& pcache = this->phraseCache();
  auto rsrcPhrases = this->phraseModel()->root()->subphrases();
  std::function<void(const smtk::view::DescriptivePhrase::Ptr&)> updater =
    [this, &resource, &pcache, &updater](const smtk::view::DescriptivePhrase::Ptr& phrase) {
      if (phrase && phrase->relatedResource() == resource)
      {
        pcache.removeSubTreeVisibilities(phrase.get(), m_uuidToRepVisibility, true);
        return;
      }
      if (phrase->areSubphrasesBuilt())
      {
        for (const auto& child : phrase->subphrases())
        {
          updater(child);
        }
      }
    };
  for (const auto& rsrcPhrase : rsrcPhrases)
  {
    updater(rsrcPhrase);
  }
  // Indicate to the Qt model that it needs to refresh every row,
  // since in theory visibility may be altered on each one:
  this->phraseModel()->triggerDataChanged();

  QObject::disconnect(
    smtkRep,
    SIGNAL(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)),
    this,
    SLOT(componentVisibilityChanged(const smtk::resource::ComponentPtr&, bool)));
}

void GeometricVisibilityBadge::componentVisibilityChanged(
  const smtk::resource::ComponentPtr& comp,
  bool visible)
{
  // The visibility should change for every row displaying the same \a comp:
  auto* pmodel = this->phraseModel();
  if (!pmodel)
  {
    return;
  }

  // Lets  get the Phrase Model uuidMap and the badge's phrase cache
  const auto& uuid2PhrasesMap = pmodel->uuidPhraseMap();
  auto& pcache = this->phraseCache();

  // Find all of the Phrases that use this component
  auto compId = comp->id();
  auto cit = uuid2PhrasesMap.find(compId);
  if (cit == uuid2PhrasesMap.end())
  {
    return; // There are no phrases for this ID
  }

  // Now that we know we have phrases to process lets set their visibility
  VisibilityState geomVis = visible ? VisibilityState::Visible : VisibilityState::Invisible;

  for (const auto& wp : cit->second)
  {
    auto phrase = wp.lock();
    pcache.update(phrase.get(), geomVis);
  }

  pmodel->triggerDataChangedFor(comp);
}

void GeometricVisibilityBadge::updateObserver(
  smtk::view::DescriptivePhrasePtr phrase,
  smtk::view::PhraseModelEvent event,
  const std::vector<int>&,
  const std::vector<int>&,
  const std::vector<int>& range)
{

  auto& pcache = this->phraseCache();
  auto& phraseInfos = pcache.phraseInfos();

  // Does this phrase exist?
  auto it = phraseInfos.find(phrase.get());
  if (it == phraseInfos.end())
  {
    return; // the phrase doesn't exist in the cache
  }

  if (event == smtk::view::PhraseModelEvent::ABOUT_TO_REMOVE)
  {
    std::size_t deltaVis = 0, deltaInvis = 0, deltaNeither = 0;
    for (int i = range[0]; i <= range[1]; i++)
    {
      auto* childPhrase = phrase->subphrases()[i].get();
      auto childIt = phraseInfos.find(childPhrase);
      if (childIt == phraseInfos.end())
      {
        continue; // The child isn't in the cache
      }
      auto childVis = childIt->second.calculateCombinedVisibility(childPhrase);
      if (childVis == VisibilityState::Visible)
      {
        ++deltaVis;
      }
      else if (childVis == VisibilityState::Invisible)
      {
        ++deltaInvis;
      }
      else
      {
        ++deltaNeither;
      }
      // Remove the child phrase and its children from the cache
      pcache.removePhrase(childPhrase);
    }
    pcache.updateForRemoval(phrase.get(), deltaVis, deltaInvis, deltaNeither);
    return;
  }
  if (event == smtk::view::PhraseModelEvent::INSERT_FINISHED)
  {
    for (int i = range[0]; i <= range[1]; i++)
    {
      auto* childPhrase = phrase->subphrases()[i].get();
      pcache.insertNewPhrase(childPhrase);
    }
    return;
  }
}

std::string GeometricVisibilityBadge::convertVisibilityToString(
  const smtk::extension::paraview::appcomponents::GeometricVisibilityBadge::VisibilityState& state)
{
  if (
    state ==
    smtk::extension::paraview::appcomponents::GeometricVisibilityBadge::VisibilityState::Visible)
  {
    return "Visible";
  }
  if (
    state ==
    smtk::extension::paraview::appcomponents::GeometricVisibilityBadge::VisibilityState::Invisible)
  {
    return "Invisible";
  }

  return "Neither";
}

} // namespace appcomponents
} // namespace paraview
} // namespace extension
} // namespace smtk
