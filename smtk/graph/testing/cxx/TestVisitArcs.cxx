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

#include "smtk/common/testing/cxx/helpers.h"

/// A basic example that constructs two nodes and noe arc that connects them.

namespace test_visit_arcs
{
/// Node inherits from smtk::graph::Component and uses its constructor.
class Node : public smtk::graph::Component
{
public:
  template<typename... Args>
  Node(Args&&... args)
    : smtk::graph::Component::Component(std::forward<Args>(args)...)
  {
  }
};

/// Arc is a basic smtk::graph::Arc that connects from one node to another node.
class ArcWithoutVisit : public smtk::graph::Arc<Node, Node>
{
public:
  template<typename... Args>
  ArcWithoutVisit(Args&&... args)
    : smtk::graph::Arc<Node, Node>::Arc(std::forward<Args>(args)...)
  {
  }
};

class ArcWithVisit : public smtk::graph::Arc<Node, Node>
{
public:
  template<typename... Args>
  ArcWithVisit(Args&&... args)
    : smtk::graph::Arc<Node, Node>::Arc(std::forward<Args>(args)...)
  {
  }

  template<typename SelfType>
  class API : public smtk::graph::Arc<test_visit_arcs::Node, test_visit_arcs::Node>::API<SelfType>
  {
  public:
    bool visit(const FromType& lhs, std::function<bool(const Node&)> visitor) const
    {
      smtk::graph::Arc<test_visit_arcs::Node, test_visit_arcs::Node>::API<SelfType>::self(lhs)
        .constVisited = true;
      return visitor(this->get(lhs).to());
    }

    bool visit(const FromType& lhs, std::function<bool(Node&)> visitor)
    {
      smtk::graph::Arc<test_visit_arcs::Node, test_visit_arcs::Node>::API<SelfType>::self(lhs)
        .visited = true;
      return visitor(this->get(lhs).to());
    }
  };

  bool visited{ false };
  mutable bool constVisited{ false };
};

/// A description of the node types and arc types that comprise our test
/// graph resource.
struct BasicTraits
{
  typedef std::tuple<test_visit_arcs::Node> NodeTypes;
  typedef std::tuple<test_visit_arcs::ArcWithoutVisit, test_visit_arcs::ArcWithVisit> ArcTypes;
};
} // namespace test_visit_arcs

int TestVisitArcs(int, char*[])
{
  // Construct a graph resource with the graph and node types described in
  // BasicTraits.
  auto resource = smtk::graph::Resource<test_visit_arcs::BasicTraits>::create();

  // Construct two instances of our node through the resource's API.
  auto node1 = resource->create<test_visit_arcs::Node>();
  auto node2 = resource->create<test_visit_arcs::Node>();

  // Construct an arc that connects our two node instance.
  const auto& arc = resource->create<test_visit_arcs::ArcWithoutVisit>(*node1, *node2);
  (void)arc;

  // test const get
  {
    const test_visit_arcs::Node& n = *node1;
    test(
      node2->id() == n.get<test_visit_arcs::ArcWithoutVisit>().to().id(),
      "Cannot access connected node via get() const");
  }

  // test non-const get
  {
    test_visit_arcs::Node& n = *node1;
    n.get<test_visit_arcs::ArcWithoutVisit>().to().setId(smtk::common::UUID::random());
    test(
      node2->id() == n.get<test_visit_arcs::ArcWithoutVisit>().to().id(),
      "Cannot access connected node via get()");
  }

  // Test const visit
  {
    bool visited = false;
    auto constVisitor = [&node2, &visited](const smtk::graph::Component& node) {
      test(node2->id() == node.id(), "Cannot access connected node via visit() const");
      visited = true;
      return false;
    };

    const test_visit_arcs::Node& n = *node1;

    n.visit<test_visit_arcs::ArcWithoutVisit>(constVisitor);
    test(visited, "Cannot visit connected node via visit() const");
  }

  // Test non-const visit
  {
    bool visited = false;
    auto visitor = [&node2, &visited](smtk::graph::Component& node) {
      node.setId(smtk::common::UUID::random());
      test(node2->id() == node.id(), "Cannot access connected node via visit() const");
      visited = true;
      return false;
    };

    node1->visit<test_visit_arcs::ArcWithoutVisit>(visitor);
    test(visited, "Cannot visit connected node via visit() const");
  }

  // Construct an arc that connects our two node instance.
  const auto& arcWithVisit = resource->create<test_visit_arcs::ArcWithVisit>(*node1, *node2);
  (void)arcWithVisit;

  test(!arcWithVisit.visited);
  test(!arcWithVisit.constVisited);

  // test const visit with custom visit logic
  {
    bool visited = false;
    auto constVisitor = [&node2, &visited](const smtk::graph::Component& node) {
      test(node2->id() == node.id(), "Cannot access connected node via visit() const");
      visited = true;
      return false;
    };

    const test_visit_arcs::Node& n = *node1;

    n.visit<test_visit_arcs::ArcWithVisit>(constVisitor);
    test(visited, "Cannot visit connected node via visit() const");
    test(!arcWithVisit.visited, "Incorrect custom visit() called");
    test(arcWithVisit.constVisited, "Failed to use custom visit() const");
  }

  arcWithVisit.constVisited = false;

  // test non-const visit with custom visit logic
  {
    bool visited = false;
    auto visitor = [&node2, &visited](smtk::graph::Component& node) {
      test(node2->id() == node.id(), "Cannot access connected node via visit()");
      visited = true;
      return false;
    };

    test_visit_arcs::Node& n = *node1;

    n.visit<test_visit_arcs::ArcWithVisit>(visitor);
    test(visited, "Cannot visit connected node via visit() const");
    test(!arcWithVisit.constVisited, "Incorrect custom visit() const called");
    test(arcWithVisit.visited, "Failed to use custom visit()");
  }

  return 0;
}
