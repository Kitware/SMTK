//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/appcomponents/VisibilityBadge.h"

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

#include "smtk/extension/paraview/appcomponents/pqEyeballClosed_svg.h"
#include "smtk/extension/paraview/appcomponents/pqEyeball_svg.h"

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
#include "vtkSMProxy.h"

namespace
{

pqSMTKResourceRepresentation* representationInView(
  const std::shared_ptr<smtk::resource::Resource>& rsrc,
  pqView* view = nullptr)
{
  auto* smtkBehavior = pqSMTKBehavior::instance();

  // Find the ParaView pipeline for the resource
  auto pvrc = smtkBehavior->getPVResource(rsrc);
  if (!pvrc)
  {
    return nullptr;
  }

  // Find the mapper in the active view for the related resource.
  if (!view)
  {
    view = pqActiveObjects::instance().activeView();
  }
  auto* mapr = pvrc->getRepresentation(view);
  auto* smap = dynamic_cast<pqSMTKResourceRepresentation*>(mapr);
  return smap;
}

} // anonymous namespace

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace appcomponents
{

template<typename T, typename U>
int UpdateVisibilityForFootprint(
  pqSMTKResourceRepresentation* smap,
  const T& comp,
  int visible,
  U& visibleThings,
  const smtk::view::DescriptivePhrase* /*unused*/)
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
        int ok = smap ? smap->setVisibility(child, visible) : true;
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
      // Composite auxiliary geometry condition
      int any = 0;
      smtk::model::AuxiliaryGeometry auxgeom =
        ment->template referenceAs<smtk::model::AuxiliaryGeometry>();
      auto auxgeomChildren = auxgeom.auxiliaryGeometries();
      if (auxgeom && !auxgeomChildren.empty())
      {
        for (const auto& child : auxgeomChildren)
        {
          int ok = smap ? smap->setVisibility(child.component(), visible != 0) : true;
          any |= ok;
          visibleThings[child.entity()] = visible;
        }
      }
      rval |= any;

      rval |= smap ? (smap->setVisibility(comp, visible != 0) ? 1 : 0) : 1;
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
    rval |= smap ? (smap->setVisibility(comp, visible != 0) ? 1 : 0) : 1;
    if (rval)
    {
      visibleThings[comp->id()] = visible;
      didUpdate = true;
    }
  }
  else if (comp)
  {
    auto resource = std::dynamic_pointer_cast<smtk::geometry::Resource>(comp->resource());
    if (resource)
    {
      smtk::extension::vtk::geometry::Backend vtk;
      // const auto& geom = resource->geometry(vtk);
      // if (geom)
      if (resource->queries().template contains<smtk::geometry::SelectionFootprint>())
      {
        const auto& query = resource->queries().template get<smtk::geometry::SelectionFootprint>();
        std::unordered_set<smtk::resource::PersistentObject*> footprint;
        query(*comp, footprint, vtk);
        // Even if the footprint does not include the component itself, we need to include it
        // here so that the descriptive phrase shows a response to user input.
        visibleThings[comp->id()] = visible;
        for (const auto& item : footprint)
        {
          visibleThings[item->id()] = visible;
          auto* itemComp = dynamic_cast<smtk::resource::Component*>(item);
          if (itemComp)
          {
            int vv = 0;
            if (itemComp->resource() == resource)
            {
              vv = smap ? (smap->setVisibility(itemComp->shared_from_this(), visible != 0) ? 1 : 0)
                        : 1;
            }
            else
            {
              auto* rep = representationInView(itemComp->resource());
              if (rep)
              {
                vv = rep->setVisibility(itemComp->shared_from_this(), visible != 0) ? 1 : 0;
              }
            }
            rval |= vv;
            if (vv)
            {
              didUpdate = true;
            }
          }
        }
      }
      else
      {
        visibleThings[comp->id()] = visible;
        int vv = smap ? (smap->setVisibility(comp, visible != 0) ? 1 : 0) : 1;
        rval |= vv;
        if (vv)
        {
          didUpdate = true;
        }
      }
    }
  }

  if (didUpdate && smap)
  {
    smap->renderViewEventually();
  }
  return rval;
}

VisibilityBadge::VisibilityBadge()
  : m_icon(pqEyeball_svg)
  , m_iconClosed(pqEyeballClosed_svg)
{
}

