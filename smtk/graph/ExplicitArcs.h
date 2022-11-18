//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_ExplicitArcs_h
#define smtk_graph_ExplicitArcs_h

#include "smtk/common/UUID.h"
#include "smtk/common/Visit.h"
#include "smtk/graph/ArcProperties.h"
#include "smtk/resource/Component.h"
#include "smtk/string/Token.h"

#include <unordered_map>
#include <unordered_set>

namespace smtk
{
namespace graph
{

/**\brief A wrapper around arc type-traits classes that provides explicit storage of arcs.
  *
  */
template<typename ArcTraits>
class ExplicitArcs
{
public:
  using Traits = ArcTraits; // Allow classes to inspect our input parameter.

  using FromType = typename ArcTraits::FromType;
  using ToType = typename ArcTraits::ToType;
  using Directed = typename ArcTraits::Directed;
  using Ordered = std::false_type; // This class cannot represent ordered arcs.
  using Mutable = typename ArcProperties<ArcTraits>::isMutable;
  using BidirIndex =
    negation<typename ArcProperties<ArcTraits>::template hasOnlyForwardIndex<ArcTraits>>;

  using UUID = smtk::common::UUID;

  static constexpr std::size_t MaxOutDegree = maxOutDegree<ArcTraits>(unconstrained());
  static constexpr std::size_t MaxInDegree = maxInDegree<ArcTraits>(unconstrained());
  static constexpr std::size_t MinOutDegree = maxOutDegree<ArcTraits>(unconstrained());
  static constexpr std::size_t MinInDegree = maxInDegree<ArcTraits>(unconstrained());

  using NoBadIndexing = disjunction<Directed, conjunction<negation<Directed>, BidirIndex>>;
  static_assert(
    NoBadIndexing::value,
    "Undirected arcs must be bidirectionally indexed (otherwise outVisitor cannot work).");

  /**\brief Visit every node which has outgoing arcs of this type.
    */
  template<typename Resource, typename Functor>
  smtk::common::Visited visitAllOutgoingNodes(Resource rr, Functor ff) const
  {
    (void)rr;
    bool didVisit = false;
    smtk::common::VisitorFunctor<Functor> visitor(ff);
    std::set<const FromType*> visitedNodes; // Only used for auto-undirected visits
    for (const auto& entry : m_forward)
    {
      if (!entry.second.empty())
      {
        didVisit = true;
        if (visitor(entry.first) == smtk::common::Visit::Halt)
        {
          return smtk::common::Visited::Some;
        }
        if (ArcProperties<ArcTraits>::isAutoUndirected::value)
        {
          visitedNodes.insert(entry.first);
        }
      }
    }
    // If arc is auto-undirected, we must also check for
    // nodes in reverse direction that we haven't already seen.
    // Alternatively, we could have visited every node in
    // entry.second in the loop above...
    if (ArcProperties<ArcTraits>::isAutoUndirected::value)
    {
      for (const auto& entry : m_reverse)
      {
        const auto* node = reinterpret_cast<const FromType*>(entry.first);
        if (!entry.second.empty() && visitedNodes.find(node) == visitedNodes.end())
        {
          if (visitor(node) == smtk::common::Visit::Halt)
          {
            return smtk::common::Visited::Some;
          }
        }
      }
    }
    return didVisit ? smtk::common::Visited::All : smtk::common::Visited::Empty;
  }

  /**\brief Visit every node which has incoming arcs of this type.
    */
  template<typename Resource, typename Functor>
  smtk::common::Visited visitAllIncomingNodes(Resource rr, Functor ff) const
  {
    (void)rr;
    bool didVisit = false;
    smtk::common::VisitorFunctor<Functor> visitor(ff);
    std::set<const ToType*> visitedNodes; // Only used for auto-undirected visits
    for (const auto& entry : m_reverse)
    {
      if (!entry.second.empty())
      {
        didVisit = true;
        if (visitor(entry.first) == smtk::common::Visit::Halt)
        {
          return smtk::common::Visited::Some;
        }
        if (ArcProperties<ArcTraits>::isAutoUndirected::value)
        {
          visitedNodes.insert(entry.first);
        }
      }
    }
    // If arc is auto-undirected, we must also check for
    // nodes in reverse direction that we haven't already seen.
    // Alternatively, we could have visited every node in
    // entry.second in the loop above...
    if (ArcProperties<ArcTraits>::isAutoUndirected::value)
    {
      for (const auto& entry : m_forward)
      {
        const auto* node = reinterpret_cast<const ToType*>(entry.first);
        if (!entry.second.empty() && visitedNodes.find(node) == visitedNodes.end())
        {
          if (visitor(node) == smtk::common::Visit::Halt)
          {
            return smtk::common::Visited::Some;
          }
        }
      }
    }
    return didVisit ? smtk::common::Visited::All : smtk::common::Visited::Empty;
  }

