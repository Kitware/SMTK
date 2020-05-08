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

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceBrowser.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceRepresentation.h"

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
#include "smtk/view/IconFactory.h"
#include "smtk/view/Manager.h"

#include "pqActiveObjects.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace appcomponents
{

template <typename T, typename U>
int UpdateVisibilityForFootprint(pqSMTKResourceRepresentation* smap, const T& comp, int visible,
  U& visibleThings, const smtk::view::DescriptivePhrasePtr& /*unused*/)
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
          int ok = smap->setVisibility(child.component(), visible != 0);
          any |= ok;
          visibleThings[child.entity()] = visible;
        }
      }
      rval |= any;

      rval |= smap->setVisibility(comp, visible != 0) ? 1 : 0;
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
    rval |= smap->setVisibility(comp, visible != 0) ? 1 : 0;
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

/// Types of queries for visibility.
enum Query
{
  DISPLAYABLE,
  EDITABLE,
  GET_VALUE,
  SET_VALUE
};

int vizQuery(Query query, int val, const smtk::view::ConstPhraseContentPtr data,
  std::map<smtk::common::UUID, int>& visibleThings)
{
  auto comp = data->relatedComponent();
  auto rsrc = data->relatedResource();

  smtk::model::EntityPtr ent = std::dynamic_pointer_cast<smtk::model::Entity>(comp);
  smtk::model::ResourcePtr modelRsrc =
    ent ? ent->modelResource() : std::dynamic_pointer_cast<smtk::model::Resource>(rsrc);

  smtk::mesh::ComponentPtr msh = std::dynamic_pointer_cast<smtk::mesh::Component>(comp);
  smtk::mesh::ResourcePtr meshRsrc = msh
    ? std::dynamic_pointer_cast<smtk::mesh::Resource>(msh->resource())
    : std::dynamic_pointer_cast<smtk::mesh::Resource>(rsrc);

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
  bool validView = pqActiveObjects::instance().activeView() != nullptr;

  switch (query)
  {
    case DISPLAYABLE:
      return validView && (ent || (!ent && modelRsrc) || (msh || (!ent && meshRsrc))) ? 1 : 0;
    case EDITABLE:
      return validView && (ent || (!ent && modelRsrc) || (msh || (!ent && meshRsrc))) ? 1 : 0;
    case GET_VALUE:
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
    case SET_VALUE:
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

VisibilityBadge::VisibilityBadge()
  : m_parent(nullptr)
{
}
VisibilityBadge::VisibilityBadge(
  smtk::view::BadgeSet& parent, const smtk::view::Configuration::Component&)
  : m_icon("<svg id=\"Layer_1\" data-name=\"Layer 1\" xmlns=\"http://www.w3.org/2000/svg\" "
           "viewBox=\"0 0 64 64\"><title>SVG_Artboards</title><path "
           "d=\"M59.94,31.86S47.49,14.37,31.31,14.37c-16.81,0-28.1,17.49-28.1,17.49S14.5,49.35,"
           "31.31,49.35C47.49,49.35,59.94,31.86,59.94,31.86Z\" style=\"fill:#fff\"/><path "
           "d=\"M31.31,50.48c-17.22,0-28.58-17.27-29-18a1.12,1.12,0,0,1,0-1.22c.47-.73,11.83-18,"
           "29.05-18,16.57,0,29,17.23,29.54,18a1.13,1.13,0,0,1,0,1.31C60.33,33.25,47.88,50.48,31."
           "31,50.48ZM4.57,31.86c2.18,3,12.51,16.37,26.74,16.37C45,48.23,56.09,35,58.52,31.86,56."
           "09,28.74,45,15.5,31.31,15.5,17.08,15.5,6.75,28.82,4.57,31.86Z\" "
           "style=\"fill:#787878\"/><circle cx=\"31.57\" cy=\"31.86\" r=\"16.19\" "
           "style=\"fill:#2896d3\"/><circle cx=\"31.57\" cy=\"31.86\" r=\"9.51\" "
           "style=\"fill:#12141c\"/><g style=\"opacity:0.9\"><circle cx=\"24.2\" cy=\"24.17\" "
           "r=\"6.65\" style=\"fill:#fff\"/></g></svg>")
  , m_iconClosed("<svg id=\"Layer_1\" data-name=\"Layer 1\" xmlns=\"http://www.w3.org/2000/svg\" "
                 "viewBox=\"0 0 64 64\"><title>SVG_Artboards</title><path "
                 "d=\"M31.47,45.11c-12.45,0-21.59-9.19-22-9.58A1.13,1.13,0,0,1,11.1,34c.09.09,8."
                 "88,8.91,20.37,8.91s20.28-9.23,20.36-9.33a1.14,1.14,0,0,1,1.6-.05,1.13,1.13,0,0,"
                 "1,0,1.59C53.09,35.48,43.93,45.11,31.47,45.11Z\" style=\"fill:#787878\"/><path "
                 "d=\"M10.3,26.11a1.13,1.13,0,0,1-.77-.3,1.11,1.11,0,0,1,0-1.59c.38-.41,9.55-10,"
                 "22-10s21.59,9.19,22,9.58a1.12,1.12,0,0,1,0,1.59,1.13,1.13,0,0,1-1.6,0c-.09-.09-"
                 "8.88-8.91-20.36-8.91s-20.28,9.23-20.37,9.33A1.14,1.14,0,0,1,10.3,26.11Z\" "
                 "style=\"fill:#787878\"/><path "
                 "d=\"M31.47,52.6a1.13,1.13,0,0,1-1.13-1.13V44a1.13,1.13,0,1,1,2.25,0v7.48A1.13,"
                 "1.13,0,0,1,31.47,52.6Z\" style=\"fill:#787878\"/><path "
                 "d=\"M21.43,51.14a1.12,1.12,0,0,1-1.09-1.44l2.12-7.18a1.13,1.13,0,1,1,2.16.64l-"
                 "2.11,7.18A1.13,1.13,0,0,1,21.43,51.14Z\" style=\"fill:#787878\"/><path "
                 "d=\"M41.5,51.14a1.13,1.13,0,0,1-1.08-.8l-2.11-7.18a1.13,1.13,0,0,1,2.16-.64l2."
                 "12,7.18a1.12,1.12,0,0,1-1.09,1.44Z\" style=\"fill:#787878\"/><path "
                 "d=\"M51.78,45.94a1.13,1.13,0,0,1-.88-.42l-4.69-5.83A1.12,1.12,0,0,1,48,38.28l4."
                 "7,5.83a1.13,1.13,0,0,1-.88,1.83Z\" style=\"fill:#787878\"/><path "
                 "d=\"M11.15,45.94a1.13,1.13,0,0,1-.88-1.83l4.57-5.67a1.13,1.13,0,0,1,1.76,1."
                 "41L12,45.52A1.11,1.11,0,0,1,11.15,45.94Z\" style=\"fill:#787878\"/></svg>")
  , m_parent(&parent)
{
}

VisibilityBadge::~VisibilityBadge()
{
  m_parent = nullptr;
}

bool VisibilityBadge::appliesToPhrase(const DescriptivePhrase* phrase) const
{
  return !!vizQuery(
    Query::DISPLAYABLE, -1, phrase->content(), const_cast<VisibilityBadge*>(this)->m_visibleThings);
}

bool VisibilityBadge::phraseVisibility(const DescriptivePhrase* phrase) const
{
  return !!vizQuery(
    Query::GET_VALUE, -1, phrase->content(), const_cast<VisibilityBadge*>(this)->m_visibleThings);
}

std::string VisibilityBadge::icon(
  const DescriptivePhrase* phrase, const std::array<float, 4>& /*background*/) const
{
  if (this->phraseVisibility(phrase))
    return m_icon;
  return m_iconClosed;
}

void VisibilityBadge::action(const DescriptivePhrase* phrase) const
{
  int newVal = !this->phraseVisibility(phrase) ? 1 : 0;
  // if (!phrase->setRelatedVisibility(!phraseVisibility(phrase)))
  // {
  //   smtkErrorMacro(
  //     smtk::io::Logger::instance(), "Could not toggle visibility of \"" << phrase->title() <<
  //     "\"");
  //   return;
  // }
  vizQuery(Query::SET_VALUE, newVal, phrase->content(),
    const_cast<VisibilityBadge*>(this)->m_visibleThings);
  auto model = phrase->phraseModel();
  if (model)
  {
    model->triggerDataChanged();
  }
}
}
}
}
}
