//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_common_RangeDetector_h
#define __smtk_common_RangeDetector_h

#include <iostream>
#include <map>

namespace smtk
{
namespace common
{

template <typename I>
class RangeDetector
{
public:
  /// Insert a new entry, which may add to an existing range or start a new one.
  void insert(I val)
  {
    typename std::map<I, I>::iterator it = m_collapse.find(val);
    if (it != m_collapse.end())
    { // val is not at
      I collapseTarget = it->second;
      m_ranges[collapseTarget]++;
      m_collapse.erase(it->first);
      typename std::map<I, I>::iterator rit = m_ranges.find(val + 1);
      if (rit != m_ranges.end())
      {
        m_ranges[collapseTarget] = rit->second;
        // The entry in m_collapse will now serve match it->second's range entry:
        m_collapse[rit->second + 1] = collapseTarget;
        m_ranges.erase(rit);
      }
      else
      {
        m_collapse[val + 1] = collapseTarget;
      }
    }
    else
    {
      it = m_ranges.find(val + 1);
      if (it != m_ranges.end())
      { // Expand the range we found to the left:
        m_ranges[val] = it->second;
        m_collapse[it->second + 1] = val;
        m_ranges.erase(it);
      }
      else
      { // Add a new range and collapse entry:
        m_ranges[val] = val;
        m_collapse[val + 1] = val;
      }
    }
  }

  /// Return the current set of intervals.
  std::map<I, I>& ranges() const { return m_ranges; }
  std::map<I, I>& ranges() { return m_ranges; }

  /// Return the number of entries (not the number of ranges) as the size
  size_t size() const
  {
    size_t nn = 0;
    typename std::map<I, I>::const_iterator it;
    for (it = m_ranges.begin(); it != m_ranges.end(); ++it)
    {
      nn += it->second - it->first + 1;
    }
    return nn;
  }

  /// Empty the list of detected ranges and start over.
  void clear()
  {
    m_ranges.clear();
    m_collapse.clear();
  }

  /// Dump the ranges
  void dump()
  {
    std::cout << "Ranges:\n";
    typename std::map<I, I>::iterator it;
    for (it = m_ranges.begin(); it != m_ranges.end(); ++it)
    {
      std::cout << "  " << it->first << " .. " << it->second << "\n";
    }
    std::cout << "Collapsers:\n";
    for (it = m_collapse.begin(); it != m_collapse.end(); ++it)
    {
      std::cout << "  " << it->first << " .. " << it->second << "\n";
    }
    std::cout << "\n";
  }

protected:
  std::map<I, I> m_collapse;
  std::map<I, I> m_ranges;
};

} // namespace common
} // namespace smtk

#endif // __smtk_common_RangeDetector_h
