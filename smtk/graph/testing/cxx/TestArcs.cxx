//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/graph/testing/cxx/TestArcs.h"
#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"
#include "smtk/graph/evaluators/Dump.h"
#include "smtk/graph/json/jsonResource.h"
#include "smtk/resource/json/Helper.h"

#include "smtk/io/Logger.h"

#include "smtk/common/Visit.h"
#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <type_traits>

namespace smtk
{
namespace test
{
using namespace smtk::graph;

int ImplicitArc::s_outVisitorCount = 0;
int InvertibleImplicitArc::s_inVisitorCount = 0;

// ++ 4 ++
void to_json(json& j, const Thingy* thingy)
{
  j["id"] = thingy->id();
  if (!thingy->name().empty())
  {
    j["name"] = thingy->name();
  }
}
// -- 4 --

// ++ 5 ++
template<typename NodeType>
void from_json(const json& j, std::shared_ptr<NodeType>& node)
{
  auto& helper = smtk::resource::json::Helper::instance();
  auto resource = std::dynamic_pointer_cast<smtk::graph::ResourceBase>(helper.resource());
  if (resource)
  {
    // Construct a node of the proper type with its resource and UUID set.
    // Note that you must provide a constructor that passes these arguments
    // to the base graph-resource component class or you will have build errors.
    node = std::make_shared<NodeType>(resource, j["id"].get<smtk::common::UUID>());
    auto it = j.find("name");
    if (it != j.end())
    {
      node->setName(it->get<std::string>());
    }
    // Adding the node can fail if the node's type is disallowed by the resource.
    if (!resource->addNode(node))
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Could not add node.");
    }
  }
}
// -- 5 --

struct PrintArcInfo
{
  template<typename ResourceType>
  static void begin(const ResourceType*)
  {
    std::cout << "  Arc type                  Implicit Explicit Mutable  Bidir   Directed\n"
                 "  ---------------------------------------------------------------------\n";
  }
  template<typename T, typename ResourceType>
  void operator()(T* arcData, const ResourceType*)
  {
    (void)arcData;
    using Traits = typename T::Traits;
    std::string typeName = smtk::common::typeName<Traits>();
    if (typeName.find("smtk::test::") == 0 || typeName.find("smtk::test::") == 0)
    {
      typeName = typeName.substr(12);
    }
#ifndef WIN32
    const std::string xx = "×";
#else
    const std::string xx = "x";
#endif
    const std::string oo = " ";
    std::cout << "  " << typeName << std::string(25 - typeName.size(), ' ') << "    "
              << (ArcProperties<Traits>::isImplicit::value ? xx : oo) << "    "
              << "    " << (ArcProperties<Traits>::isExplicit::value ? xx : oo) << "    "
              << "    " << (ArcProperties<Traits>::isMutable::value ? xx : oo) << "    "
              << "    " << (ArcProperties<Traits>::isOnlyForwardIndexed::value ? oo : xx) << "    "
              << "    " << (Traits::Directed::value ? xx : oo) << "    "
              << "\n"
              << "       From: " << smtk::common::typeName<typename Traits::FromType>() << "\n"
              << "       To:   " << smtk::common::typeName<typename Traits::ToType>() << "\n";
  }
  template<typename ResourceType>
  static void end(const ResourceType*)
  {
    std::cout << "  ---------------------------------------------------------------------\n";
  }
};

void testArcProperties()
{
  using namespace smtk::graph;
  std::cout << "Test arc properties\n";

  // Print out properties of the arc types held by our example resource:
  auto resource = smtk::graph::Resource<ExampleTraits>::create();
  resource->evaluateArcs<PrintArcInfo>();

  // Test that the properties match what we expect:
  ::test(
    ArcProperties<InvertibleImplicitArc>::isImplicit::value,
    "Expected InvertibleImplicitArc to be implicit.");
  ::test(
    !ArcProperties<InvertibleImplicitArc>::isExplicit::value,
    "Expected InvertibleImplicitArc to not be explicit.");

  ::test(ArcProperties<ImplicitArc>::isImplicit::value, "Expected ImplicitArc to be implicit.");
  ::test(
    !ArcProperties<ImplicitArc>::isExplicit::value, "Expected ImplicitArc to not be explicit.");

  ::test(
    !ArcProperties<ExplicitArc>::isImplicit::value, "Expected ExplicitArc to not be implicit.");
  ::test(ArcProperties<ExplicitArc>::isExplicit::value, "Expected ExplicitArc to be explicit.");

  ::test(
    !ArcProperties<InvertibleImplicitArc>::isMutable::value,
    "Expected InvertibleImplicitArc to be immutable.");
  ::test(!ArcProperties<ImplicitArc>::isMutable::value, "Expected ImplicitArc to be immutable.");
  ::test(ArcProperties<ExplicitArc>::isMutable::value, "Expected ExplicitArc to be mutable.");

  ::test(
    ArcProperties<InvertibleImplicitArc>::isImplicit::value,
    "Expected InvertibleImplicitArc to be implicit.");
  ::test(
    !ArcProperties<InvertibleImplicitArc>::isExplicit::value,
    "Expected InvertibleImplicitArc to not be explicit.");

  // Test that our ExplicitArc traits-class does not implement a contains() method, but that
  // the inferred storage class *does* implement contains() for us. (We want to prevent
  // regressions where the default, slow implementation is used.)
  using ExplicitStorage =
    typename smtk::graph::detail::SelectArcContainer<ExplicitArc, ExplicitArc>::type;
  ::test(
    !ArcProperties<ExplicitArc>::hasContains<ExplicitArc>::value,
    "Expected ExplicitArc to not implement contains().");
  ::test(
    ArcProperties<ExplicitStorage>::hasContains<ExplicitStorage>::value,
    "Expected ExplicitArc's storage class to implement contains().");

  // Test that our explicit arc's storage class provides visitors for nodes with
  // outgoing/incoming arcs (and only those nodes).
  ::test(
    ArcProperties<ExplicitStorage>::template hasOutNodeVisitor<ExplicitStorage>::value,
    "Expect explicit arc type to provide an outgoing-node visitor.");
  ::test(
    ArcProperties<ExplicitStorage>::template hasInNodeVisitor<ExplicitStorage>::value,
    "Expect explicit arc type to provide an incoming-node visitor.");

  // Test that the minimum- and maximum- in- and out- degrees are properly passed or added
  // to the implementation from traits objects:
  ::test(
    ArcImplementation<ExplicitArc>::MaxOutDegree == smtk::graph::unconstrained(),
    "Implementation has bad max-out-degree.");
  ::test(ArcImplementation<ExplicitArc>::MaxInDegree == 1, "Implementation has bad max-in-degree.");
  ::test(
    ArcImplementation<ExplicitArc>::MinOutDegree == 0, "Implementation has bad min-out-degree.");
  ::test(ArcImplementation<ExplicitArc>::MinInDegree == 0, "Implementation has bad min-in-degree.");

  ::test(
    ArcImplementation<UndirectedSelfArc>::MaxOutDegree == 2,
    "Implementation has bad max-out-degree.");
  ::test(
    ArcImplementation<UndirectedSelfArc>::MaxInDegree == 2,
    "Implementation has bad max-in-degree.");
  ::test(
    ArcImplementation<UndirectedSelfArc>::MinOutDegree == 2,
    "Implementation has bad min-out-degree.");
  ::test(
    ArcImplementation<UndirectedSelfArc>::MinInDegree == 2,
    "Implementation has bad min-in-degree.");
  std::cout << "\n";
}

// Test basic functionality of a simple example resource, including
// 1. A mutable, explicit, undirected arc
// 2. An immutable, implicit, directed, unidirectionally-indexed arc
// 3. An immutable, implicit, directed, bidirectionally-indexed arc
void testExampleResource()
{
  using namespace smtk::graph;
  std::cout << "Test example resource\n";
  // Create a resource and add a "plain" component (aa) and two comments (bb, cc).
  auto resource = smtk::graph::Resource<ExampleTraits>::create();
  auto aa = resource->create<Thingy>();
  auto bb = resource->create<Comment>();
  auto cc = resource->create<Comment>();

  std::cout << "  Resource " << resource.get() << " node a " << aa << " node b " << bb << " node c "
            << cc << "\n";
  // Create some helper functors:
  auto commentVisitor = [&aa](const Comment* dest) {
    std::cout << "      From " << aa.get() << " (" << aa->typeName() << ") to " << dest << " ("
              << dest->typeName() << ")\n";
  };
  auto componentVisitor1 = [&bb](const Component* dest) {
    std::cout << "      To " << bb.get() << " (" << bb->typeName() << ") from " << dest << " ("
              << dest->typeName() << ")\n";
  };
  auto componentVisitor2 = [&cc](const Component* dest) {
    std::cout << "      To " << cc.get() << " (" << cc->typeName() << ") from " << dest << " ("
              << dest->typeName() << ")\n";
  };

  //
  // Test explicit arcs:
  //
  // Test node(), front(), and empty() when an arc does not exist.
  ::test(aa->outgoing<ExplicitArc>().empty(), "Expected no arcs leaving aa.");
  ::test(!aa->outgoing<ExplicitArc>().front(), "Expected a null front() arc.");
  ::test(!aa->outgoing<ExplicitArc>().node(), "Expected a null node().");
  auto* exa = resource->arcs().at<ExplicitArc>();
  std::cout << "    Explicit arc object " << exa << " directed? "
            << (exa ? (exa->isDirected() ? "Y" : "N") : "null") << " mutable? "
            << (exa ? (exa->isMutable() ? "Y" : "N") : "null") << "\n";
  // Test that connecting arcs works:
  exa->connect(aa.get(), bb.get());
  // Test node(), front(), and empty() when an arc exists.
  ::test(!aa->outgoing<ExplicitArc>().empty(), "Expected an arc leaving aa.");
  ::test(aa->outgoing<ExplicitArc>().front() == bb.get(), "Expected front() to return bb.");
  ::test(aa->outgoing<ExplicitArc>().node() == bb.get(), "Expected node() to return bb.");
  // Test forward traversal (from the component to comment(s)):
  ::test(
    exa->outVisitor(aa.get(), commentVisitor) == smtk::common::Visited::All,
    "Expected to visit at least one comment.");
  ::test(
    exa->outVisitor(bb.get(), commentVisitor) == smtk::common::Visited::Empty,
    "Expected no comment on the comment; that would be too 'meta'.");
  // Test reverse traversal (from each comment to components, which includes non-self comments):
  ::test(
    exa->inVisitor(bb.get(), componentVisitor1) == smtk::common::Visited::All,
    "Expected to visit the component that is the subject of the comment.");
  ::test(
    aa->outgoing<ExplicitArc>().visit(commentVisitor) == smtk::common::Visited::All,
    "Expected to visit at least one comment.");
  ::test(aa->outgoing<ExplicitArc>().connect(cc.get()), "Expected to connect to another comment.");
  ::test(
    !aa->outgoing<ExplicitArc>().connect(cc.get()),
    "Expected not to connect to an already-connected comment.");
  ::test(
    aa->outgoing<ExplicitArc>().disconnect(cc.get()), "Expected to disconnect from a comment.");
  ::test(
    !aa->outgoing<ExplicitArc>().erase(cc.get()),
    "Expected not to disconnect from an unconnected comment.");
  ::test(
    bb->incoming<ExplicitArc>().visit(componentVisitor1) == smtk::common::Visited::All,
    "Expected to visit at least one component.");
  ::test(
    cc->incoming<ExplicitArc>().visit(componentVisitor2) == smtk::common::Visited::Empty,
    "Expected to visit no components.");
  ::test(aa->outgoing<ExplicitArc>().contains(bb.get()), "Expected an existing arc.");
  ::test(bb->incoming<ExplicitArc>().contains(aa.get()), "Expected an existing arc.");
  ::test(!aa->outgoing<ExplicitArc>().contains(cc.get()), "Expected no existing arc.");
  ::test(!cc->incoming<ExplicitArc>().contains(aa.get()), "Expected no existing arc.");

  //
  // Test implicit (forward-only) arcs
  //
  auto* ima = resource->arcs().at<ImplicitArc>();
  std::cout << "    Implicit arc object " << ima << " directed? "
            << (ima ? (ima->isDirected() ? "Y" : "N") : "null") << " mutable? "
            << (ima ? (ima->isMutable() ? "Y" : "N") : "null") << "\n";
  ::test(ImplicitArc::outVisitorCount() == 0, "Expected zero initial count.");
  ::test(
    ima->outVisitor(aa.get(), commentVisitor) == smtk::common::Visited::All,
    "Expected an implicit comment.");
  ::test(ImplicitArc::outVisitorCount() == 2, "Expected count of outVisitor calls to increase.");
  // Test that editing our implicit arcs does not work (but does compile).
  ::test(!ima->connect(aa.get(), bb.get()), "Expected editing an immutable graph to fail.");
  ::test(
    aa->outgoing<ImplicitArc>().contains(bb.get()), "Expected implicit arcs to contain test arc.");

#if defined(SMTK_FAILURE_INDEX) && SMTK_FAILURE_INDEX == 0
  // Attempt to build this to verify we cannot edit const-referenced nodes' arcs.
  // (It should trigger a static_assert.)
  const Component* aac = aa.get();
  aac->outgoing<ExplicitArc>().connect(cc.get());
#elif defined(SMTK_FAILURE_INDEX) && SMTK_FAILURE_INDEX == 1
  // Attempt to build this to verify we cannot access the "outgoing()"
  // endpoint of an arc from a node of the wrong type.
  bb->outgoing<ImplicitArc>().contains(aa.get());
#elif defined(SMTK_FAILURE_INDEX) && SMTK_FAILURE_INDEX == 2
  // Attempt to build this to verify we cannot access the "incoming()"
  // endpoint of a non-invertible arc.
  bb->incoming<ImplicitArc>().contains(aa.get());
#endif

  //
  // Test invertible (i.e., bidirectional) implicit arcs
  //
  auto* iima = resource->arcs().at<InvertibleImplicitArc>();
  std::cout << "    Invertible implicit arc object " << iima << " directed? "
            << (iima ? (iima->isDirected() ? "Y" : "N") : "null") << " mutable? "
            << (iima ? (iima->isMutable() ? "Y" : "N") : "null") << "\n";
  ::test(InvertibleImplicitArc::outVisitorCount() == 2, "Expected initial count of 2.");
  // Test forward traversal (from the component to comment(s)):
  ::test(
    iima->outVisitor(aa.get(), commentVisitor) == smtk::common::Visited::All,
    "Expected an implicit comment.");
  ::test(
    InvertibleImplicitArc::outVisitorCount() == 4,
    "Expected count of outVisitor calls to increase.");
  ::test(InvertibleImplicitArc::inVisitorCount() == 0, "Expected zero initial inverse count.");

  // Test reverse traversal (from each comment to components, which includes non-self comments):
  ::test(
    iima->inVisitor(bb.get(), componentVisitor1) == smtk::common::Visited::All,
    "Expected an implicit component.");
  ::test(
    InvertibleImplicitArc::inVisitorCount() == 1, "Expected count of inVisitor calls to increase.");

  ::test(
    iima->inVisitor(cc.get(), componentVisitor2) == smtk::common::Visited::All,
    "Expected an implicit component.");
  ::test(
    InvertibleImplicitArc::inVisitorCount() == 2,
    "Expected count of inVisitor calls to increase again.");
  ::test(
    aa->outgoing<InvertibleImplicitArc>().contains(bb.get()),
    "Expected invertible implicit arcs to contain test arc.");
  ::test(
    bb->incoming<InvertibleImplicitArc>().contains(aa.get()),
    "Expected invertible implicit arcs to contain inverse of test arc.");
  ::test(
    !bb->incoming<InvertibleImplicitArc>().contains(bb.get()),
    "Expected invertible implicit arcs not to contain inverse test arc.");

  std::cout << "Test lambda pass-by-value:\n";
  aa->outgoing<InvertibleImplicitArc>().visit([&aa](const Comment* dest) {
    std::cout << "      Inline aa " << aa.get() << " (" << aa->typeName() << ") to " << dest << " ("
              << dest->typeName() << ")\n";
  });
  // Test that editing our implicit arcs will not work (because InvertibleImplicitArc is immutable).
  ::test(!iima->connect(aa.get(), bb.get()), "Expected editing an immutable graph to fail.");

  std::cout << "Test out-degree, in-degree of implicit arcs with default computation\n";
  std::cout << "  ImplicitArc aa out-degree " << aa->outgoing<ImplicitArc>().degree() << "\n";
  std::cout << "  InvertibleImplicitArc aa out-degree "
            << aa->outgoing<InvertibleImplicitArc>().degree() << "\n";
  std::cout << "  InvertibleImplicitArc bb in-degree "
            << bb->incoming<InvertibleImplicitArc>().degree() << "\n";
  std::cout << "  InvertibleImplicitArc cc in-degree "
            << bb->incoming<InvertibleImplicitArc>().degree() << "\n";
  std::cout << "  Implicit arc outgoing "
            << "minDegree " << aa->outgoing<ImplicitArc>().minDegree() << " "
            << "maxDegree " << aa->outgoing<ImplicitArc>().maxDegree() << "\n";
  std::cout << "  Explicit arc outgoing "
            << "minDegree " << aa->outgoing<ExplicitArc>().minDegree() << " "
            << "maxDegree " << aa->outgoing<ExplicitArc>().maxDegree() << "\n";
  std::cout << "  Explicit arc incoming "
            << "minDegree " << bb->incoming<ExplicitArc>().minDegree() << " "
            << "maxDegree " << bb->incoming<ExplicitArc>().maxDegree() << "\n";
  ::test(
    aa->outgoing<ExplicitArc>().minDegree() ==
      smtk::graph::ArcImplementation<ExplicitArc>::MinOutDegree,
    "Bad explicit arc outgoing min-degree");
  ::test(
    aa->outgoing<ExplicitArc>().maxDegree() ==
      smtk::graph::ArcImplementation<ExplicitArc>::MaxOutDegree,
    "Bad explicit arc outgoing max-degree");
  ::test(
    bb->incoming<ExplicitArc>().minDegree() ==
      smtk::graph::ArcImplementation<ExplicitArc>::MinInDegree,
    "Bad explicit arc incoming min-degree");
  ::test(
    bb->incoming<ExplicitArc>().maxDegree() ==
      smtk::graph::ArcImplementation<ExplicitArc>::MaxInDegree,
    "Bad explicit arc incoming max-degree");

  // Test node(), front(), and empty() when an arc exists.
  const auto* front = bb->incoming<InvertibleImplicitArc>().front();
  const auto* fnode = bb->incoming<InvertibleImplicitArc>().node();
  std::cout << "Incoming bb (InvertibleImplicitArc) " << fnode << " ("
            << (fnode ? fnode->name() : "null") << ")\n";
  ::test(!bb->incoming<InvertibleImplicitArc>().empty(), "Expected an arc arriving at bb.");
  ::test(front == aa.get() || front == cc.get(), "Expected front() to return aa or cc.");
  ::test(fnode == aa.get() || fnode == cc.get(), "Expected node() to return aa or cc.");
}

// This tests a different explicit arc with non-castable node types for
// endpoints, to verify that ExplicitArcs properly compiles methods that work
// even when dynamic/static_cast fails (either by using reinterpret_cast or
// templated SFINAE methods).
void testExample2Resource()
{
  using namespace smtk::graph;
  std::cout << "Test example resource 2\n";

  // Create a resource and add a "plain" component (aa) and two comments (bb, cc).
  auto resource = smtk::graph::Resource<Example2Traits>::create();
  auto aa = resource->create<Notable>();
  auto bb = resource->create<Comment>();
  auto cc = resource->create<Comment>();

  std::cout << "  Resource " << resource.get() << " node a " << aa << " node b " << bb << " node c "
            << cc << "\n";
  // Create some helper functors:
  auto commentVisitor = [&aa](const Comment* dest) {
    std::cout << "      From " << aa.get() << " (" << aa->typeName() << ") to " << dest << " ("
              << dest->typeName() << ")\n";
  };
  auto componentVisitor1 = [&bb](const Component* dest) {
    std::cout << "      To " << bb.get() << " (" << bb->typeName() << ") from " << dest << " ("
              << dest->typeName() << ")\n";
  };
  auto componentVisitor2 = [&cc](const Component* dest) {
    std::cout << "      To " << cc.get() << " (" << cc->typeName() << ") from " << dest << " ("
              << dest->typeName() << ")\n";
  };

  //
  // Test explicit arcs:
  //
  auto* exa = resource->arcs().at<ExplicitArc>();
  std::cout << "    Explicit arc object " << exa << " directed? "
            << (exa ? (exa->isDirected() ? "Y" : "N") : "null") << " mutable? "
            << (exa ? (exa->isMutable() ? "Y" : "N") : "null") << "\n";
  // Test that connecting arcs works:
  exa->connect(aa.get(), bb.get());
  // Test forward traversal (from the component to comment(s)):
  ::test(
    exa->outVisitor(aa.get(), commentVisitor) == smtk::common::Visited::All,
    "Expected to visit at least one comment.");
  // Test reverse traversal (from each comment to components, which includes non-self comments):
  ::test(
    exa->inVisitor(bb.get(), componentVisitor1) == smtk::common::Visited::All,
    "Expected to visit the component that is the subject of the comment.");
  ::test(
    aa->outgoing<ExplicitArc>().visit(commentVisitor) == smtk::common::Visited::All,
    "Expected to visit at least one comment.");
  ::test(aa->outgoing<ExplicitArc>().connect(cc.get()), "Expected to connect to another comment.");
  ::test(
    !aa->outgoing<ExplicitArc>().connect(cc.get()),
    "Expected not to connect to an already-connected comment.");
  ::test(
    aa->outgoing<ExplicitArc>().disconnect(cc.get()), "Expected to disconnect from a comment.");
  ::test(
    !aa->outgoing<ExplicitArc>().erase(cc.get()),
    "Expected not to disconnect from an unconnected comment.");
  ::test(
    bb->incoming<ExplicitArc>().visit(componentVisitor1) == smtk::common::Visited::All,
    "Expected to visit at least one component.");
  ::test(
    cc->incoming<ExplicitArc>().visit(componentVisitor2) == smtk::common::Visited::Empty,
    "Expected to visit no components.");
  ::test(aa->outgoing<ExplicitArc>().contains(bb.get()), "Expected an existing arc.");
  ::test(bb->incoming<ExplicitArc>().contains(aa.get()), "Expected an existing arc.");
  ::test(!aa->outgoing<ExplicitArc>().contains(cc.get()), "Expected no existing arc.");
  ::test(!cc->incoming<ExplicitArc>().contains(aa.get()), "Expected no existing arc.");
}

struct DegreeDumper
{
  template<typename ResourceType>
  static void begin(const ResourceType*)
  {
  }

