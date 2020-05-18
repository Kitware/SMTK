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
    for (const auto& rule : m_data)
    {
      if (!(*rule)(object))
      {
        return false;
      }
    }
    return true;
  }

  template<typename... Args>
  void emplace_back(Args&&... args)
  {
    return m_data.emplace_back(std::forward<Args>(args)...);
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
