//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_MembershipBadge_h
#define smtk_extension_qt_MembershipBadge_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/qt/Exports.h"

#include "smtk/view/Badge.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/DescriptivePhrase.h"

#include <QObject>

#include <algorithm>
#include <array>
#include <cctype>
#include <iostream>
#include <string>

namespace smtk
{
namespace extension
{
namespace qt
{

/**\brief Rules for determining which phrases a membership badge applies to.
 */
enum class MembershipCriteria
{
  All,                    //!< All descriptive phrases should have a membership badge.
  Components,             //!< All phrases with a related component.
  ComponentsWithGeometry, //!< Like Components, but restricted to components with renderable geometry.
  Objects, //!< All descriptive phrases with a related persistent object should have a badge.
  ObjectsWithGeometry,  //!< Like Objects, but restricted to objects with renderable geometry.
  Resources,            //!< All phrases with a related resource but not a related component.
  ResourcesWithGeometry //!< Like Resources, but restricted to resources with renderable geometry.
};

/// A type-conversion operation to cast enumerants to strings.
inline std::string membershipCriteriaName(const MembershipCriteria& mc)
{
  static std::array<std::string, 7> names{ { "all",
                                             "components",
                                             "componentswithgeometry",
                                             "objects",
                                             "objectswithgeometry",
                                             "resources",
                                             "resourceswithgeometry" } };
  return names[static_cast<int>(mc)];
}

/// A type-conversion operation to cast strings to enumerants.
inline MembershipCriteria membershipCriteriaEnum(const std::string& mcn)
{
  std::string criteriaName(mcn);
  std::transform(
    criteriaName.begin(), criteriaName.end(), criteriaName.begin(), [](unsigned char c) {
      return std::tolower(c);
    });
  if (criteriaName.substr(0, 20) == "membershipcriteria::")
  {
    criteriaName = criteriaName.substr(20);
  }
  if (criteriaName == "components")
  {
    return MembershipCriteria::Components;
  }
  else if (criteriaName == "componentswithgeometry")
  {
    return MembershipCriteria::ComponentsWithGeometry;
  }
  else if (criteriaName == "objects")
  {
    return MembershipCriteria::Objects;
  }
  else if (criteriaName == "objectswithgeometry")
  {
    return MembershipCriteria::ObjectsWithGeometry;
  }
  else if (criteriaName == "resources")
  {
    return MembershipCriteria::Resources;
  }
  else if (criteriaName == "resourceswithgeometry")
  {
    return MembershipCriteria::ResourcesWithGeometry;
  }
  return MembershipCriteria::All;
}

/**\brief A badge that lets the user choose from a set of objects.
  *
  */
class SMTKQTEXT_EXPORT MembershipBadge
  : public QObject
  , public smtk::view::Badge
{
  Q_OBJECT
public:
  smtkTypeMacro(smtk::extension::qt::MembershipBadge);
  smtkSuperclassMacro(smtk::view::Badge);
  smtkSharedFromThisMacro(smtk::view::Badge);
  smtkCreateMacro(smtk::view::Badge);
  using DescriptivePhrase = smtk::view::DescriptivePhrase;

  MembershipBadge();
  MembershipBadge(smtk::view::BadgeSet&, const smtk::view::Configuration::Component&);
  ~MembershipBadge() override;

  bool appliesToPhrase(const DescriptivePhrase*) const override;

  std::string icon(const DescriptivePhrase* phrase, const std::array<float, 4>&) const override;

  bool action(const smtk::view::DescriptivePhrase*, const smtk::view::BadgeAction&) override;

  using MemberMap = std::map<
    std::weak_ptr<smtk::resource::PersistentObject>,
    int,
    std::owner_less<std::weak_ptr<smtk::resource::PersistentObject>>>;

  /// Provide external access to which items are selected.
  MemberMap& getMemberMap() { return m_members; };

  /// Returns true if this badge is set to only allow a single member at a time.
  bool singleSelect() const { return m_singleSelect; }

  MembershipCriteria membershipCriteria() const { return m_criteria; }

Q_SIGNALS:
  void membershipChange(int val);

protected:
  MemberMap m_members; //!< From available items, has this object been turned on?
  bool m_singleSelect{
    false
  };                     //!< If true, only 1 item may be a member; toggling an item resets others.
  std::string m_iconOn;  //!< SVG for icon showing membership.
  std::string m_iconOff; //!< SVG for icon showing non-membership.
  const smtk::view::BadgeSet* m_parent{ nullptr };
  MembershipCriteria m_criteria{ MembershipCriteria::All };
  std::string m_filter;
};
} // namespace qt
} // namespace extension
} // namespace smtk

#endif