VisibilityBadge::VisibilityBadge(
  smtk::view::BadgeSet& parent,
  const smtk::view::Configuration::Component&)
  : m_icon(pqEyeball_svg)
  , m_iconClosed(pqEyeballClosed_svg)
  , m_parent(&parent)
{
  // Reset eyeball icons when the active view changes:
  pqActiveObjects& act(pqActiveObjects::instance());
  QObject::connect(&act, SIGNAL(viewChanged(pqView*)), this, SLOT(activeViewChanged(pqView*)));
  // Now call immediately, since in at least some circumstances, a view may already be active.
  if (this->phraseModel())
  {
    this->activeViewChanged(act.activeView());
  }
}

VisibilityBadge::~VisibilityBadge()
{
  m_parent = nullptr;
}

bool VisibilityBadge::appliesToPhrase(const DescriptivePhrase* phrase) const
{
  auto rsrc = phrase->relatedResource();
  smtk::geometry::ResourcePtr geomRsrc = std::dynamic_pointer_cast<smtk::geometry::Resource>(rsrc);

  bool validView = pqActiveObjects::instance().activeView() != nullptr;
  if (validView && geomRsrc)
  {
    auto att = std::dynamic_pointer_cast<smtk::attribute::Attribute>(phrase->relatedComponent());
    if (!att)
    {
      return true;
    }
    // Only show the visibility badge on attributes that explicitly have renderable geometry.
    smtk::extension::vtk::geometry::Backend vtk;
    const auto& geom = geomRsrc->geometry(vtk);
    if (geom && geom->generationNumber(att) != smtk::geometry::Geometry::Invalid)
    {
      return true;
    }
  }
  return false;
}

bool VisibilityBadge::phraseVisibility(const DescriptivePhrase* phrase) const
{
  auto comp = phrase->relatedComponent();
  auto rsrc = phrase->relatedResource();

  smtk::model::EntityPtr ent = std::dynamic_pointer_cast<smtk::model::Entity>(comp);
  smtk::model::ResourcePtr modelRsrc =
    ent ? ent->modelResource() : std::dynamic_pointer_cast<smtk::model::Resource>(rsrc);

  smtk::mesh::ComponentPtr msh = std::dynamic_pointer_cast<smtk::mesh::Component>(comp);
  smtk::mesh::ResourcePtr meshRsrc = msh
    ? std::dynamic_pointer_cast<smtk::mesh::Resource>(msh->resource())
    : std::dynamic_pointer_cast<smtk::mesh::Resource>(rsrc);

  smtk::geometry::ResourcePtr geomRsrc = std::dynamic_pointer_cast<smtk::geometry::Resource>(rsrc);

  auto* smtkBehavior = pqSMTKBehavior::instance();

  auto pvrc = smtkBehavior->getPVResource(rsrc);
  if (!pvrc)
  {
    return true; // pipeline hasn't been created yet; the default is visible.
  }

  if (ent || msh)
  {
    auto valIt = m_visibleThings.find(comp->id());
    if (valIt != m_visibleThings.end())
    {
      return valIt->second;
    }
    return true; // visibility is assumed if there is no entry.
  }
  else if (modelRsrc || meshRsrc || (geomRsrc && !comp))
  {
    auto* view = pqActiveObjects::instance().activeView();
    auto* mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
    return mapr ? mapr->isVisible() : false;
  }
  else if (geomRsrc)
  {
    auto valIt = m_visibleThings.find(comp->id());
    if (valIt != m_visibleThings.end())
    {
      return valIt->second;
    }
    return true;
  }
  return false; // visibility is false if the component is not a model entity or nullptr.
}

void VisibilityBadge::setPhraseVisibility(const DescriptivePhrase* phrase, int val)
{
  auto comp = phrase->relatedComponent();
  auto rsrc = phrase->relatedResource();

  smtk::model::EntityPtr ent = std::dynamic_pointer_cast<smtk::model::Entity>(comp);
  smtk::model::ResourcePtr modelRsrc =
    ent ? ent->modelResource() : std::dynamic_pointer_cast<smtk::model::Resource>(rsrc);

  smtk::mesh::ComponentPtr msh = std::dynamic_pointer_cast<smtk::mesh::Component>(comp);
  smtk::mesh::ResourcePtr meshRsrc = msh
    ? std::dynamic_pointer_cast<smtk::mesh::Resource>(msh->resource())
    : std::dynamic_pointer_cast<smtk::mesh::Resource>(rsrc);

  smtk::geometry::ResourcePtr geomRsrc = std::dynamic_pointer_cast<smtk::geometry::Resource>(rsrc);

  auto* smtkBehavior = pqSMTKBehavior::instance();

  auto pvrc = smtkBehavior->getPVResource(rsrc);

  if (ent || msh || (comp && geomRsrc))
  { // Find the mapper in the active view for the related resource, then set the visibility.
    auto* view = pqActiveObjects::instance().activeView();
    auto* mapr = pvrc ? pvrc->getRepresentation(view) : nullptr;
    auto* smap = dynamic_cast<pqSMTKResourceRepresentation*>(mapr);
    UpdateVisibilityForFootprint(smap, comp, val, m_visibleThings, phrase);
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
  }
}

