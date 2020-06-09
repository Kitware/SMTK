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

namespace smtk
{
namespace extension
{
namespace qt
{

/**\brief A badge that lets the user choose from a set of objects.
  *
  */
class SMTKQTEXT_EXPORT MembershipBadge : public QObject, public smtk::view::Badge
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
  virtual ~MembershipBadge();

  bool appliesToPhrase(const DescriptivePhrase*) const override { return true; }

  std::string icon(const DescriptivePhrase* phrase, const std::array<float, 4>&) const override;

  bool action(const smtk::view::DescriptivePhrase*, const smtk::view::BadgeAction&) override;

  using MemberMap = std::map<std::weak_ptr<smtk::resource::PersistentObject>, int,
    std::owner_less<std::weak_ptr<smtk::resource::PersistentObject> > >;

  /// Provide external access to which items are selected.
  MemberMap& getMemberMap() { return m_members; };

  /// Returns true if this badge is set to only allow a single member at a time.
  bool singleSelect() const { return m_singleSelect; }

signals:
  void membershipChange(int val);

protected:
  MemberMap m_members;   //!< From available items, has this object been turned on?
  bool m_singleSelect;   //!< If true, only 1 item may be a member; toggling an item resets others.
  std::string m_iconOn;  //!< SVG for icon showing membership.
  std::string m_iconOff; //!< SVG for icon showing non-membership.
  const smtk::view::BadgeSet* m_parent;
};
}
}
}

#endif
