//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/SequentialAssignedIds.h"

#include "smtk/markup/IdSpace.h"

#include "smtk/string/Token.h"

namespace smtk
{
namespace markup
{

SequentialAssignedIds::IdRange SequentialAssignedIds::range() const
{
  return m_range;
}

IdType SequentialAssignedIds::size() const
{
  return m_range[1] - m_range[0];
}

IdType SequentialAssignedIds::maxId() const
{
  IdType result = this->size();
  --result;
  return result;
}

bool SequentialAssignedIds::empty() const
{
  return m_range[1] <= m_range[0];
}

SequentialAssignedIds::Iterator<SequentialAssignedIds::Forward> SequentialAssignedIds::begin() const
{
  Iterator<Forward> result(
    m_range[0] < m_range[1] ? m_range[0] : Iterator<Forward>::Invalid, m_range);
  return result;
}

SequentialAssignedIds::Iterator<SequentialAssignedIds::Forward> SequentialAssignedIds::end() const
{
  Iterator<Forward> result(Iterator<Forward>::Invalid, m_range);
  return result;
}

SequentialAssignedIds::Iterator<SequentialAssignedIds::Reverse> SequentialAssignedIds::rbegin()
  const
{
  Iterator<Reverse> result(
    m_range[0] < m_range[1] ? m_range[1] - 1 : Iterator<Reverse>::Invalid, m_range);
  return result;
}

SequentialAssignedIds::Iterator<SequentialAssignedIds::Reverse> SequentialAssignedIds::rend() const
{
  Iterator<Reverse> result(Iterator<Reverse>::Invalid, m_range);
  return result;
}

IdType SequentialAssignedIds::contains(IdType begin, IdType end) const
{
  if (begin <= m_range[0] && end >= m_range[1])
  {
    // Covers the entire interval.
    return this->size();
  }
  else if (begin >= m_range[1] || end < m_range[0] || begin >= end)
  {
    return 0;
  }
  IdType aa = begin > m_range[0] ? begin : m_range[0];
  IdType bb = end < m_range[1] ? end : m_range[1] - 1;
  return bb - aa;
}

} // namespace markup
} // namespace smtk
