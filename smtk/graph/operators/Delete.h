//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_Delete_h
#define smtk_graph_Delete_h

#include "smtk/operation/XMLOperation.h"

#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"
#include "smtk/graph/evaluators/OwnersOf.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"

#include <map>
#include <set>

namespace smtk
{
namespace graph
{

/**\brief Delete components from a resource.
  *
  * This operation is an abstract base class for operations.
  * You must subclass it for your particular graph-resource
  * and override operationInternal() and ableToOperate() in
  * order to pass helper methods your resource's Traits object.
  *
  * Given a set, S, of nodes to remove, this operation will examine
  * graph arcs connected to all members of S and, if any are marked
  * with OwnershipSemantics, either: (a) insert dependent nodes into
  * S so that they are also removed or (b) refuse to operate in order
  * to preserve semantic consistency. Which of these is done depends
  * on whether the "delete dependents" parameter is enabled or not.
  *
  * Assuming semantic consistency allows the operation to proceed,
  * 1. a DeletionCleanup query is initialized (if the resource provides
  *    one) for nodes in S that belong to the resource;
  * 2. all of the nodes in S (including additions above) are removed
  *    from their respective graph resources;
  * 3. properties of these nodes are removed;
  * 4. arcs connected to all nodes in S are removed;
  * 5. links to/from all nodes in S are invalidated;
  * 6. any DeletionCleanup queries from step 1 are finalized.
  */
class SMTKCORE_EXPORT Delete : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::graph::Delete);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  /// Set/get whether log messages should be suppressed.
  ///
  /// Ephemeral cells created by primitive selections use this
  /// to prevent confusing the user when the selection is
  /// deleted. You should not normally need to use this feature.
  void setSuppressOutput(bool suppress) { m_suppressOutput = suppress; }
  bool suppressOutput() const { return m_suppressOutput; }

protected:
  Delete();
  void generateSummary(smtk::operation::Operation::Result& result) override;
  const char* xmlDescription() const override;

  /// For each associated component, insert any owning nodes into \a deps.
  ///
  /// This method must be provided with a \a Traits type so it can iterate
  /// over edge
  template<typename ResourceType, typename Traits = typename ResourceType::TypeTraits>
  bool insertDependencies(std::set<Component*>& deps);

  Result m_result; // TODO: Let subclass define?
  bool m_suppressOutput{ false };
};

template<typename ResourceType, typename Traits>
bool Delete::insertDependencies(std::set<Component*>& deps)
{
  bool externalDeps = false;
  std::map<ResourceType*, std::set<Component*>> byResource;
  auto assoc = this->parameters()->associations();
  std::size_t nn = assoc->numberOfValues();
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    if (!assoc->isSet(ii))
    {
      continue;
    }
    auto* del = dynamic_cast<Component*>(assoc->value(ii).get());
    if (!del)
    {
      continue;
    }
    if (auto* rsrc = dynamic_cast<ResourceType*>(del->parentResource()))
    {
      byResource[rsrc].insert(del);
    }
  }
  for (const auto& entry : byResource)
  {
    // Traverse component's arcs; if an arc type is marked with ownership semantics,
    // each component in \a entry.second that matches the owned end causes the other
    // arc endpoint to be added to \a deps.
    std::set<Component*> queue;
    entry.first->template evaluateArcs<evaluators::OwnersOf>(entry.second, queue, externalDeps);
    // Now, anything added to \a queue must have *its* owners added, recursively
    // until there are no more to add. We can't expect OwnersOf to do this as
    // different arc-types may own the nodes in \a queue than the arc-types that
    // forced the addition of \a queue.
    std::set<Component*> swap;
    for (bool addedSomething = !queue.empty(); addedSomething; addedSomething = !queue.empty())
    {
      deps.insert(queue.begin(), queue.end());
      swap.insert(queue.begin(), queue.end());
      queue.clear();
      entry.first->template evaluateArcs<evaluators::OwnersOf>(swap, queue, addedSomething);
      // Remove things from queue that are in deps so we don't ping-pong.
      swap.clear();
      for (const auto& qq : queue)
      {
        if (deps.find(qq) != deps.end())
        {
          swap.insert(qq);
        }
      }
      queue.erase(swap.begin(), swap.end());
      swap.clear();
    }
  }
  return externalDeps;
}

} // namespace graph
} // namespace smtk

#endif