  /**\brief Visit outgoing arcs from a \a node.
    */
  template<typename Functor>
  smtk::common::Visited outVisitor(const FromType* node, Functor ff) const
  {
    if (!node)
    {
      throw std::invalid_argument("Null from node.");
    }
    auto resource = node->resource();
    if (!resource)
    {
      throw std::invalid_argument("Input node has no parent resource.");
    }

    smtk::common::VisitorFunctor<Functor> visitor(ff);
    bool didVisit = false;

    // Find matching forward arcs.
    auto it = m_forward.find(node);
    if (it != m_forward.end())
    {
      didVisit |= !it->second.empty();
      for (const auto* other : it->second)
      {
        if (other)
        {
          if (visitor(other) == smtk::common::Visit::Halt)
          {
            return smtk::common::Visited::Some;
          }
        }
      }
    }

    // If the graph is bidirectional and types match, find matching reverse arcs.
    if (std::is_same<FromType, ToType>::value && !Directed::value && BidirIndex::value)
    {
      auto rit = m_reverse.find(reinterpret_cast<const ToType*>(node));
      if (rit != m_reverse.end())
      {
        didVisit = true;
        for (const auto* other : rit->second)
        {
          if (other)
          {
            if (visitor(reinterpret_cast<const ToType*>(other)) == smtk::common::Visit::Halt)
            {
              return smtk::common::Visited::Some;
            }
          }
          else
          {
            throw std::runtime_error("Null node.");
          }
        }
      }
    }

    return didVisit ? smtk::common::Visited::All : smtk::common::Visited::Empty;
  };

  /**\brief Visit incoming arcs to a \a node.
    */
  template<typename Functor>
  smtk::common::Visited inVisitor(const ToType* node, Functor ff) const
  {
    if (!node)
    {
      throw std::invalid_argument("Null to node.");
    }
    auto resource = node->resource();
    if (!resource)
    {
      throw std::invalid_argument("Input node has no parent resource.");
    }

    smtk::common::VisitorFunctor<Functor> visitor(ff);
    bool didVisit = false;

    // Find matching reverse arcs.
    auto rit = m_reverse.find(node);
    if (rit != m_reverse.end())
    {
      didVisit |= !rit->second.empty();
      for (const auto* other : rit->second)
      {
        if (other)
        {
          if (visitor(other) == smtk::common::Visit::Halt)
          {
            return smtk::common::Visited::Some;
          }
        }
      }
    }

    // If the graph is bidirectional and types match, find matching forward arcs.
    if (std::is_same<FromType, ToType>::value && !Directed::value && BidirIndex::value)
    {
      auto it = m_forward.find(reinterpret_cast<const FromType*>(node));
      if (it != m_forward.end())
      {
        didVisit = true;
        for (const auto* other : it->second)
        {
          if (other)
          {
            if (visitor(reinterpret_cast<const FromType*>(other)) == smtk::common::Visit::Halt)
            {
              return smtk::common::Visited::Some;
            }
          }
          else
          {
            throw std::runtime_error("Null node.");
          }
        }
      }
    }

    return didVisit ? smtk::common::Visited::All : smtk::common::Visited::Empty;
  }

  /// Return true if an arc exists between \a from and \a to.
  bool contains(const FromType* from, const ToType* to) const
  {
    if (!from || !to)
    {
      return false;
    }
    auto it = m_forward.find(from);
    if (it != m_forward.end())
    {
      auto it2 = it->second.find(to);
      return it2 != it->second.end();
    }

    // Handle undirected arcs where std::is_same<FromType, ToType>:
    if (std::is_same<FromType, ToType>::value && !Directed::value)
    {
      // We must also verify whether to → from exists.
      auto rit = m_reverse.find(reinterpret_cast<const ToType*>(from));
      if (rit != m_reverse.end())
      {
        auto it2 = rit->second.find(reinterpret_cast<const FromType*>(to));
        return it2 != rit->second.end();
      }
    }

    return false;
  }

  /// Return the number of outgoing arcs from the \a node.
  std::size_t outDegree(const FromType* node) const
  {
    std::size_t result = 0;
    if (!node)
    {
      return result;
    }
    auto it = m_forward.find(node);
    if (it != m_forward.end())
    {
      result = it->second.size();
    }
    if (std::is_same<FromType, ToType>::value && !Directed::value)
    {
      // Add any arcs "incoming" to the node.
      auto rit = m_reverse.find(reinterpret_cast<const ToType*>(node));
      if (rit != m_reverse.end())
      {
        result += rit->second.size();
      }
    }
    return result;
  }

