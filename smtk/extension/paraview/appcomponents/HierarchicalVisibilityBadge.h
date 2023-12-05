//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_paraview_appcomponents_HierarchicalVisibilityBadge_h
#define smtk_extension_paraview_appcomponents_HierarchicalVisibilityBadge_h

#include "smtk/extension/paraview/appcomponents/GeometricVisibilityBadge.h"

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

///\brief A badge to show/toggle the visibility of all of a phrase's children.
class SMTKPQCOMPONENTSEXT_EXPORT HierarchicalVisibilityBadge
  : public smtk::extension::qt::VisibilityBadge
{
  Q_OBJECT
public:
  using DescriptivePhrase = smtk::view::DescriptivePhrase;
  using BadgeAction = smtk::view::BadgeAction;
  smtkTypeMacro(smtk::extension::paraview::appcomponents::HierarchicalVisibilityBadge);
  smtkSuperclassMacro(smtk::extension::qt::VisibilityBadge);
  smtkSharedFromThisMacro(smtk::view::Badge);
  smtkCreateMacro(smtk::view::Badge);

  HierarchicalVisibilityBadge();
  HierarchicalVisibilityBadge(
    smtk::view::BadgeSet& parent,
    const smtk::view::Configuration::Component&);

  ~HierarchicalVisibilityBadge() override;

  bool appliesToPhrase(const DescriptivePhrase* phrase) const override;
  std::string tooltip(const DescriptivePhrase*) const override;
  std::string icon(const DescriptivePhrase* phrase, const std::array<float, 4>&) const override;
  bool action(const DescriptivePhrase* phrase, const BadgeAction& act) override;

protected:
  smtk::view::PhraseModel* phraseModel() const { return this->m_parent->phraseModel(); }
  GeometricVisibilityBadge::VisibilityState phraseVisibility(const DescriptivePhrase* phrase) const;

  ///\brief Recursively set the geometric visibility of this Phrase and all of its descendants.
  void setPhraseVisibility(DescriptivePhrase* phrase, bool visible);

private:
  std::string m_iconVisible;
  std::string m_iconPartial;
  std::string m_iconBlanked;
  smtk::view::BadgeSet* m_parent{ nullptr };
  GeometricVisibilityBadge* m_partner{ nullptr };
};

} // namespace appcomponents
} // namespace paraview
} // namespace extension
} // namespace smtk
#endif
