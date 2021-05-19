

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
{
}

MembershipBadge::MembershipBadge(
  smtk::view::BadgeSet& parent,
  const smtk::view::Configuration::Component& config)
  : m_iconOn(selected_svg)
  , m_iconOff(unselected_svg)
  , m_parent(&parent)
{
  config.attributeAsBool("SingleSelect", m_singleSelect);
  if (config.attribute("MemberIcon"))
  {
    config.attribute("MemberIcon", m_iconOn);
  }
  if (config.attribute("NonMemberIcon"))
  {
    config.attribute("NonMemberIcon", m_iconOff);
  }
}

MembershipBadge::~MembershipBadge() = default;

std::string MembershipBadge::icon(
  const DescriptivePhrase* phrase,
  const std::array<float, 4>& background) const
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

bool MembershipBadge::action(
  const smtk::view::DescriptivePhrase* phrase,
  const smtk::view::BadgeAction& act)
{
  using smtk::view::DescriptivePhrase;

  if (!dynamic_cast<const smtk::view::BadgeActionToggle*>(&act))
  {
    return false; // we only support toggling.
  }

  auto persistentObj = phrase->relatedObject();
  if (phrase->content() == nullptr || !persistentObj)
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(), "Can not access content or object for membership!");
    return false;
  }
  auto valIt = m_members.find(persistentObj);
  int newValue = (valIt == m_members.end()) ? 1 : !valIt->second;

  if (m_singleSelect)
  {
    // Selecting a new item when only 1 is allowed should reset all other membership
    // and ignore any multiple-selection passed via the action.
    if (newValue && !m_members.empty())
    {
      m_members.clear();
    }
    m_members[persistentObj] = newValue;
    emit membershipChange(newValue);
    return true;
  }

  bool didVisit = false;
  act.visitRelatedPhrases([this, newValue, &didVisit](const DescriptivePhrase* related) -> bool {
    auto obj = related->relatedObject();
    if (obj)
    {
      auto it = m_members.find(obj);
      if (it == m_members.end())
      {
        if (newValue)
        {
          m_members[obj] = newValue;
          didVisit = true;
        }
      }
      else
      {
        if (it->second != newValue)
        {
          it->second = newValue;
          didVisit = true;
        }
      }
    }
    return false; // do not terminate early
  });
  if (!didVisit)
  {
    m_members[persistentObj] = newValue;
  }
  emit membershipChange(newValue);
  return true;
}
} // namespace qt
} // namespace extension
} // namespace smtk