  /// Return the number of incoming arcs to the \a node.
  std::size_t inDegree(const ToType* node) const
  {
    std::size_t result = 0;
    if (!node)
    {
      return result;
    }
    auto it = m_reverse.find(node);
    if (it != m_reverse.end())
    {
      result = it->second.size();
    }
    if (std::is_same<FromType, ToType>::value && !Directed::value)
    {
      // Add any arcs "outgoing" to the node.
      auto fit = m_forward.find(reinterpret_cast<const FromType*>(node));
      if (fit != m_forward.end())
      {
        result += fit->second.size();
      }
    }
    return result;
  }

  /**\brief Insert an arc from \a from to \a to,
    *       optionally ordered by \a beforeFrom and \a beforeTo.
    *
    * If the arc is ordered, then \a beforeFrom indicates where
    * \a from should be placed in the order of incoming nodes and
    * similarly for \a beforeTo and \a to.
    * If the arc is not ordered, then \a beforeFrom and \a beforeTo
    * are ignored.
    */
  //@{
  template<bool MM = Mutable::value>
  typename std::enable_if<!MM, bool>::type connect(
    const FromType* from,
    const ToType* to,
    const FromType* beforeFrom = nullptr,
    const ToType* beforeTo = nullptr)
  {
    (void)from;
    (void)to;
    (void)beforeFrom;
    (void)beforeTo;
    return false;
  }

  template<bool MM = Mutable::value>
  typename std::enable_if<MM, bool>::type connect(
    const FromType* from,
    const ToType* to,
    const FromType* beforeFrom = nullptr,
    const ToType* beforeTo = nullptr)
  {
    (void)beforeFrom;
    (void)beforeTo;
    if (!from || !to)
    {
      throw std::domain_error("Cannot connect null nodes.");
    }
    bool inserting = true;
    // For auto-undirected arcs, we must verify that to → from does not
    // already exist before inserting.
    if (std::is_same<FromType, ToType>::value && !Directed::value)
    {
      auto it = m_forward.find(reinterpret_cast<const FromType*>(to));
      if (
        it != m_forward.end() &&
        it->second.find(reinterpret_cast<const ToType*>(from)) != it->second.end())
      {
        // The arc already exists; do not allow insertion to proceed:
        inserting = false;
      }
    }
    // TODO: Also test when FromType != ToType, but arc could still have been reversed?
    //       (i.e., when From and ToTypes are distinct but share a common base class).

    // Verify that the in/out-degree constraints will be honored:
    if (inserting && (MaxOutDegree != unconstrained() || MaxInDegree != unconstrained()))
    {
      if (MaxOutDegree != unconstrained())
      {
        inserting &= (MaxOutDegree > this->outDegree(from));
      }
      if (MaxInDegree != unconstrained())
      {
        inserting &= (MaxInDegree > this->inDegree(to));
      }
    }
    // NB: We do not enforce MinInDegree or MinOutDegree because those
    //     conditions would frequently be validated. In the future,
    //     we may wish to mark our state as invalid if those (soft)
    //     constraints are invalid.
    if (inserting)
    {
      // Insert from → to.
      inserting = m_forward[from].insert(to).second;
      if (BidirIndex::value)
      {
        inserting |= m_reverse[to].insert(from).second;
      }
    }
    return inserting;
  }
  //@}

  /**\brief Remove an arc from \a from to \a to,
    *       optionally ordered by \a beforeFrom and \a beforeTo.
    *
    * If the arc is ordered, then \a beforeFrom indicates where
    * \a from should be placed in the order of incoming nodes and
    * similarly for \a beforeTo and \a to.
    * If the arc is not ordered, then \a beforeFrom and \a beforeTo
    * are ignored.
    */
  //@{
  template<bool MM = Mutable::value>
  typename std::enable_if<!MM, bool>::type disconnect(const FromType* from, const ToType* to)
  {
    (void)from;
    (void)to;
    return false;
  }

