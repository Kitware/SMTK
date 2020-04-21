//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_Badge_h
#define smtk_view_Badge_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/SharedFromThis.h"

#include <array>

namespace smtk
{
namespace view
{
class SMTKCORE_EXPORT Badge : smtkEnableSharedPtr(Badge)
{
public:
  smtkTypeMacroBase(Badge);

  // Badge(
  //   const std::string name,
  //   const std::array<float, 4>& background,
  //   const std::string& icon);

  virtual ~Badge() = 0;

  std::string name() const { return m_name; }
  std::size_t hash() const { return std::hash<std::string>{}(m_name); }
  virtual std::string svg(DescriptivePhrasePtr phrase, bool lightBackground = true) const
  {
    return m_icon;
  }
  std::array<float, 4> background() const { return m_background; }

  /// take an action when the badge is clicked.
  virtual bool onClick(DescriptivePhrasePtr phrase) { return false; }

private:
  std::string m_name;
  std::array<float, 4> m_background;
  std::string m_icon;
};

class SMTKCORE_EXPORT BadgeSet
{
public:
  BadgeSet(const smtk::view::ConfigurationPtr& viewSpec, const smtk::view::ManagerPtr& manager);

  bool addBadge(const std::string& name, Badge::Ptr badge)
  {
    m_badges[name] = badge;
    return true;
  }
  void setOrder(const std::vector<std::string>& names) { m_order = names; }
  /// return ordered list of badge ptrs, ignoring any names without a matching badge.
  std::vector<Badge::Ptr> getBadges() const
  {
    std::vector<Badge::Ptr> ret;
    for (auto name : m_order)
    {
      auto iter = m_badges.find(name);
      if (iter != m_badges.end())
      {
        ret.push_back(iter->second);
      }
    }
    return ret;
  }

private:
  std::map<std::string, Badge::Ptr> m_badges;
  std::vector<std::string> m_order;
};
}
}
#endif
