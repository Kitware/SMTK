//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_BadgeSet_h
#define smtk_view_BadgeSet_h

#include "smtk/view/Badge.h"
#include "smtk/view/Configuration.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include <array>

namespace smtk
{
namespace view
{

/**\brief A container for the badges that apply to a view.
  *
  */
class SMTKCORE_EXPORT BadgeSet
{
public:
  /// Remove this once view::Manager uses the new factory method to construct PhraseModel with arguments.
  BadgeSet() {}

  /// Construct and configure a set of badges for a view.
  BadgeSet(const Configuration* viewSpec, const ManagerPtr& manager)
    : m_manager(manager)
  {
    this->configure(viewSpec, manager);
  }

  BadgeSet(const BadgeSet&) = delete;
  void operator=(const BadgeSet&) = delete;

  void configure(const Configuration* viewSpec, const smtk::view::ManagerPtr& manager);

  /// Return ordered list of badge ptrs, ignoring any names without a matching badge.
  std::vector<const Badge*> badgesFor(const DescriptivePhrase* phrase) const;

  /// Return the manager (if any) used to create this badge-set.
  ///
  /// Some badges may need access to the manager to function.
  /// One example is the ObjectTypeAndColorBadge, which needs
  /// access to the manager's IconFactory to obtain SVG icons.
  smtk::view::ManagerPtr manager() const { return m_manager.lock(); }

private:
  std::weak_ptr<Manager> m_manager;
  std::vector<std::unique_ptr<Badge> > m_badges;
};
}
}
#endif
