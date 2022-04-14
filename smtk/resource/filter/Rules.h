//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_Rules_h
#define smtk_resource_filter_Rules_h

#include "smtk/resource/PersistentObject.h"

#include "smtk/resource/filter/Rule.h"

#include <algorithm>

namespace smtk
{
namespace resource
{
namespace filter
{

class SMTKCORE_EXPORT Rules
{
public:
  typedef std::vector<std::unique_ptr<smtk::resource::filter::Rule>> Container;

  Rules() = default;

  Rules(const Rules&) = delete;
  Rules(Rules&&) = default;

  Rules& operator=(const Rules&) = delete;
  Rules& operator=(Rules&&) = default;

  bool operator()(const PersistentObject& object) const
  {
    return std::all_of(m_data.begin(), m_data.end(), [&object](const std::unique_ptr<Rule>& rule) {
      return (*rule)(object);
    });
  }

  template<typename... Args>
  void emplace_back(Args&&... args)
  {
    m_data.emplace_back(std::forward<Args>(args)...);
  }

  Container::reference back() { return m_data.back(); }
  Container::const_reference back() const { return m_data.back(); }

  Container& data() { return m_data; }
  const Container& data() const { return m_data; }

private:
  Container m_data;
};
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