  template<bool MM = Mutable::value>
  typename std::enable_if<MM, bool>::type disconnect(const FromType* from, const ToType* to)
  {
    if (!from && !to)
    {
      throw std::domain_error("Cannot disconnect null nodes.");
    }
    bool didDisconnect = false;
    if (!from)
    {
      // We are removing all arcs to "to".
      // First handle forward arcs to "to".
      auto it = m_reverse.find(to);
      if (it != m_reverse.end())
      {
        didDisconnect = true;
        if (BidirIndex::value)
        {
          for (const auto& other : it->second)
          {
            // Remove "to" from all of the forward map's range.
            m_forward[other].erase(to);
            if (m_forward[other].empty())
            {
              m_forward.erase(other);
            }
          }
        }
        // Remove "to" from the reverse map's domain.
        m_reverse.erase(it);
      }
      if (std::is_base_of<FromType, ToType>::value && BidirIndex::value)
      {
        auto fit = m_forward.find(reinterpret_cast<const FromType*>(to));
        if (fit != m_forward.end())
        {
          didDisconnect = true;
          for (const auto& other : fit->second)
          {
            // Remove "to" from all of the reverse map's range.
            m_reverse[reinterpret_cast<const ToType*>(other)].erase(
              reinterpret_cast<const FromType*>(to));
            if (m_reverse[reinterpret_cast<const ToType*>(other)].empty())
            {
              m_reverse.erase(reinterpret_cast<const ToType*>(other));
            }
          }
          // Remove "to" from the forward map's domain.
          m_forward.erase(fit);
        }
      }
      else if (!BidirIndex::value)
      {
        // We do not have a reverse index. Iterate over the entire set of arcs
        // and look for "to"
        for (auto& entry : m_forward)
        {
          auto fit = entry.second.find(to);
          if (fit != entry.second.end())
          {
            didDisconnect = true;
            entry.second.erase(fit);
          }
        }
      }
      return didDisconnect;
    }
    auto it = m_forward.find(from);
    if (!to)
    {
      // We are removing all arcs to/from "from".
      // First handle forward arcs from "from".
      if (it != m_forward.end())
      {
        didDisconnect = true;
        if (BidirIndex::value)
        {
          for (const auto& other : it->second)
          {
            // Remove "from" from all of the reverse map's range.
            m_reverse[other].erase(from);
            if (m_reverse[other].empty())
            {
              m_reverse.erase(other);
            }
          }
        }
        // Remove "from" from the forward map's domain.
        m_forward.erase(it);
      }
      if (std::is_base_of<ToType, FromType>::value)
      {
        if (BidirIndex::value)
        {
          auto rit = m_reverse.find(reinterpret_cast<const ToType*>(from));
          if (rit != m_reverse.end())
          {
            didDisconnect = true;
            for (const auto& other : rit->second)
            {
              // Remove "from" from all of the forward map's range.
              m_forward[reinterpret_cast<const FromType*>(other)].erase(
                reinterpret_cast<const ToType*>(from));
              if (m_forward[reinterpret_cast<const FromType*>(other)].empty())
              {
                m_forward.erase(reinterpret_cast<const FromType*>(other));
              }
            }
            // Remove "from" from the reverse map's domain.
            m_reverse.erase(rit);
          }
        }
        else
        {
          // Ensure from node is not used as the "other" endpoint
          // of an arc by slowly iterating over all arcs.
          for (auto& entry : m_forward)
          {
            auto fit = entry.second.find(reinterpret_cast<const ToType*>(from));
            if (fit != entry.second.end())
            {
              didDisconnect = true;
              entry.second.erase(fit);
            }
          }
        }
      }
      return didDisconnect;
    }
    // We are only removing the single arc from → to.
    if (it != m_forward.end())
    {
      didDisconnect = it->second.erase(to) > 0;
      if (didDisconnect && BidirIndex::value)
      {
        // TODO: Check that this returns > 0:
        m_reverse[to].erase(from);
        if (m_reverse[to].empty())
        {
          m_reverse.erase(to);
        }
      }
    }
    else if (std::is_same<FromType, ToType>::value && !Directed::value)
    {
      // We may have stored this arc as to → from.
      if (BidirIndex::value)
      {
        auto rit = m_reverse.find(reinterpret_cast<const ToType*>(from));
        if (rit != m_reverse.end())
        {
          didDisconnect = rit->second.erase(reinterpret_cast<const FromType*>(to)) > 0;
          if (didDisconnect)
          {
            m_forward[reinterpret_cast<const FromType*>(to)].erase(
              reinterpret_cast<const ToType*>(from));
            if (m_forward[reinterpret_cast<const FromType*>(to)].empty())
            {
              m_forward.erase(reinterpret_cast<const FromType*>(to));
            }
          }
        }
      }
      else
      {
        it = m_forward.find(reinterpret_cast<const FromType*>(to));
        if (it != m_forward.end())
        {
          didDisconnect = it->second.erase(reinterpret_cast<const ToType*>(from)) > 0;
          if (didDisconnect && it->second.empty())
          {
            m_forward.erase(it);
          }
        }
      }
    }
    return didDisconnect;
  }
  //@}

protected:
  std::unordered_map<const FromType*, std::unordered_set<const ToType*>> m_forward;
  std::unordered_map<const ToType*, std::unordered_set<const FromType*>> m_reverse;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_ExplicitArcs_h
