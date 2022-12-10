//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/LockedResourceBadge.h"

#include "smtk/view/BadgeSet.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/Manager.h"

#include "smtk/common/Color.h"

#include "smtk/view/icons/lock_locked_cpp.h"

#include "smtk/Regex.h"

namespace smtk
{
namespace view
{

LockedResourceBadge::LockedResourceBadge() = default;

LockedResourceBadge::LockedResourceBadge(BadgeSet& parent, const Configuration::Component&)
  : m_parent(&parent)
{
}

LockedResourceBadge::~LockedResourceBadge()
{
  m_parent = nullptr;
}

bool LockedResourceBadge::appliesToPhrase(const DescriptivePhrase* phrase) const
{
  using namespace smtk::resource;
  if (!m_parent || !phrase)
  {
    return false;
  }
  auto obj = phrase->relatedObject();
  if (!obj)
  {
    return false;
  }
  auto rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(obj);
  return rsrc && rsrc->locked() != LockType::Unlocked;
}

std::string LockedResourceBadge::tooltip(const DescriptivePhrase* phrase) const
{
  using namespace smtk::resource;
  std::string result;
  if (!m_parent || !phrase)
  {
    return result;
  }
  auto rsrc = phrase->relatedResource();
  if (!rsrc)
  {
    return result;
  }
  switch (rsrc->locked())
  {
    case LockType::Unlocked:
      result = "This resource is not locked.";
      break;
    case LockType::Read:
      result = "This resource is read-locked.";
      break;
    case LockType::Write:
      result = "This resource is write-locked.";
      break;
    default:
      result = "This resource has an unknown lock type.";
      break;
  }
  return result;
}

std::string LockedResourceBadge::icon(
  const DescriptivePhrase* phrase,
  const std::array<float, 4>& background) const
{
  std::string icon;
  if (!m_parent || !phrase)
  { // NB: The check on m_parent prevents color with no icon in some cases.
    return icon;
  }
  smtk::view::ManagerPtr manager = m_parent->manager();
  if (!manager)
  { // NB: This check prevents color with no icon in some cases.
    return icon;
  }
  auto obj = phrase->relatedObject();
  if (!obj)
  {
    return icon;
  }
  float lightness = smtk::common::Color::floatRGBToLightness(background.data());
  icon = lock_locked_svg();
  icon = smtk::regex_replace(icon, smtk::regex("#ff7f2a"), lightness < 0.5 ? "#bebebe" : "#2a2a2a");
  return icon;
}
} // namespace view
} // namespace smtk
