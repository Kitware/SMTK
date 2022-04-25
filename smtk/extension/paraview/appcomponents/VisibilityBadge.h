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
#include "smtk/extension/qt/VisibilityBadge.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/view/Badge.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseModel.h"

#include "smtk/io/Logger.h"

#include <QObject>

class pqView;
class pqRepresentation;

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace appcomponents
{

class SMTKPQCOMPONENTSEXT_EXPORT VisibilityBadge : public smtk::extension::qt::VisibilityBadge
{
  Q_OBJECT
public:
  smtkTypeMacro(smtk::extension::paraview::appcomponents::VisibilityBadge);
  smtkSuperclassMacro(smtk::extension::qt::VisibilityBadge);
  smtkSharedFromThisMacro(smtk::view::Badge);
  smtkCreateMacro(smtk::view::Badge);

  using DescriptivePhrase = smtk::view::DescriptivePhrase;
  using BadgeAction = smtk::view::BadgeAction;

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
  bool action(const DescriptivePhrase* phrase, const BadgeAction& act) override;

  /// Set visibility for this phrase.
  void setPhraseVisibility(const DescriptivePhrase* phrase, int val);
  bool phraseVisibility(const DescriptivePhrase* phrase) const;

protected Q_SLOTS:
  /// handle the active view changing, change the visible icons.
  void activeViewChanged(pqView* view);
  void representationAddedToActiveView(pqRepresentation* rep);
  void representationRemovedFromActiveView(pqRepresentation* rep);
  void componentVisibilityChanged(smtk::resource::ComponentPtr comp, bool visible);

protected:
  smtk::view::PhraseModel* phraseModel() const { return this->m_parent->phraseModel(); }

private:
  // borrowed from paraview Qt/Components
  std::string m_icon;
  std::string m_iconClosed;
  // Selection state of items shown in m_phraseModel:
  std::map<smtk::common::UUID, int> m_visibleThings;
  const smtk::view::BadgeSet* m_parent{ nullptr };
};
} // namespace appcomponents
} // namespace paraview
} // namespace extension
} // namespace smtk
#endif
