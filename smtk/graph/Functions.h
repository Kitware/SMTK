//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_Functions_h
#define smtk_graph_Functions_h

#include "smtk/graph/ArcImplementation.h"
#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"

#include <set>
#include <utility>

namespace smtk
{
/// Subsystem for modeling using nodes connected to one another by arcs.
namespace graph
{

/**\brief A template for visiting nodes at either end of a given node and arc type.
 */
///@{
template<typename NodeType, typename ArcType, typename Outgoing>
struct OtherNodeVisitor;

template<typename NodeType, typename ArcType>
struct OtherNodeVisitor<NodeType, ArcType, smtk::graph::OutgoingArc>
{
  void operator()(
    typename ArcType::FromType* node,
    std::function<void(const typename ArcType::ToType*)> visitor)
  {
    if (node)
    {
      node->template outgoing<ArcType>().visit(visitor);
    }
  }
};

template<typename NodeType, typename ArcType>
struct OtherNodeVisitor<NodeType, ArcType, smtk::graph::IncomingArc>
{
  void operator()(
    typename ArcType::ToType* node,
    std::function<void(const typename ArcType::FromType*)> visitor)
  {
    if (node)
    {
      node->template incoming<ArcType>().visit(visitor);
    }
  }
};
///@}

/**\brief Find correspondences between destination nodes of two nodes along
  *       the given \a ArcType.
  *
  * Given:
  * + an \a ArcType arc-trait object,
  * + two nodes (\a nodeA and \a nodeB), and
  * + a lambda \a comparator that returns true if two destination nodes
  *   (\a nodeC and \a nodeD, connected to \a nodeA and \a nodeB, respectively)
  *   correspond to one another.
  *
  * This function returns a set of pairs of nodes such that
  * + if both nodes in a pair are non-null, the \a comparator returns true; and
  * + if one node in a pair is null, there are no nodes \a nodeC and \a nodeD
  *   that correspond to one another attached to \a nodeA and \a nodeB, respectively.
  */
template<
  typename ArcType,
  typename SourceNode,
  typename Outgoing = typename std::conditional<
    std::is_same<typename ArcType::FromType, SourceNode>::value,
    smtk::graph::OutgoingArc,
    smtk::graph::IncomingArc>::type,
  typename TargetNode = typename std::
    conditional<Outgoing::value, typename ArcType::ToType, typename ArcType::FromType>::type>
std::set<std::pair<TargetNode*, TargetNode*>> findArcCorrespondences(
  SourceNode* nodeA,
  SourceNode* nodeB,
  std::function<bool(TargetNode*, TargetNode*)> comparator)
{

  std::set<TargetNode*> targetsA;
  OtherNodeVisitor<SourceNode, ArcType, Outgoing>()(nodeA, [&targetsA](const TargetNode* nodeC) {
    targetsA.insert(const_cast<TargetNode*>(nodeC));
  });
  std::set<std::pair<TargetNode*, TargetNode*>> matches;
  OtherNodeVisitor<SourceNode, ArcType, Outgoing>()(
    nodeB, [&targetsA, &matches, &comparator](const TargetNode* constNodeD) {
      auto* nodeD = const_cast<TargetNode*>(constNodeD);
      for (auto* candidate : targetsA)
      {
        if (comparator(candidate, nodeD))
        {
          matches.insert(std::make_pair(candidate, nodeD));
          targetsA.erase(candidate);
          return;
        }
      }
      matches.insert(std::make_pair(nullptr, nodeD));
    });
  // Any nodes remaining in targetsA are unmatched; insert a null correspondence.
  for (auto* unmatched : targetsA)
  {
    matches.insert(std::make_pair(unmatched, nullptr));
  }
  return matches;
}

/// A variant of findArcCorrespondences that takes shared pointers to nodes.
template<
  typename ArcType,
  typename SourceNode,
  typename Outgoing = std::conditional<
    std::is_same<typename ArcType::FromType, SourceNode>::value,
    smtk::graph::OutgoingArc,
    smtk::graph::IncomingArc>,
  typename TargetNode =
    std::conditional<Outgoing::value, typename ArcType::ToType, typename ArcType::FromType>>
std::set<std::pair<TargetNode*, TargetNode*>> findArcCorrespondences(
  const std::shared_ptr<SourceNode>& nodeA,
  const std::shared_ptr<SourceNode>& nodeB,
  std::function<bool(TargetNode*, TargetNode*)> comparator)
{
  return findArcCorrespondences(nodeA.get(), nodeB.get(), comparator);
}

} // namespace graph
} // namespace smtk

#endif // smtk_graph_Functions_h
