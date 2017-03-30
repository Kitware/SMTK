//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_vtk_union_find_h
#define __smtk_vtk_union_find_h

#include "vtkObjectBase.h" // for vtkIdType

#include <map>
#include <set>
#include <vector>

struct UnionFindSet
{
  UnionFindSet(vtkIdType parent, vtkIdType rank = 0)
  {
    this->Parent = parent;
    this->Rank = rank;
  }
  UnionFindSet(const UnionFindSet& other)
  {
    this->Parent = other.Parent;
    this->Rank = other.Rank;
  }
  vtkIdType Parent;
  vtkIdType Rank;
};

/// A disjoint-set structure for fast union-find operations.
class UnionFind
{
public:
  /// Add a new set (disjoint from all others).
  vtkIdType NewSet();
  /// Connect two disjoint sets, returning the ID of their union (which may not be a or b).
  vtkIdType MergeSets(vtkIdType a, vtkIdType b);
  /// Find the parent of a set.
  vtkIdType Find(vtkIdType src);
  /// Get the current root sets (entries in Sets whose parents are themselves)
  std::set<vtkIdType> Roots();
  /// Popoulate a map with remaining disjoint sets numbered starting at \a startCount.
  void CollapseIds(std::map<vtkIdType, vtkIdType>& collapsedIds, vtkIdType startCount);
  /// Return the number of sets that have been created.
  vtkIdType Size() const { return static_cast<vtkIdType>(this->Sets.size()); }

  std::vector<UnionFindSet> Sets;
};

inline vtkIdType UnionFind::NewSet()
{
  vtkIdType setId = static_cast<vtkIdType>(this->Sets.size());
  UnionFindSet entry(setId, 0);
  this->Sets.push_back(entry);
  return setId;
}

inline vtkIdType UnionFind::MergeSets(vtkIdType a, vtkIdType b)
{
  vtkIdType aRoot = this->Find(a);
  vtkIdType bRoot = this->Find(b);

  if (aRoot == bRoot)
  {
    return aRoot;
  }

  vtkIdType aRank = this->Sets[aRoot].Rank;
  vtkIdType bRank = this->Sets[bRoot].Rank;
  if (aRank < bRank)
  {
    this->Sets[aRoot].Parent = bRoot;
    return bRoot;
  }
  else if (bRank == aRank)
  {
    ++this->Sets[aRoot].Rank;
  }
  this->Sets[bRoot].Parent = aRoot;
  return aRoot;
}

inline vtkIdType UnionFind::Find(vtkIdType src)
{
  if (src < 0 || src > vtkIdType(this->Sets.size()))
  {
    return -1;
  }
  vtkIdType parent = this->Sets[src].Parent;
  if (parent != src)
  {
    this->Sets[src].Parent = this->Find(parent);
  }
  return this->Sets[src].Parent;
}

inline std::set<vtkIdType> UnionFind::Roots()
{
  std::set<vtkIdType> roots;
  std::vector<UnionFindSet>::iterator it;
  vtkIdType i = 0;
  for (it = this->Sets.begin(); it != this->Sets.end(); ++it, ++i)
  {
    if (i == it->Parent)
    {
      roots.insert(i);
    }
  }
  return roots;
}

inline void UnionFind::CollapseIds(
  std::map<vtkIdType, vtkIdType>& collapsedIds, vtkIdType startCount)
{
  std::set<vtkIdType> roots = this->Roots();
  std::set<vtkIdType>::iterator it;
  for (it = roots.begin(); it != roots.end(); ++it)
  {
    // Do not relabel any pre-existing entries in collapsedIds.
    std::map<vtkIdType, vtkIdType>::iterator cit = collapsedIds.find(*it);
    if (cit == collapsedIds.end())
    {
      //cout << "Collapse " << *it << " to " << startCount << "\n";
      collapsedIds[*it] = startCount++;
    }
  }
}

#endif // __union_find_h
