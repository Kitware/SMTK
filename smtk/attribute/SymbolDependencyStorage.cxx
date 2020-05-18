//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/SymbolDependencyStorage.h"

#include <algorithm>
#include <iostream>
#include <queue>

namespace smtk
{
namespace attribute
{

std::vector<std::string> SymbolDependencyStorage::allDependentSymbols(
  const std::string& symbol) const
{
  std::vector<std::string> result;

  if (!m_cache.count(symbol))
    return result;

  std::queue<std::string> q;
  q.push(symbol);

  while (!q.empty())
  {
    std::string current = q.front();
    q.pop();

    // As of now, this data structure does not consider |symbol| to be dependent on itself
    // in the return value of this method. The caller must check that separately.
    if (current != symbol)
      result.push_back(current);

    // Sorts dependents of |current| lexicographically so that ordering among
    // a level is guaranteed.
    auto it = m_cache.find(current);
    if (it == m_cache.end())
      continue;

    std::vector<std::string> currentLevelDeps(it->second.begin(), it->second.end());
    //std::vector<std::string> currentLevelDeps(m_cache[current].begin(), m_cache[current].end());
    std::sort(currentLevelDeps.begin(), currentLevelDeps.end());
    for (const auto& dep : currentLevelDeps)
    {
      q.push(dep);
    }
  }

  return result;
}

bool SymbolDependencyStorage::addDependency(const std::string& from, const std::string& to)
{
  // Is |to| dependent on |from|? If so, adding this dependency would cause a cycle.
  if (isDependentOn(to, from))
    return false;

  const auto fromIt = m_cache.find(from);
  if (fromIt == m_cache.end())
    m_cache.insert(std::make_pair(from, Entry()));

  m_cache[from].insert(to);

  const auto toIt = m_cache.find(to);
  if (toIt == m_cache.end())
    m_cache.insert(std::make_pair(to, Entry()));

  return true;
}

void SymbolDependencyStorage::pruneOldSymbols(
  const std::unordered_set<std::string>& newSymbols,
  const std::string& dependentSymbol)
{
  // A nice and easy O(n) time operation.
  for (const auto& pair : m_cache)
  {
    if (!newSymbols.count(pair.first) && pair.second.count(dependentSymbol))
    {
      m_cache[pair.first].erase(dependentSymbol);
    }
  }
}

bool SymbolDependencyStorage::isDependentOn(const std::string& start, const std::string& end) const
{
  if (start == end)
    return true;

  if (!m_cache.count(start) || !m_cache.count(end))
    return false;

  std::queue<std::string> q;
  q.push(start);

  while (!q.empty())
  {
    std::string current = q.front();
    q.pop();

    if (current == end)
      return true;

    auto currentIt = m_cache.find(current);
    if (currentIt == m_cache.end())
      continue;

    for (const auto& dep : currentIt->second)
    {
      q.push(dep);
    }
  }

  return false;
}

#ifndef NDEBUG
void SymbolDependencyStorage::dump()
{
  for (const auto& p : m_cache)
  {
    std::cout << p.first << " : { ";
    for (const auto& dependent : p.second)
    {
      std::cout << dependent << ", ";
    }
    std::cout << "}" << std::endl;
  }
}
#endif

} // namespace attribute
} // namespace smtk