std::string VisibilityBadge::icon(
  const DescriptivePhrase* phrase,
  const std::array<float, 4>& /*background*/) const
{
  if (this->phraseVisibility(phrase))
    return m_icon;
  return m_iconClosed;
}

bool VisibilityBadge::action(const DescriptivePhrase* phrase, const BadgeAction& act)
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

  auto model = phrase->phraseModel();
  if (model)
  {
    model->triggerDataChanged();
  }
  return true;
}

void VisibilityBadge::activeViewChanged(pqView* view)
{
  // Disconnect old representations, clear local visibility map.
  QObject::disconnect(this, SLOT(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)));
  m_visibleThings.clear();
  // Connect new representations, initialize visibility map..
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
  if (!this->phraseModel()->root())
  {
    return;
  }
  auto rsrcPhrases = this->phraseModel()->root()->subphrases();
  auto* behavior = pqSMTKBehavior::instance();
  for (const auto& rsrcPhrase : rsrcPhrases)
  {
    auto rsrc = rsrcPhrase->relatedResource();
    if (!rsrc)
    {
      continue;
    }
    auto pvr = behavior->getPVResource(rsrc);
    auto* rep = pvr ? pvr->getRepresentation(view) : nullptr;
    // TODO: At a minimum, we can update the representation's visibility now
    //       since if rep is null it is invisible and if not null, we can ask
    //       for its visibility.
    if (rep)
    {
      m_visibleThings[rsrc->id()] = rep->isVisible() ? 1 : 0;
      auto* thingy = rep->getProxy()->GetClientSideObject();
      auto* thingy2 = vtkCompositeRepresentation::SafeDownCast(thingy);
      auto* srvrep = vtkSMTKResourceRepresentation::SafeDownCast(
        thingy2 ? thingy2->GetActiveRepresentation() : nullptr);
      if (srvrep)
      {
        // TODO: This assumes we are running in built-in mode. Remove the need for me.
        srvrep->GetEntityVisibilities(m_visibleThings);
      }
    }
    else
    {
      // This is a sign that things are going poorly.
      // The representation should already have been created either when
      // the view was created or the resource loaded.
      m_visibleThings[rsrc->id()] = behavior->createRepresentation(pvr, view) ? 1 : 0;
    }
  }
  // Indicate to the Qt model that it needs to refresh every row,
  // since visibility may be altered on each one:
  this->phraseModel()->triggerDataChanged();
}

void VisibilityBadge::representationAddedToActiveView(pqRepresentation* rep)
{
  auto* smtkRep = dynamic_cast<pqSMTKResourceRepresentation*>(rep);
  if (smtkRep)
  {
    QObject::connect(
      smtkRep,
      SIGNAL(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)),
      this,
      SLOT(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)));
  }
}

void VisibilityBadge::representationRemovedFromActiveView(pqRepresentation* rep)
{
  auto* smtkRep = dynamic_cast<pqSMTKResourceRepresentation*>(rep);
  if (smtkRep)
  {
    auto* pipeline = dynamic_cast<pqSMTKResource*>(smtkRep->getInput());
    if (pipeline)
    {
      auto resource = pipeline->getResource();
      // Ensure that when a representation is removed due to
      // the resource being closed that we "forget" the visibility
      // state of its components — otherwise, reloading the resource
      // will result in inconsistent state.
      if (!this->phraseModel()->root())
      {
        return;
      }
      auto rsrcPhrases = this->phraseModel()->root()->subphrases();
      std::function<void(const smtk::view::DescriptivePhrase::Ptr&)> updater =
        [this, &resource, &updater](const smtk::view::DescriptivePhrase::Ptr& phrase) {
          if (phrase && phrase->relatedResource() == resource)
          {
            m_visibleThings.erase(phrase->relatedObject()->id());
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
    }
    QObject::disconnect(
      smtkRep,
      SIGNAL(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)),
      this,
      SLOT(componentVisibilityChanged(smtk::resource::ComponentPtr, bool)));
  }
}

void VisibilityBadge::componentVisibilityChanged(smtk::resource::ComponentPtr comp, bool visible)
{
  // The visibility should change for every row displaying the same \a comp:
  m_visibleThings[comp->id()] = visible;
}
} // namespace appcomponents
} // namespace paraview
} // namespace extension
} // namespace smtk
