//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_BadgeFactory_h
#define smtk_view_BadgeFactory_h

#include "smtk/view/BadgeSet.h"
#include "smtk/view/Configuration.h"

#include "smtk/common/TypeName.h"

#include <functional>
#include <map>
#include <string>

namespace smtk
{
namespace view
{
/**\brief A factory for badges that appear in descriptive phrases.
  *
  * A badge factory creates badges for use in PhraseModel views. A "badge"
  * in this context is an object that takes an input descriptive phrase and
  * returns an SVG icon with an optional tooltip and action to
  * be performed when clicked by a user. A badge may indicate it is not
  * applicable to a descriptive phrase.
  *
  * Badges are owned by a BadgeSet, which is in turn owned by a PhraseModel,
  * Thus, there is a single Badge instance per PhraseModel rather than one
  * per DescriptivePhrase.
  * To register a Badge subclass, it must have a default constructor
  * and a constructor that takes references to a BadgeSet and a
  * Configuration::Component.
  */
class SMTKCORE_EXPORT BadgeFactory
{
public:
  using BadgeConstructor = std::function<Badge::Ptr(BadgeSet&, const Configuration::Component&)>;

  /// Register a badge type.
  template <typename BadgeType>
  bool registerBadge()
  {
    BadgeType temporaryBadge;
    std::string name = temporaryBadge.typeName();

    return this->registerBadge(
      name, [](BadgeSet& parent, const smtk::view::Configuration::Component& config) {
        auto badge = std::shared_ptr<Badge>(new BadgeType(parent, config));
        return badge;
      });
  }

  bool registerBadge(const std::string& typeName, const BadgeConstructor& ctor)
  {
    if (typeName.empty() || !ctor)
    {
      return false;
    }

    m_badges[typeName] = ctor;
    return true;
  }

  template <typename BadgeType>
  bool unregisterBadge()
  {
    BadgeType temporaryBadge;
    std::string name = temporaryBadge.typeName();
    if (name.empty())
    {
      return false;
    }

    return m_badges.erase(name) > 0;
  }

  /// represents.
  bool unregisterBadge(const std::string& typeName) { return m_badges.erase(typeName) > 0; }

  Badge::Ptr create(const std::string& typeName, smtk::view::BadgeSet& parent,
    const smtk::view::Configuration::Component& config) const
  {
    auto it = m_badges.find(typeName);
    if (it == m_badges.end())
    {
      return Badge::Ptr();
    }
    auto badge = it->second(parent, config);
    return badge;
  }

private:
  /// A container for all registered badge constructors.
  std::map<std::string, BadgeConstructor> m_badges;
};
}
}

#endif
