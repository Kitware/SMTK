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
      const auto& phraseConfig = viewSpec->details().child(phraseModelComp);
      badgeListComp = phraseConfig.findChild("Badges");
      if (badgeListComp >= 0)
      {
        const auto& tmp = phraseConfig.child(badgeListComp);
        badgeList = &tmp;
      }
    }
  }
  else
  {
    const auto& tmp = viewSpec->details().child(badgeListComp);
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
    const auto& configComp(badgeList->child(ii));
    if (configComp.name() == "Badge")
    {
      if (configComp.attribute("Type", badgeName))
      {
        // Construct a badge with that name via the view manager:
        auto badge = manager->badgeFactory().createFromName(badgeName, *this, configComp);
        if (badge)
        {
          badge->setIsDefault(configComp.attributeAsBool("Default"));
          m_badges.push_back(std::move(badge));
        }
        else
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Badge " << ii << " (" << badgeName << ") could not be constructed.");
        }
      }
      else
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "Badge " << ii << " must have a type.");
      }
    }
    else if (configComp.name() != "Comment")
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Unknown Badges entry " << configComp.name() << " (" << ii << ") will be ignored.");
    }
  }
}

/// Return ordered list of badge ptrs, ignoring any names without a matching badge.
BadgeSet::BadgeList BadgeSet::badgesFor(const DescriptivePhrase* phrase) const
{
  BadgeSet::BadgeList result;
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
} // namespace view
} // namespace smtk
