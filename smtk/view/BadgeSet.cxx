//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/BadgeSet.h"
#include "smtk/view/Manager.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace view
{

void BadgeSet::configure(const Configuration* viewSpec, const smtk::view::ManagerPtr& manager)
{
  m_badges.clear();
  m_order.clear();
  if (!viewSpec)
  {
    return;
  }

  // Find the ordered list of badges in viewSpec.
  const smtk::view::Configuration::Component* badgeList = nullptr;
  int badgeListComp = viewSpec->details().findChild("Badges");
  if (badgeListComp < 0)
  {
    int phraseModelComp = viewSpec->details().findChild("PhraseModel");
    if (phraseModelComp >= 0)
    {
      auto& phraseConfig = viewSpec->details().child(phraseModelComp);
      badgeListComp = phraseConfig.findChild("Badges");
      if (badgeListComp >= 0)
      {
        auto& tmp = phraseConfig.child(badgeListComp);
        badgeList = &tmp;
      }
    }
  }
  else
  {
    auto& tmp = viewSpec->details().child(badgeListComp);
    badgeList = &tmp;
  }
  if (!badgeList)
  {
    // No badges configured.
    return;
  }
  for (std::size_t ii = 0; ii < badgeList->numberOfChildren(); ++ii)
  {
    std::string badgeName;
    auto& configComp(badgeList->child(ii));
    if (configComp.attribute("Type", badgeName))
    {
      // Populate the m_order map from values in viewSpec
      // WARNING: This *must* come before m_badges.insert() below or the badge will
      //          not be inserted in the correct order.
      m_order[badgeName] = static_cast<int>(ii);
      // Construct a badge with that name via the view manager:
      auto badge = manager->badgeFactory().create(badgeName, *this, configComp);
      if (badge)
      {
        m_badges.insert(badge);
      }
    }
    else
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Badge " << ii << " must have a type.");
    }
  }
}

/// Return ordered list of badge ptrs, ignoring any names without a matching badge.
std::vector<const Badge*> BadgeSet::badgesFor(const DescriptivePhrase* phrase) const
{
  std::vector<const Badge*> result;
  if (!phrase)
  {
    return result;
  }

  for (const auto& badge : m_badges)
  {
    if (badge->appliesToPhrase(phrase))
    {
      result.push_back(badge.get());
    }
  }
  return result;
}

/// Return an index for a badge representing its order in the container.
int BadgeSet::badgeIndex(const Badge::Ptr& badge) const
{
  auto it = m_order.find(badge->typeName());
  if (it == m_order.end())
  {
    return -1;
  }
  return it->second;
}
}
}
