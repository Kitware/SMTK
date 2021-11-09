//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Item.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_Tag_h
#define smtk_attribute_Tag_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include <algorithm>
#include <set>
#include <string>

namespace smtk
{
namespace attribute
{

class SMTKCORE_EXPORT Tag
{
public:
  Tag(const std::string& name)
    : m_name(name)
  {
  }

  Tag(const std::string& name, const std::set<std::string>& values)
    : m_name(name)
    , m_values(values)
  {
  }

  Tag(const std::string& name, std::set<std::string>&& values)
    : m_name(name)
    , m_values(values)
  {
  }

  const std::string& name() const { return m_name; }
  const std::set<std::string>& values() const { return m_values; }
  std::set<std::string>& values() { return m_values; }

  bool add(const std::string& value) { return m_values.insert(value).second; }

  bool remove(const std::string& value) { return m_values.erase(value) > 0; }

  bool contains(const std::string& value) const { return m_values.find(value) != m_values.end(); }

  bool operator<(const Tag& rhs) const { return m_name < rhs.m_name; }

protected:
  std::string m_name;
  std::set<std::string> m_values;
};

typedef std::set<Tag> Tags;
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_Tag_h */
