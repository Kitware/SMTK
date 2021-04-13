//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_ObjectIconBadge_h
#define smtk_view_ObjectIconBadge_h

#include "smtk/view/Badge.h"

namespace smtk
{
namespace view
{
class BadgeSet;
class Manager;

/**\brief A badge that illustrates the type and color of a persistent object.
  *
  */
class SMTKCORE_EXPORT ObjectIconBadge : public Badge
{
public:
  smtkTypeMacro(smtk::view::ObjectIconBadge);
  ObjectIconBadge();
  ObjectIconBadge(BadgeSet&, const Configuration::Component&);
  virtual ~ObjectIconBadge();

  /// This badge only applies to phrases with a persistent object that has an icon.
  bool appliesToPhrase(const DescriptivePhrase* phrase) const override;

  /// The tool-tip string is the object's type.
  std::string tooltip(const DescriptivePhrase* phrase) const override;

  /// Returns an SVG string for rendering the badge icon.
  ///
  /// This uses a ObjectIcons to generate the badge.
  std::string icon(const DescriptivePhrase* phrase, const std::array<float, 4>& background)
    const override;

  // No action is taken when the badge is clicked.
  //
  // Subclass this badge if you wish to implement an action.
  // void action(const DescriptivePhrase* phrase) const override { }

protected:
  const BadgeSet* m_parent;
};
} // namespace view
} // namespace smtk
#endif
