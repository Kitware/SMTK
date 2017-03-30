//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_common_UnionFind_h
#define __smtk_common_UnionFind_h

#include <map>
#include <set>
#include <vector>

namespace smtk
{
namespace common
{

/// Internal storage for the UnionFind class.
template <typename T>
struct UnionFindSet
{
  UnionFindSet(T parent, T rank = 0)
  {
    this->m_parent = parent;
    this->m_rank = rank;
  }
  UnionFindSet(const UnionFindSet& other)
  {
    this->m_parent = other.m_parent;
    this->m_rank = other.m_rank;
  }
  T m_parent;
  T m_rank;
};

/**\brief A disjoint-set structure for fast union-find operations.
  *
  * This class is templated on the integer set-id type.
  * The template parameter should be a signed integer;
  * smaller sizes may improve performance when a smaller number
  * of unique sets is acceptable.
  *
  * A negative integer is used to identify an invalid value;
  * for instance, calling Find() on an integer not returned by
  * NewSet or MergeSets is an error and will return -1.
  */
template <typename T>
class UnionFind
{
public:
  /// Each value used to identify a set is of this type.
  typedef T value_type;

  /// Add a new set (disjoint from all others).
  T newSet();
  /// Connect two disjoint sets, returning the ID of their union (which may not be a or b).
  T mergeSets(T a, T b);
  /// Find the parent of a set.
  T find(T src);
  /// Get the current root sets (entries in Sets whose parents are themselves)
  std::set<T> roots();
  /// Popoulate a map with remaining disjoint sets numbered starting at \a startCount.
  void collapseIds(std::map<T, T>& collapsedIds, T startCount);
  /// Return the number of sets that have been created.
  T size() const { return static_cast<T>(this->m_sets.size()); }

  std::vector<UnionFindSet<T> > m_sets;
};

template <typename T>
T UnionFind<T>::newSet()
{
  T setId = this->size();
  UnionFindSet<T> entry(setId, 0);
  this->m_sets.push_back(entry);
  return setId;
}

template <typename T>
T UnionFind<T>::mergeSets(T a, T b)
{
  T aRoot = this->find(a);
  T bRoot = this->find(b);

  if (aRoot == bRoot)
  {
    return aRoot;
  }

  if (aRoot < 0 || bRoot < 0)
  {
    return -1;
  }

  T aRank = this->m_sets[aRoot].m_rank;
  T bRank = this->m_sets[bRoot].m_rank;
  if (aRank < bRank)
  {
    this->m_sets[aRoot].m_parent = bRoot;
    return bRoot;
  }
  else if (bRank == aRank)
  {
    ++this->m_sets[aRoot].m_rank;
  }
  this->m_sets[bRoot].m_parent = aRoot;
  return aRoot;
}

template <typename T>
T UnionFind<T>::find(T src)
{
  if (src < 0 || src >= this->size())
  {
    return -1;
  }
  T parent = this->m_sets[src].m_parent;
  if (parent != src)
  {
    this->m_sets[src].m_parent = this->find(parent);
  }
  return this->m_sets[src].m_parent;
}

template <typename T>
std::set<T> UnionFind<T>::roots()
{
  typename std::set<T> roots;
  typename std::vector<UnionFindSet<T> >::iterator it;
  T i = 0;
  for (it = this->m_sets.begin(); it != this->m_sets.end(); ++it, ++i)
  {
    if (i == it->m_parent)
    {
      roots.insert(i);
    }
  }
  return roots;
}

template <typename T>
void UnionFind<T>::collapseIds(std::map<T, T>& collapsedIds, T startCount)
{
  std::set<T> roots = this->roots();
  typename std::set<T>::iterator it;
  for (it = roots.begin(); it != roots.end(); ++it)
  {
    // Do not relabel any pre-existing entries in collapsedIds.
    typename std::map<T, T>::iterator cit = collapsedIds.find(*it);
    if (cit == collapsedIds.end())
    {
      //cout << "Collapse " << *it << " to " << startCount << "\n";
      collapsedIds[*it] = startCount++;
    }
  }
}

} // namespace common
} // namespace smtk

#endif // __smtk_common_UnionFind_h
