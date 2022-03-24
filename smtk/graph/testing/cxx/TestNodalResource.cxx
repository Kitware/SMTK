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

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>

/// A basic example that constructs two nodes and noe arc that connects them.

namespace test_nodal_resource
{
/// Node inherits from smtk::graph::Component and uses its constructor.
class Node : public smtk::graph::Component
{
public:
  smtkTypeMacro(test_nodal_resource::Node);
  template<typename... Args>
  Node(Args&&... args)
    : smtk::graph::Component::Component(std::forward<Args>(args)...)
  {
  }
};

class BadNode : public smtk::graph::Component
{
public:
  smtkTypeMacro(test_nodal_resource::BadNode);
  template<typename... Args>
  BadNode(Args&&... args)
    : smtk::graph::Component::Component(std::forward<Args>(args)...)
  {
  }
};

/// Arc is a basic smtk::graph::Arc that connects from one node to another node.
struct Arc
{
  using FromType = Node;
  using ToType = Node;
  using Directed = std::false_type;
};

/// A description of the node types and arc types that comprise our test
/// graph resource.
struct BasicTraits
{
  typedef std::tuple<test_nodal_resource::Node> NodeTypes;
  typedef std::tuple<test_nodal_resource::Arc> ArcTypes;
};
} // namespace test_nodal_resource

int TestNodalResource(int, char*[])
{
  // Construct a graph resource with the graph and node types described in
  // BasicTraits.
  auto resource = smtk::graph::Resource<test_nodal_resource::BasicTraits>::create();

  std::cout << resource->typeName() << std::endl;

  // Construct two instances of our node through the resource's API.
  auto node1 = resource->create<test_nodal_resource::Node>();
  auto node2 = resource->create<test_nodal_resource::Node>();

  // Verify that adding nodes that belong to a different resource cause an exception.
  auto node3 = std::make_shared<test_nodal_resource::Node>(nullptr);
  bool didThrow = false;
  try
  {
    resource->add(node3);
  }
  catch (std::invalid_argument& /* ee */)
  {
    didThrow = true;
  }
  test(didThrow, "Adding an improper node should throw an exception.");

  // Verify that it is impossible to add a node of the wrong type at run time.
  auto badNode = smtk::make_shared<test_nodal_resource::BadNode>(resource);
  auto hiddenBadNode = std::dynamic_pointer_cast<smtk::graph::Component>(badNode);
  test(!resource->addNode(hiddenBadNode), "Expecting adding a bad node to fail.");

  std::cout << node1->typeName() << std::endl;

  test(!node2->outgoing<test_nodal_resource::Arc>().contains(node1.get()), "Arc should not exist.");

  // Construct an arc that connects our two node instance.
  const auto& arc = node1->outgoing<test_nodal_resource::Arc>().connect(node2.get());
  (void)arc;

  std::cout << smtk::common::typeName<test_nodal_resource::Arc>() << std::endl;

  auto printNodes = [](const test_nodal_resource::Node* other) {
    std::cout << "    connected to " << other->id() << std::endl;
  };
  std::cout << "node 1 " << node1->id() << std::endl;
  node1->outgoing<test_nodal_resource::Arc>().visit(printNodes);
  std::cout << "node 2 " << node2->id() << std::endl;
  node2->outgoing<test_nodal_resource::Arc>().visit(printNodes);

  // Verify that, because the Arc class is marked as undirected,
  // either endpoint sees the other node as attached.
  test(
    node2->outgoing<test_nodal_resource::Arc>().contains(node1.get()),
    "Arc existence not tested properly.");
  test(
    node1->outgoing<test_nodal_resource::Arc>().contains(node2.get()),
    "Arc existence not tested properly.");
  test(
    node2->incoming<test_nodal_resource::Arc>().contains(node1.get()),
    "Arc existence not tested properly.");
  test(
    node1->incoming<test_nodal_resource::Arc>().contains(node2.get()),
    "Arc existence not tested properly.");

  return 0;
}
