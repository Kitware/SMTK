//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"
#include "smtk/graph/arcs/Arc.h"

/// A basic example that constructs two nodes and noe arc that connects them.

namespace
{
/// Node inherits from smtk::graph::Component and uses its constructor.
class Node : public smtk::graph::Component
{
public:
  template <typename... Args>
  Node(Args&&... args)
    : smtk::graph::Component::Component(std::forward<Args>(args)...)
  {
  }
};

/// Arc is a basic smtk::graph::Arc that connects from one node to another node.
class Arc : public smtk::graph::Arc<Node, Node>
{
public:
  template <typename... Args>
  Arc(Args&&... args)
    : smtk::graph::Arc<Node, Node>::Arc(std::forward<Args>(args)...)
  {
  }
};

/// A description of the node types and arc types that comprise our test
/// graph resource.
struct BasicTraits
{
  typedef std::tuple<Node> NodeTypes;
  typedef std::tuple<Arc> ArcTypes;
};
}

int TestNodalResource(int, char* [])
{
  // Construct a graph resource with the graph and node types described in
  // BasicTraits.
  auto resource = smtk::graph::Resource<BasicTraits>::create();

  std::cout << resource->typeName() << std::endl;

  // Construct two instances of our node through the resource's API.
  auto node1 = resource->create<Node>();
  auto node2 = resource->create<Node>();

  std::cout << node1->typeName() << std::endl;

  // Construct an arc that connects our two node instance.
  const auto& arc = resource->create<Arc>(*node1, *node2);
  (void)arc;

  std::cout << smtk::common::typeName<Arc>() << std::endl;

  std::cout << node1->id() << std::endl;
  std::cout << node2->id() << std::endl;

  // Access the second node using the first node's API.
  std::cout << node1->get<Arc>().id() << std::endl;

  return 0;
}
