//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_LockedResourceBadge_h
#define smtk_view_LockedResourceBadge_h

#include "smtk/view/Badge.h"

namespace smtk
{
namespace view
{
class BadgeSet;
class Manager;

/**\brief A badge that appears next to resources that are locked.
  *
  */
class SMTKCORE_EXPORT LockedResourceBadge : public Badge
{
public:
  smtkTypeMacro(smtk::view::LockedResourceBadge);
  LockedResourceBadge();
  LockedResourceBadge(BadgeSet&, const Configuration::Component&);
  ~LockedResourceBadge() override;

  /// This badge only applies to phrases whose subject is a locked resource.
  bool appliesToPhrase(const DescriptivePhrase* phrase) const override;

  /// The tool-tip string describes the lock type (read/write).
  std::string tooltip(const DescriptivePhrase* phrase) const override;

  /// Returns an SVG string for rendering the badge icon.
  std::string icon(const DescriptivePhrase* phrase, const std::array<float, 4>& background)
    const override;

  // No action is taken when the badge is clicked.
  //
  // Subclass this badge if you wish to implement an action.
  // void action(const DescriptivePhrase* phrase) const override { }

protected:
  const BadgeSet* m_parent{ nullptr };
};
} // namespace view
} // namespace smtk
#endif
