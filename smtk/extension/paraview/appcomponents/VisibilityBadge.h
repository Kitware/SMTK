//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_paraview_appcomponents_VisibilityBadge_h
#define smtk_extension_paraview_appcomponents_VisibilityBadge_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/view/Badge.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/DescriptivePhrase.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace appcomponents
{
using DescriptivePhrase = smtk::view::DescriptivePhrase;

class SMTKPQCOMPONENTSEXT_EXPORT VisibilityBadge : public smtk::view::Badge
{
public:
  smtkTypeMacro(smtk::extension::paraview::appcomponents::VisibilityBadge);
  smtkSuperclassMacro(smtk::view::Badge);
  smtkSharedFromThisMacro(smtk::view::Badge);
  smtkCreateMacro(smtk::view::Badge);

  VisibilityBadge();
  VisibilityBadge(smtk::view::BadgeSet& parent, const smtk::view::Configuration::Component&);

  ~VisibilityBadge() override;

  bool appliesToPhrase(const DescriptivePhrase* phrase) const override;

  std::string tooltip(const DescriptivePhrase*) const override
  {
    return std::string("Click to toggle visibility");
  }
  std::string icon(const DescriptivePhrase* phrase, const std::array<float, 4>&) const override;

  /// take an action when the badge is clicked.
  void action(DescriptivePhrase* phrase) const override;

private:
  bool phraseVisibility(const DescriptivePhrase* phrase) const;
  // borrowed from paraview Qt/Components
  std::string m_icon;
  std::string m_iconClosed;
  // Selection state of items shown in m_phraseModel:
  std::map<smtk::common::UUID, int> m_visibleThings;
  const smtk::view::BadgeSet* m_parent;
};
}
}
}
}
#endif
