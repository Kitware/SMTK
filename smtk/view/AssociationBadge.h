//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_AssociationBadge_h
#define smtk_view_AssociationBadge_h

#include "smtk/view/Badge.h"

namespace smtk
{
namespace view
{
class BadgeSet;
class Manager;

/**\brief A badge that appears when an object is missing a mandatory association.
  *
  * This badge must be configured with a set of required definitions.
  * To do this, the badge's `Configuration::Component` must have
  * one or more children of type `Requires` whose `Definition` attribute
  * is the name of a definition.
  *
  * The badge may also discriminate on the type of object to which associations
  * are required.
  * To do this, the badge's `Configuration::Component` must have
  * exactly one child of type `AppliesTo` that has an attribute
  * `Resource` that specifies the type-name of a resource.
  * If the badge should appear next to resources of this type, that is all
  * that is required. However, if the badge should instead appear next to
  * components owned by resources of that type, the `AppliesTo` component must
  * also have a `Component` attribute that specifies matching components with
  * a query string.
  * If an `AppliesTo` component is not provided, then all persistent objects
  * will be checked for the required associations.
  *
  * The badge will appear next to matching objects when:
  * + the required definitions are not present in at least one attribute resource,
  * + the object's associated attributes do not include at least one whose definition
  *   is a required definition type (for each required definition type).
  *
  * When the badge appears, its tooltip will include the list of definitions
  * that are not satisfied.
  */
class SMTKCORE_EXPORT AssociationBadge : public Badge
{
public:
  smtkTypeMacro(smtk::view::AssociationBadge);
  AssociationBadge();
  AssociationBadge(BadgeSet&, const Configuration::Component&);
  virtual ~AssociationBadge();

  /// This badge only applies to phrases with a persistent object that has an icon.
  bool appliesToPhrase(const DescriptivePhrase* phrase) const override;

  /// The tool-tip string is the object's type.
  std::string tooltip(const DescriptivePhrase* phrase) const override;

  /// Returns an SVG string for rendering the badge icon.
  ///
  /// The badge is an exclamation mark subtracted from a red circle.
  std::string icon(
    const DescriptivePhrase* phrase, const std::array<float, 4>& background) const override;

  // No action is taken when the badge is clicked.
  //
  // Subclass this badge if you wish to implement an action.
  // void action(const DescriptivePhrase* phrase) const override { }

protected:
  bool appliesToObject(const smtk::resource::PersistentObjectPtr& obj) const;
  std::set<std::string> unmetRequirements(const smtk::resource::PersistentObjectPtr& obj) const;

  const BadgeSet* m_parent;
  std::string m_applyToResource;
  std::string m_applyToComponent;
  std::set<std::string> m_requiredDefinitions;
};
}
}
#endif
