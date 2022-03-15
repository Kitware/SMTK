//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#define STREAM_EXPR(expr) #expr << ": " << (expr)
#define VERBOSE 0

#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"

#include "smtk/graph/arcs/Arc.h"
#include "smtk/graph/arcs/Arcs.h"
#include "smtk/graph/arcs/OrderedArcs.h"

#include <fstream>
#include <iostream>
#include <sstream>

static int check_count = 0;
static int failure_count = 0;
static bool throw_check;

#if !defined(WIN32)
static std::ofstream null_stream("/dev/null");
#else
static std::ofstream null_stream("dump.txt");
#endif

#define REQUIRE(expr)                                                                              \
  check_count++;                                                                                   \
  (expr) ? null_stream                                                                             \
         : (failure_count++, std::cout << "Check failed.\n\tExpression: (" << #expr << ")\n")

#define REQUIRE_THROW(expr)                                                                        \
  check_count++;                                                                                   \
  throw_check = false;                                                                             \
  try                                                                                              \
  {                                                                                                \
    (expr);                                                                                        \
  }                                                                                                \
  catch (...)                                                                                      \
  {                                                                                                \
    throw_check = true;                                                                            \
  }                                                                                                \
  throw_check ? null_stream : (failure_count++, std::cout << #expr << " did not throw.\n")

#define FINISH_TEST()                                                                              \
  null_stream.close();                                                                             \
  std::cout << "\nSummary:\n";                                                                     \
  if (failure_count)                                                                               \
  {                                                                                                \
    std::cout << "\tFailed " << failure_count << " out of " << check_count << " checks\n";         \
  }                                                                                                \
  else                                                                                             \
  {                                                                                                \
    std::cout << "\tPassed " << check_count << " checks\n";                                        \
  }                                                                                                \
  return failure_count ? 1 : 0

namespace
{
/// A central graph node type, everything connects to it, and
/// visa versa
class Node : public smtk::graph::Component
{
  std::string m_name;
  int m_index;

public:
  template<typename... Args>
  Node(Args&&... args)
    : smtk::graph::Component(std::forward<Args>(args)...)
  {
  }
  void initialize(std::string name, int index)
  {
    m_name = name;
    m_index = index;
  }
  std::string name() const override { return m_name; }
  int index() const { return m_index; }
};

/// A type of thing that is connected to a node
/// Each node may have many labels
/// Each label may have a single node
/// Each Label has information that distinguishs it from other Labels
class Label : public smtk::graph::Component
{
  std::string m_info;

public:
  template<typename... Args>
  Label(Args&&... args)
    : smtk::graph::Component(std::forward<Args>(args)...)
  {
  }

  void initialize(std::string info_string) { m_info = info_string; }

  std::string info() { return m_info; }
};

/// A type of thing that is connected to a node.
/// Each node can only have one flag.
class Flag : public Label
{
public:
  template<typename... Args>
  Flag(Args&&... args)
    : Label(std::forward<Args>(args)...)
  {
  }
};

///////////////////////////////////////////////////////////////////////////////
// Arc(Flags) <=> Arc(FlaggedNode)
class Flags;
class FlaggedNode;

/// Connect a flag to a node, implies FlaggedNode
class Flags : public smtk::graph::Arc<Node, Flag, Flags>
{
public:
  using InverseArcType = FlaggedNode;
  using smtk::graph::Arc<Node, Flag, Flags>::Arc;
};

/// Connect node to a flag, implies Flags
class FlaggedNode : public smtk::graph::Arc<Flag, Node, FlaggedNode>
{
public:
  using InverseArcType = Flags;
  using smtk::graph::Arc<Flag, Node, FlaggedNode>::Arc;
};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Arcs(Labels) <=> Arc(LabeledNode)
class Labels;
class LabeledNode;

class Labels : public smtk::graph::Arcs<Node, Label, Labels>
{
public:
  using InverseArcType = LabeledNode;
  using smtk::graph::Arcs<Node, Label, Labels>::Arcs;
};

class LabeledNode : public smtk::graph::Arc<Label, Node, LabeledNode>
{
public:
  using InverseArcType = Labels;
  using smtk::graph::Arc<Label, Node, LabeledNode>::Arc;
};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Arcs(OtherNodes) <=> Arcs(OtherNodes)
class OtherNodes : public smtk::graph::Arcs<Node, Node, OtherNodes>
{
public:
  using InverseArcType = OtherNodes;
  using smtk::graph::Arcs<Node, Node, OtherNodes>::Arcs;
};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Arcs(UnorderedOtherNodes) <=> OrderedArcs(OrderedOtherNodes)
class OrderedOtherNodes;
class UnorderedOtherNodes;

class OrderedOtherNodes : public smtk::graph::OrderedArcs<Node, Node, OrderedOtherNodes>
{
public:
  using InverseArcType = UnorderedOtherNodes;
  using smtk::graph::OrderedArcs<Node, Node, OrderedOtherNodes>::OrderedArcs;
};

class UnorderedOtherNodes : public smtk::graph::Arcs<Node, Node, UnorderedOtherNodes>
{
public:
  using InverseArcType = OrderedOtherNodes;
  using smtk::graph::Arcs<Node, Node, UnorderedOtherNodes>::Arcs;
};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Arc() <=> OrderedArcs()
class OrderedAllLabels;
class TheNode;

class OrderedAllLabels : public smtk::graph::OrderedArcs<Node, Label, OrderedAllLabels>
{
public:
  using InverseArcType = TheNode;
  using smtk::graph::OrderedArcs<Node, Label, OrderedAllLabels>::OrderedArcs;
};

class TheNode : public smtk::graph::Arc<Label, Node, TheNode>
{
public:
  using InverseArcType = OrderedAllLabels;
  using smtk::graph::Arc<Label, Node, TheNode>::Arc;
};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Graph Traits
struct LabeledAndFlaggedNodeTraits
{
  using NodeTypes = std::tuple<Node, Label, Flag>;
  using ArcTypes = std::tuple<
    Labels,
    LabeledNode,
    Flags,
    FlaggedNode,
    OtherNodes,
    OrderedAllLabels,
    TheNode,
    OrderedOtherNodes,
    UnorderedOtherNodes>;
};
///////////////////////////////////////////////////////////////////////////////
} // namespace

/// Test inserting arc types that have inverses.
/// TODO (ryan.krattiger) Add erase and assignment cases for each set of arc types.
int TestInverseArcs(int argc, char* argv[])
{
#define REQUIRE_INVERTIBLE_ARC(Type, Inverse)                                                      \
  REQUIRE(!smtk::graph::detail::is_ordered_arcs<Type>::value);                                     \
  REQUIRE((smtk::graph::detail::is_inverse_pair<Type, Inverse>::value));                           \
  REQUIRE(smtk::graph::detail::has_inverse<Type>::value)

#define REQUIRE_ORDERED_INVERTIBLE_ARC(Type, Inverse)                                              \
  REQUIRE(smtk::graph::detail::is_ordered_arcs<Type>::value);                                      \
  REQUIRE((smtk::graph::detail::is_inverse_pair<Type, Inverse>::value));                           \
  REQUIRE(smtk::graph::detail::has_inverse<Type>::value)

  // Check type traits
  REQUIRE_INVERTIBLE_ARC(Labels, LabeledNode);
  REQUIRE_INVERTIBLE_ARC(LabeledNode, Labels);
  REQUIRE_INVERTIBLE_ARC(Flags, FlaggedNode);
  REQUIRE_INVERTIBLE_ARC(FlaggedNode, Flags);
  REQUIRE_INVERTIBLE_ARC(OtherNodes, OtherNodes);
  REQUIRE_INVERTIBLE_ARC(TheNode, OrderedAllLabels);
  REQUIRE_ORDERED_INVERTIBLE_ARC(OrderedAllLabels, TheNode);
  REQUIRE_INVERTIBLE_ARC(UnorderedOtherNodes, OrderedOtherNodes);
  REQUIRE_ORDERED_INVERTIBLE_ARC(OrderedOtherNodes, UnorderedOtherNodes);

  // By default construct 4 nodes
  int numNodes = 4;
  if (argc == 2)
  {
    numNodes = std::stod(argv[1]);
  }
  else if (argc > 2)
  {
    std::cerr << "Too many arguments, expected 0 or 1\n";
    return 0;
  }

  auto graph = smtk::graph::Resource<LabeledAndFlaggedNodeTraits>::create();
  // Create some nodes
  std::vector<std::shared_ptr<Node>> nodes(numNodes);
  int index = 0;
  for (auto& node : nodes)
  {
    std::stringstream name_str;
    name_str << "node:" << index;
    node = graph->create<Node>(name_str.str(), index);
    index++;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Each node will get 3 labels, 2 normal labels and 1 flag
  //
  // \a toggle is used to test adding arcs in different orders
  bool toggle = true;
  for (const auto& node : nodes)
  {
    std::stringstream label_str1;
    label_str1 << "node_to_label(" << toggle << "):" << node->id();
    auto label1 = graph->create<Label>(label_str1.str());

    std::stringstream label_str2;
    label_str2 << "label_to_node(" << toggle << "):" << node->id();
    auto label2 = graph->create<Label>(label_str2.str());

    std::stringstream flag_str;
    flag_str << "flag(" << toggle << "):" << node->id();
    auto flag = graph->create<Flag>(flag_str.str());
    if (toggle)
    {
      toggle = false;
      //////////////////////////////////////////////////////////////////////////
      // Arc -> Arc
      graph->connect<Flags>(*node, *flag);
      //////////////////////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////////////////////
      // Arcs -> Arc
      // Arc -> Arcs
      graph->connect<Labels>(*node, *label1);
      graph->connect<LabeledNode>(*label2, *node);
      //////////////////////////////////////////////////////////////////////////
    }
    else
    {
      toggle = true;
      //////////////////////////////////////////////////////////////////////////
      // Arc -> Arc
      graph->connect<FlaggedNode>(*flag, *node);
      //////////////////////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////////////////////
      // Arc -> Arcs
      // Arcs -> Arc
      graph->connect<LabeledNode>(*label2, *node);
      graph->connect<Labels>(*node, *label1);
      //////////////////////////////////////////////////////////////////////////
    }
    ////////////////////////////////////////////////////////////////////////////
    // OrderedArcs -> Arc
    graph->connect<OrderedAllLabels>(*node, *label1, *label2, *flag);
    ////////////////////////////////////////////////////////////////////////////
  }

  //////////////////////////////////////////////////////////////////////////////
  // Arcs -> OrderedArcs fails with exception
  REQUIRE_THROW((graph->connect<UnorderedOtherNodes>(*nodes[0], *nodes[1])))
    << "Adding OrderedArcs implicitly should throw\n";
  REQUIRE_THROW(graph->arcs().at<UnorderedOtherNodes>(nodes[0]->id()));
  for (int i = 0; i < numNodes; ++i)
  {
    ////////////////////////////////////////////////////////////////////////////
    // OrderArcs -> Arcs
    graph->connect<OrderedOtherNodes>(
      *nodes[i], *nodes[(i + 1) % 4], *nodes[(i + 2) % 4], *nodes[(i + 3) % 4]);
    for (int j = i + 1; j < numNodes; ++j)
    {
      //////////////////////////////////////////////////////////////////////////
      // Arcs -> Arcs
      graph->connect<OtherNodes>(*nodes[i], *nodes[j]);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  // Check the arcs and their inverses have been created
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  for (const auto& node : graph->nodes())
  {
    if (auto n = std::dynamic_pointer_cast<Node>(node))
    {
#if VERBOSE
      std::cout << "Node: " << n->id() << "\n";
      std::cout << "  Name: " << n->name() << "\n";
#endif

      REQUIRE(n->get<Labels>().count() == 2) << "  Count: " << n->get<Labels>().count() << "\n";
#if VERBOSE
      std::cout << "\tNode Labels\n";
      for (auto& label : n->get<Labels>())
      {
        std::cout << "\t\t" << static_cast<Label&>(label).info() << "\n";
      }
#endif

      REQUIRE(n->get<Flags>().count() == 1) << "  Count: " << n->get<Flags>().count() << "\n";
#if VERBOSE
      std::cout << "\tNode Flags\n";
      for (auto& label : n->get<Flags>())
      {
        std::cout << "\t\t" << static_cast<Label&>(label).info() << "\n";
      }
#endif

      REQUIRE(n->get<OrderedAllLabels>().count() == 3)
        << "  Count: " << n->get<OrderedAllLabels>().count() << "\n";
#if VERBOSE
      std::cout << "\tNode Ordered All Labels\n";
      for (auto& label : n->get<OrderedAllLabels>())
      {
        std::cout << "\t\t" << static_cast<Label&>(label).info() << "\n";
      }
#endif

      REQUIRE(n->get<OtherNodes>().count() == numNodes - 1)
        << "  Count: " << n->get<OtherNodes>().count() << "\n";
#if VERBOSE
      std::cout << "\tOther Nodes:\n";
      for (auto& othernode : n->get<OtherNodes>())
      {
        std::cout << "\t\t" << static_cast<Node&>(othernode).id() << "\n";
      }
#endif

      REQUIRE(n->get<OrderedOtherNodes>().count() == numNodes - 1)
        << "  Count: " << n->get<OrderedOtherNodes>().count() << "\n";
      int prevIndex = n->index();
#if VERBOSE
      std::cout << "\tOrdered Nodes:\n";
#endif
      for (auto& othernode : n->get<OrderedOtherNodes>())
      {
        REQUIRE(static_cast<Node&>(othernode).index() == (prevIndex + 1) % 4)
          << "Ordered nodes our not ordered";
        prevIndex = static_cast<Node&>(othernode).index();
#if VERBOSE
        std::cout << "\t\t" << static_cast<Node&>(othernode).index() << " "
                  << static_cast<Node&>(othernode).id() << "\n";
#endif
      }

      REQUIRE(n->get<UnorderedOtherNodes>().count() == numNodes - 1)
        << "  Count: " << n->get<UnorderedOtherNodes>().count() << "\n";
#if VERBOSE
      std::cout << "\tUnordered Nodes:\n";
      for (auto& othernode : n->get<UnorderedOtherNodes>())
      {
        std::cout << "\t\t" << static_cast<Node&>(othernode).index() << " "
                  << static_cast<Node&>(othernode).id() << "\n";
      }
#endif
    }
    else if (auto f = std::dynamic_pointer_cast<Flag>(node))
    {
#if VERBOSE
      std::cout << "Flag: " << f->id() << "\n"
                << "  Info: " << f->info() << "\n";
#endif

      REQUIRE(f->get<FlaggedNode>().count() == 1)
        << "  Count: " << f->get<FlaggedNode>().count() << "\n";
#if VERBOSE
      std::cout << "\tFlagged Node:\n";
      for (auto& node : f->get<FlaggedNode>())
      {
        std::cout << "\t\t" << node.id() << "\n";
      }
#endif
    }
    else if (auto l = std::dynamic_pointer_cast<Label>(node))
    {
#if VERBOSE
      std::cout << "Label: " << l->id() << "\n"
                << "  Info: " << l->info() << "\n";
#endif

      REQUIRE(l->get<LabeledNode>().count() == 1)
        << "  Count: " << l->get<LabeledNode>().count() << "\n";
#if VERBOSE
      std::cout << "\tLabled Node:\n";
      for (auto& node : l->get<LabeledNode>())
      {
        std::cout << "\t\t" << node.id() << "\n";
      }
#endif
    }
    else
    {
      REQUIRE(false) << "Invalid node cast\n";
    }

    if (auto l = std::dynamic_pointer_cast<Label>(node))
    {
      smtk::common::UUID id;
      if (auto f = std::dynamic_pointer_cast<Flag>(l))
      {
        id = static_cast<Node&>(f->get<FlaggedNode>().to()).id();
      }
      else
      {
        id = static_cast<Node&>(l->get<LabeledNode>().to()).id();
      }

      REQUIRE(static_cast<Node&>(l->get<TheNode>().to()).id() == id) << "  Node IDs do not match\n";

      REQUIRE(l->get<TheNode>().count() == 1) << "  Count: " << l->get<TheNode>().count() << "\n";
#if VERBOSE
      std::cout << "\tLabled Node:\n";
      for (auto& node : l->get<TheNode>())
      {
        std::cout << "\t\t" << node.id() << "\n";
      }
#endif
    }
  }

  FINISH_TEST();
}
