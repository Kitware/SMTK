

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/MembershipBadge.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/Color.h"
#include "smtk/io/Logger.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/FloatData.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/operators/SetProperty.h"
#include "smtk/view/Badge.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/IconFactory.h"
#include "smtk/view/Manager.h"

#include "smtk/view/icons/selected_svg.h"
#include "smtk/view/icons/unselected_svg.h"

#include <regex>

namespace smtk
{
namespace extension
{
namespace qt
{

MembershipBadge::MembershipBadge()
  : m_iconOn(selected_svg)
  , m_iconOff(unselected_svg)
  , m_parent(nullptr)
{
}

MembershipBadge::MembershipBadge(
  smtk::view::BadgeSet& parent, const smtk::view::Configuration::Component&)
  : m_iconOn(selected_svg)
  , m_iconOff(unselected_svg)
  , m_parent(&parent)
{
}

MembershipBadge::~MembershipBadge() = default;

std::string MembershipBadge::icon(
  const DescriptivePhrase* phrase, const std::array<float, 4>& background) const
{
  auto persistentObj = phrase->relatedObject();
  auto valIt = m_members.find(persistentObj);
  int member = 0;
  if (valIt != m_members.end())
  {
    member = valIt->second;
  }
  float lightness = smtk::common::Color::floatRGBToLightness(background.data());
  const std::string& icon = member ? m_iconOn : m_iconOff;
  return lightness >= 0.5 ? icon : std::regex_replace(icon, std::regex("black"), "white");
}

void MembershipBadge::action(const smtk::view::DescriptivePhrase* phrase)
{
  auto persistentObj = phrase->relatedObject();
  if (phrase->content() == nullptr || !persistentObj)
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(), "Can not access content or object for membership!");
    return;
  }
  auto valIt = m_members.find(persistentObj);
  if (valIt != m_members.end())
  {
    this->m_members[valIt->first] = !valIt->second;
  }
  else
  {
    this->m_members[persistentObj] = 1;
  }
  emit membershipChange(this->m_members[persistentObj]);
}
}
}
}
