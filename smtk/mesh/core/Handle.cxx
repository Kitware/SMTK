//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/Handle.h"

namespace smtk
{
namespace mesh
{

const_element_iterator rangeElementsBegin(const HandleRange& range)
{
  return boost::icl::elements_begin(range);
}

const_element_iterator rangeElementsEnd(const HandleRange& range)
{
  return boost::icl::elements_end(range);
}

Handle rangeElement(const HandleRange& range, std::size_t i)
{
  return *std::next(boost::icl::elements_begin(range), i);
}

bool rangeContains(const HandleRange& range, Handle i)
{
  return boost::icl::contains(range, i);
}

bool rangeContains(const HandleRange& range, const HandleInterval& i)
{
  return boost::icl::contains(range, i);
}

bool rangeContains(const HandleRange& super, const HandleRange& sub)
{
  return boost::icl::contains(super, sub);
}

std::size_t rangeIndex(const HandleRange& range, Handle i)
{
  std::size_t index = 0;

  HandleRange::const_iterator interval = range.find(i);
  for (HandleRange::const_iterator it = range.begin(); it != interval; ++it)
  {
    index += (it->upper() - it->lower()) + 1;
  }

  index += i - interval->lower();

  return index;
}

std::size_t rangeIntervalCount(const HandleRange& range)
{
  return boost::icl::interval_count(range);
}

bool rangesEqual(const HandleRange& lhs, const HandleRange& rhs)
{
  if (smtk::mesh::rangeIntervalCount(lhs) != smtk::mesh::rangeIntervalCount(rhs))
  {
    return false;
  }

  auto i = lhs.begin();
  auto j = rhs.begin();
  for (; i != lhs.end(); ++i, ++j)
  {
    if (i->lower() != j->lower() || i->upper() != j->upper())
    {
      return false;
    }
  }
  return true;
}
} // namespace mesh
} // namespace smtk

std::ostream& operator<<(std::ostream& os, const smtk::mesh::HandleRange& range)
{
  os << "[";
  for (auto interval : range)
  {
    os << interval << ",";
  }
  if (!range.empty())
  {
    os << '\b';
  }
  os << "]";
  return os;
}
