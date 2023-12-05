//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/appcomponents/HierarchicalVisibilityBadge.h"
#include "smtk/extension/paraview/appcomponents/GeometricVisibilityBadge.h"

#include "smtk/view/DescriptivePhrase.h"

#include "smtk/view/icons/tree_blanked_cpp.h"
#include "smtk/view/icons/tree_partial_cpp.h"
#include "smtk/view/icons/tree_visible_cpp.h"

// ParaView headers
#include "pqActiveObjects.h"
#include "pqView.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace appcomponents
{

HierarchicalVisibilityBadge::HierarchicalVisibilityBadge()
  : m_iconVisible(tree_visible_svg())
  , m_iconPartial(tree_partial_svg())
  , m_iconBlanked(tree_blanked_svg())
{
}

HierarchicalVisibilityBadge::HierarchicalVisibilityBadge(
  smtk::view::BadgeSet& parent,
  const smtk::view::Configuration::Component&)
  : m_iconVisible(tree_visible_svg())
  , m_iconPartial(tree_partial_svg())
  , m_iconBlanked(tree_blanked_svg())
  , m_parent(&parent)
{
  // NB: For now, we assume only one geometric visibility badge exists in the set.
  m_partner = m_parent->findBadgeOfType<GeometricVisibilityBadge>();
}

HierarchicalVisibilityBadge::~HierarchicalVisibilityBadge()
{
  m_parent = nullptr;
}

bool HierarchicalVisibilityBadge::appliesToPhrase(const DescriptivePhrase* phrase) const
{
  return phrase && !phrase->subphrases().empty();
}

std::string HierarchicalVisibilityBadge::tooltip(const DescriptivePhrase*) const
{
  return std::string("Toggle the visibility of this item's children.");
}

std::string HierarchicalVisibilityBadge::icon(
  const DescriptivePhrase* phrase,
  const std::array<float, 4>& /*background*/) const
{
  switch (this->phraseVisibility(phrase))
  {
    case GeometricVisibilityBadge::VisibilityState::Visible:
      return m_iconVisible;
    case GeometricVisibilityBadge::VisibilityState::Invisible:
      return m_iconBlanked;
    case GeometricVisibilityBadge::VisibilityState::Neither:
      return m_iconPartial;
  }
  return m_iconPartial; // TODO: Warn?
}

bool HierarchicalVisibilityBadge::action(const DescriptivePhrase* phrase, const BadgeAction& act)
{
  if (!dynamic_cast<const smtk::view::BadgeActionToggle*>(&act))
  {
    return false; // we only support toggling.
  }

  int newVal =
    this->phraseVisibility(phrase) == GeometricVisibilityBadge::VisibilityState::Invisible ? 1 : 0;
  bool didVisit = false;

  act.visitRelatedPhrases([this, newVal, &didVisit](const DescriptivePhrase* related) -> bool {
    didVisit = true;
    this->setPhraseVisibility(const_cast<DescriptivePhrase*>(related), newVal);
    return false;
  });

  // If the UI component did not provide a set of related phrases, at least
  // toggle visibility of our primary phrase:
  if (!didVisit)
  {
    this->setPhraseVisibility(const_cast<DescriptivePhrase*>(phrase), newVal);
  }

  auto model = phrase->phraseModel();
  if (model)
  {
    model->triggerDataChanged();
  }
  return true;
}

GeometricVisibilityBadge::VisibilityState HierarchicalVisibilityBadge::phraseVisibility(
  const DescriptivePhrase* phrase) const
{
  if (!m_partner)
  {
    const_cast<HierarchicalVisibilityBadge*>(this)->m_partner =
      m_parent->findBadgeOfType<GeometricVisibilityBadge>();
    if (!m_partner)
    {
      return GeometricVisibilityBadge::VisibilityState::Neither;
    }
  }
  auto it = m_partner->phraseCache().phraseInfos().find(const_cast<DescriptivePhrase*>(phrase));
  if (it == m_partner->phraseCache().phraseInfos().end())
  {
    return GeometricVisibilityBadge::VisibilityState::Neither;
  }
  return it->second.calculateHierarchicalVisibility(phrase);
}

void recursivelySetVisibility(
  HierarchicalVisibilityBadge* hbadge,
  GeometricVisibilityBadge* gbadge,
  smtk::view::DescriptivePhrase* phrase,
  bool visible)
{
  if (gbadge->appliesToPhrase(phrase))
  {
    gbadge->setPhraseVisibility(phrase, visible);
  }
  for (const auto& child : phrase->subphrases())
  {
    recursivelySetVisibility(hbadge, gbadge, child.get(), visible);
  }
}

void HierarchicalVisibilityBadge::setPhraseVisibility(DescriptivePhrase* phrase, bool visible)
{
  if (!m_partner)
  {
    m_partner = m_parent->findBadgeOfType<GeometricVisibilityBadge>();
    if (!m_partner)
    {
      return;
    }
  }
  for (const auto& child : phrase->subphrases())
  {
    recursivelySetVisibility(this, m_partner, child.get(), visible);
  }
}

} // namespace appcomponents
} // namespace paraview
} // namespace extension
} // namespace smtk
