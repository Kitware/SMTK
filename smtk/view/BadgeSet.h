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

#include "smtk/common/TypeContainer.h"

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
  BadgeSet() = default;

  /// Construct and configure a set of badges for a view.
  BadgeSet(const Configuration* viewSpec, const ManagerPtr& manager, PhraseModel* phraseModel)
    : m_manager(manager)
    , m_phraseModel(phraseModel)
  {
    this->configure(viewSpec, manager);
  }

  BadgeSet(const BadgeSet&) = delete;
  void operator=(const BadgeSet&) = delete;

  void configure(const Configuration* viewSpec, const smtk::view::ManagerPtr& manager);

  using BadgeList = std::vector<Badge*>;
  /// Return ordered list of badge ptrs, ignoring any names without a matching badge.
  BadgeList badgesFor(const DescriptivePhrase* phrase) const;

  /// Return the manager (if any) used to create this badge-set.
  ///
  /// Some badges may need access to the manager to function.
  /// One example is the ObjectTypeAndColorBadge, which needs
  /// access to the manager's ObjectIcons to obtain SVG icons.
  smtk::view::ManagerPtr manager() const { return m_manager.lock(); }

  /// Return the phraseModel (if any) that owns this badge-set.
  PhraseModel* phraseModel() const { return m_phraseModel; }

  /// Get the first existing badge matching a type.
  template<typename T>
  T* findBadgeOfType();

  smtk::common::TypeContainer& badgeData() { return m_badgeData; }
  const smtk::common::TypeContainer& badgeData() const { return m_badgeData; }

private:
  std::weak_ptr<Manager> m_manager;
  PhraseModel* m_phraseModel{ nullptr };
  std::vector<std::unique_ptr<Badge>> m_badges;
  /// Place to store badge related information
  smtk::common::TypeContainer m_badgeData;
};

template<typename T>
T* BadgeSet::findBadgeOfType()
{
  for (auto& badge : m_badges)
  {
    T* result = dynamic_cast<T*>(badge.get());
    if (result)
      return result;
  }
  return nullptr;
}
} // namespace view
} // namespace smtk
#endif
