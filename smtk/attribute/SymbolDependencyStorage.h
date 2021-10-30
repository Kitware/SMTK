//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_SymbolDependencyStorage_h
#define smtk_attribute_SymbolDependencyStorage_h

#include "smtk/CoreExports.h"

#include "smtk/resource/query/Cache.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace smtk
{
namespace attribute
{

// SymbolDependencyStorage maintains a tree representing which symbols depend on
// other symbols. It may hold trees that are disjoint, meaning a set of symbols
// have mutually exclusive dependencies.
struct SMTKCORE_EXPORT SymbolDependencyStorage : public smtk::resource::query::Cache
{
public:
  // Returns all symbols dependent on |symbol| in level-order traversal order.
  std::vector<std::string> allDependentSymbols(const std::string& symbol) const;

  // Adds a dependency from |from| to |to|. That is, makes |from| dependent on
  // |to|. Fails and returns false if adding such a dependency would cause a
  // cyclic dependency.
  bool addDependency(const std::string& from, const std::string& to);

  // Given |newSymbols| used by |dependentSymbol|, removes linkage between
  // |dependentSymbol| and each symbol in the storage not in |newSymbols|.
  // I.e. each symbol erased means it is no longer needed by |dependentSymbol|.
  void pruneOldSymbols(
    const std::unordered_set<std::string>& newSymbols,
    const std::string& dependentSymbol);

  // Returns whether start is dependent on end by checking for a path between start
  // and end using breadth-first search.
  bool isDependentOn(const std::string& start, const std::string& end) const;

#ifndef NDEBUG
  void dump();
#endif

private:
  // Edges in the tree.
  typedef std::unordered_set<std::string> Entry;

  // The hash table of nodes to their direct dependents.
  std::unordered_map<std::string, Entry> m_cache;
};

} // namespace attribute
} // namespace smtk

#endif // smtk_attribute_SymbolDependencyStorage_h
