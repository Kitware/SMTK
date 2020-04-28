//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/ObjectIconBadge.h"

#include "smtk/view/BadgeSet.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/IconFactory.h"
#include "smtk/view/Manager.h"

#include "smtk/common/Color.h"

namespace smtk
{
namespace view
{

ObjectIconBadge::ObjectIconBadge()
  : m_parent(nullptr)
{
}

ObjectIconBadge::ObjectIconBadge(BadgeSet& parent, const Configuration::Component&)
  : m_parent(&parent)
{
}

ObjectIconBadge::~ObjectIconBadge()
{
  m_parent = nullptr;
}

bool ObjectIconBadge::appliesToPhrase(const DescriptivePhrase* phrase) const
{
  if (!m_parent || !phrase)
  { // NB: The check on m_parent prevents color with no icon in some cases.
    return false;
  }
  smtk::view::ManagerPtr manager = m_parent->manager();
  if (!manager)
  { // NB: This check prevents color with no icon in some cases.
    return false;
  }
  auto obj = phrase->relatedObject();
  return !!obj;
}

std::string ObjectIconBadge::tooltip(const DescriptivePhrase* phrase) const
{
  std::string result;
  if (!m_parent || !phrase)
  { // NB: The check on m_parent prevents color with no icon in some cases.
    return result;
  }
  auto obj = phrase->relatedObject();
  if (!obj)
  {
    return result;
  }
  return obj->typeName();
}

std::string ObjectIconBadge::svg(
  const DescriptivePhrase* phrase, const std::array<float, 4>& background) const
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
  std::string bg = smtk::common::Color::floatRGBToString(background.data());
  icon = manager->iconFactory().createIcon(*obj, bg);
  return icon;
}
}
}