  template<typename Impl, typename ArcTraits = typename Impl::Traits, typename ResourceType>
  void operator()(const Impl* arcs, const ResourceType* resource) const
  {
    std::string arcType = smtk::common::typeName<ArcTraits>();
    smtk::string::Token arcToken(arcType);
    std::cout << "  " << arcType << "\n    outgoing degree\n";
    arcs->visitAllOutgoingNodes(resource, [](const typename ArcTraits::FromType* node) {
      std::cout << "      " << node->name() << ": " << node->template outgoing<ArcTraits>().degree()
                << "\n";
    });
    std::cout << "    incoming degree\n";
    arcs->visitAllIncomingNodes(resource, [](const typename ArcTraits::ToType* node) {
      std::cout << "      " << node->name() << ": " << node->template incoming<ArcTraits>().degree()
                << "\n";
    });
  }

  template<typename ResourceType>
  static void end(const ResourceType*)
  {
  }
};

// This tests several explicit arc options.
void testExplicitArcsVariations()
{
  using namespace smtk::graph;
  std::cout << "Test explicit arcs variations\n";

  // Create a resource and components of several types.
  auto resource = smtk::graph::Resource<Example3Traits>::create();
  auto a0 = resource->create<Thingy>();
  auto a1 = resource->create<Thingy>();
  auto a2 = resource->create<Thingy>();
  auto b0 = resource->create<Notable>();
  auto b1 = resource->create<Notable>();
  auto b2 = resource->create<Notable>();
  auto c0 = resource->create<Comment>();
  auto c1 = resource->create<Comment>();
  auto c2 = resource->create<Comment>();

  resource->setName("foober");
  a0->setName("a0");
  a1->setName("a1");
  a2->setName("a2");

  b0->setName("b0");
  b1->setName("b1");
  b2->setName("b2");

  c0->setName("c0");
  c1->setName("c1");
  c2->setName("c2");

  // Useful for debugging:
  std::cout << "  Resource " << resource.get() << "\n"
            << "    node a0 " << a0 << "    node a1 " << a1 << "    node a2 " << a2 << "\n"
            << "    node b0 " << b0 << "    node b1 " << b1 << "    node b2 " << b2 << "\n"
            << "    node c0 " << c0 << "    node c1 " << c1 << "    node c2 " << c2 << "\n";

  // Connect some nodes like so (note that
  // directed arcs have only one arrowhead
  // while undirected arcs have 2):
  //   a1    b2 b1
  //  ⤢  ⤡   ↓ ↙ ↓
  // a0  a2  c0 c2
  //     🡙
  //     b0
  //     ↓
  //     c1
  ::test(a0->outgoing<UndirectedSelfArc>().connect(a1), "Expected to connect a0→a1.");
  ::test(!a0->outgoing<UndirectedSelfArc>().connect(a1), "Expected not to connect a0→a1 twice.");
  ::test(!a1->outgoing<UndirectedSelfArc>().connect(a0), "Expected not to connect a1→a0.");
  ::test(a1->outgoing<UndirectedSelfArc>().connect(a2), "Expected to connect a1→a2.");
  ::test(a2->outgoing<UndirectedSelfArc>().connect(b0), "Expected to connect a2→b0.");
  // This should be disallowed due to the maximum-degree constraint:
  ::test(!a2->outgoing<UndirectedSelfArc>().connect(c0), "Expected not to connect a2→c0.");

  ::test(b0->outgoing<DirectedDistinctArc>().connect(c1), "Expected to connect b0→c1.");
  ::test(b1->outgoing<DirectedDistinctArc>().connect(c0), "Expected not to connect b1→c0.");
  ::test(b1->outgoing<DirectedDistinctArc>().connect(c2), "Expected to connect b1→c2.");
  ::test(b2->outgoing<DirectedDistinctArc>().connect(c0), "Expected to connect b2→c0.");
  ::test(!b2->outgoing<DirectedDistinctArc>().connect(c0), "Expected not to connect b2→c0 twice.");

  // Dump out-degree and in-degree of all connected nodes for each arc type:
  resource->evaluateArcs<DegreeDumper>();

  // Now verify that the degrees match the graph above.
  ::test(a0->outgoing<UndirectedSelfArc>().degree() == 1, "Expected a0's out-degree to be 1.");
  ::test(a0->incoming<UndirectedSelfArc>().degree() == 1, "Expected a0's in-degree to be 1.");
  ::test(a1->outgoing<UndirectedSelfArc>().degree() == 2, "Expected a1's out-degree to be 2.");
  ::test(a1->incoming<UndirectedSelfArc>().degree() == 2, "Expected a1's in-degree to be 2.");
  ::test(a2->outgoing<UndirectedSelfArc>().degree() == 2, "Expected a2's out-degree to be 2.");
  ::test(a2->incoming<UndirectedSelfArc>().degree() == 2, "Expected a2's in-degree to be 2.");

  ::test(b0->outgoing<UndirectedSelfArc>().degree() == 1, "Expected b0's out-degree to be 1.");
  ::test(b0->incoming<UndirectedSelfArc>().degree() == 1, "Expected b0's in-degree to be 1.");
  ::test(b1->outgoing<UndirectedSelfArc>().degree() == 0, "Expected b1's out-degree to be 0.");
  ::test(b1->incoming<UndirectedSelfArc>().degree() == 0, "Expected b1's in-degree to be 0.");
  ::test(b2->outgoing<UndirectedSelfArc>().degree() == 0, "Expected b2's out-degree to be 0.");
  ::test(b2->incoming<UndirectedSelfArc>().degree() == 0, "Expected b2's in-degree to be 0.");

  ::test(b0->outgoing<DirectedDistinctArc>().degree() == 1, "Expected b0's out-degree to be 1.");
  ::test(b1->outgoing<DirectedDistinctArc>().degree() == 2, "Expected b1's out-degree to be 2.");
  ::test(b2->outgoing<DirectedDistinctArc>().degree() == 1, "Expected b2's out-degree to be 1.");

  ::test(c0->incoming<DirectedDistinctArc>().degree() == 2, "Expected c0's in-degree to be 2.");
  ::test(c1->incoming<DirectedDistinctArc>().degree() == 1, "Expected c1's in-degree to be 1.");
  ::test(c2->incoming<DirectedDistinctArc>().degree() == 1, "Expected c2's in-degree to be 1.");

  // The UndirectedSelfArc specifies explicit min- and max- in- and out-degrees.
  // Test that the endpoint interface passes these non-default values properly:
  ::test(
    a0->outgoing<UndirectedSelfArc>().minDegree() ==
      smtk::graph::ArcImplementation<UndirectedSelfArc>::MinOutDegree,
    "Bad arc outgoing min-degree");
  ::test(
    a0->outgoing<UndirectedSelfArc>().maxDegree() ==
      smtk::graph::ArcImplementation<UndirectedSelfArc>::MaxOutDegree,
    "Bad arc outgoing max-degree");
  ::test(
    a0->incoming<UndirectedSelfArc>().minDegree() ==
      smtk::graph::ArcImplementation<UndirectedSelfArc>::MinInDegree,
    "Bad arc incoming min-degree");
  ::test(
    a0->incoming<UndirectedSelfArc>().maxDegree() ==
      smtk::graph::ArcImplementation<UndirectedSelfArc>::MaxInDegree,
    "Bad arc incoming max-degree");

  // For debugging: resource->dump("", "text/vnd.graphviz");
}

// Test that JSON serialization properly round-trips.
void testReadWrite()
{
  using namespace smtk::graph;
  std::cout << "Test json read/write\n";
  nlohmann::json j;
  nlohmann::json j2;

  // Create a resource and add a "plain" component (aa) and two comments (bb, cc).
  auto resource = smtk::graph::Resource<ExampleTraits>::create();
  auto aa = resource->create<Thingy>();
  auto bb = resource->create<Comment>();
  auto cc = resource->create<Comment>();
  aa->setName("aa");
  // Not setting name for "bb" to test things work without names.
  cc->setName("cc");

  std::cout << "  Resource " << resource.get() << "\n"
            << "    node a " << aa->id() << " " << aa->typeName() << "\n"
            << "    node b " << bb->id() << " " << bb->typeName() << "\n"
            << "    node c " << cc->id() << " " << cc->typeName() << "\n";

  auto aaArcs = aa->outgoing<ExplicitArc>();
  aaArcs.connect(bb.get());
  aaArcs.connect(cc.get());

  j = resource;

  std::cout << j.dump(2) << "\n";

  {
    std::cout << "Deserialize\n";
    auto resource2 = smtk::graph::Resource<ExampleTraits>::create();
    smtk::resource::json::Helper::pushInstance(resource2);
    resource2 = j;

    // Serialize again...
    j2 = resource2;
    std::cout << "  round trip json\n" << j2.dump(2) << "\n";
    // NB: We cannot just compare JSON objects as explicit arcs
    //     are unordered – the serialization order of destination
    //     nodes is not guaranteed and frequently changes. So...
    //     test that the UUIDs and arc structure match.
    ::test(resource->nodes().size() == resource2->nodes().size(), "Expect node counts to match.");
    const auto* aa2 = resource2->componentAs<Thingy>(aa->id());
    const auto* bb2 = resource2->componentAs<Comment>(bb->id());
    const auto* cc2 = resource2->componentAs<Comment>(cc->id());
    ::test(!!aa2, "Expected to find node aa2.");
    ::test(!!bb2, "Expected to find node bb2.");
    ::test(!!cc2, "Expected to find node cc2.");
    ::test(aa2->outgoing<ExplicitArc>().contains(bb2), "aa2 not connected to bb2.");
    ::test(aa2->outgoing<ExplicitArc>().contains(cc2), "aa2 not connected to bb2.");
    ::test(bb2->incoming<ExplicitArc>().contains(aa2), "bb2 not connected to aa2.");
    ::test(cc2->incoming<ExplicitArc>().contains(aa2), "cc2 not connected to aa2.");
  }
}

// Test that arc evaluators and graphviz output work.
void testDump()
{
  using namespace smtk::graph;
  std::cout << "Test arc dump\n";
  nlohmann::json j;

  // Create a resource and add a "plain" component (aa) and two comments (bb, cc).
  auto resource = smtk::graph::Resource<ExampleTraits>::create();
  resource->setName("arc dump test");
  auto aa = resource->create<Thingy>();
  auto bb = resource->create<Comment>();
  auto cc = resource->create<Comment>();
  aa->setName("aa");
  bb->setName("bb");
  cc->setName("cc");

  std::cout << "  Resource " << resource.get() << "\n"
            << "    node a " << aa->id() << " " << aa->typeName() << "\n"
            << "    node b " << bb->id() << " " << bb->typeName() << "\n"
            << "    node c " << cc->id() << " " << cc->typeName() << "\n";

  auto aaArcs = aa->outgoing<ExplicitArc>();
  aaArcs.connect(bb.get());
  aaArcs.connect(cc.get());

  resource->dump("", "text/plain");
  // resource->evaluateArcs<evaluators::Dump>(std::cout, evaluators::Dump("text/plain"));

  evaluators::Dump dot(
    "text/vnd.graphviz",
    /* whitelist */ {},
    /* blacklist */
    { smtk::common::typeName<ImplicitArc>(), smtk::common::typeName<InvertibleImplicitArc>() });
  resource->evaluateArcs<evaluators::Dump>(std::cout, dot);
}

// Test that node typenames are iterable.
void testNodeTypeNames()
{
  using namespace smtk::graph;
  std::cout << "Test node type-names\n";
  auto resource = smtk::graph::Resource<ExampleTraits>::create();
  // resource->setName("arc dump test");
  std::cout << "  Node types accepted by " << resource->typeName() << ":\n";
  const auto& nodeTypes = resource->nodeTypes();
  std::set<std::string> nodeTypeNames;
  std::set<std::string> expectedNodeTypes{ "Comment", "Thingy" };
  for (const auto& nodeType : nodeTypes)
  {
    nodeTypeNames.insert(nodeType.data());
    std::cout << "    \"" << nodeType.data() << "\"\n";
  }
  ::test(nodeTypeNames == expectedNodeTypes, "Node types do not match.");
}

} // namespace test
} // namespace smtk

int TestArcs(int, char*[])
{
  // Needed for log messages to appear in test output:
  smtk::io::Logger::instance().setFlushToStdout(true);

  smtk::test::testArcProperties();
  smtk::test::testExampleResource();
  smtk::test::testExample2Resource();
  smtk::test::testExplicitArcsVariations();
  smtk::test::testReadWrite();
  smtk::test::testDump();
  smtk::test::testNodeTypeNames();

  return 0;
}
