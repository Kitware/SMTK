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

#include "smtk/graph/RuntimeArc.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/resource/Properties.h"

#include "smtk/graph/json/jsonResource.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace test
{

class RuntimeTraits
{
public:
  using NodeTypes = std::tuple<Thingy, Notable, Comment>;
  using ArcTypes = std::tuple<
    DirectedDistinctArc,
    UndirectedSelfArc,
    smtk::graph::RuntimeArc<smtk::graph::IsDirected>,
    smtk::graph::RuntimeArc<smtk::graph::IsUndirected>>;
};

void to_json(json& jj, const smtk::test::Thingy* component);

template<typename NodeType>
void SMTK_ALWAYS_EXPORT from_json(const json& j, std::shared_ptr<NodeType>& node)
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

} // namespace test
} // namespace smtk

namespace
{

} // anonymous namespace

int TestRuntimeJSON(int, char*[])
{
  bool ok = true;
  using namespace smtk::test;
  using namespace smtk::graph;

  // We need to create an entry in the string-token manager for this type-name
  // so that it exists when referenced by DeleteArc::registerDeleter():
  smtk::string::Token dummy("smtk::graph::ResourceBase");

  // DirectedDistinct: Notable → Comment
  // DirectedSelfArc: Thingy → Thingy
  // UndirectedSelfArc: Thingy — Thingy
  // RuntimeDirected: Notable['foo'{int}=7] → Notable['bar'{int}=12]
  // RuntimeUndirected: Thingy — Thingy

  auto graph = smtk::graph::Resource<RuntimeTraits>::create();
  auto n1 = graph->create<Notable>();
  auto n2 = graph->create<Notable>();
  auto n3 = graph->create<Notable>();
  auto n4 = graph->create<Notable>();
  n1->properties().get<long>()["foo"] = 7;
  n2->properties().get<long>()["bar"] = 12;
  n3->properties().get<long>()["foo"] = 7;
  n4->properties().get<long>()["bar"] = 12;

  auto c1 = graph->create<Comment>();
  auto c2 = graph->create<Comment>();
  auto c3 = graph->create<Comment>();
  auto c4 = graph->create<Comment>();

  auto t1 = graph->create<Thingy>();
  auto t2 = graph->create<Thingy>();
  auto t3 = graph->create<Thingy>();
  auto t4 = graph->create<Thingy>();

  n1->setName("n1");
  n2->setName("n2");
  n3->setName("n3");
  n4->setName("n4");

  c1->setName("c1");
  c2->setName("c2");
  c3->setName("c3");
  c4->setName("c4");

  t1->setName("t1");
  t2->setName("t2");
  t3->setName("t3");
  t4->setName("t4");

  std::function<bool(Comment*, Comment*)> commentComparator = [](Comment* aa, Comment* bb) -> bool {
    return aa == bb;
  };
  std::function<bool(Notable*, Notable*)> notableComparator = [](Notable* aa, Notable* bb) -> bool {
    return aa == bb;
  };
  std::function<bool(Thingy*, Thingy*)> thingyComparator = [](Thingy* aa, Thingy* bb) -> bool {
    return aa == bb;
  };

  n1->outgoing<DirectedDistinctArc>().connect(c1);
  n1->outgoing<DirectedDistinctArc>().connect(c2);
  n2->outgoing<DirectedDistinctArc>().connect(c2);
  n2->outgoing<DirectedDistinctArc>().connect(c3);
  n3->outgoing<DirectedDistinctArc>().connect(c3);
  n4->outgoing<DirectedDistinctArc>().connect(c4);

  t1->outgoing<UndirectedSelfArc>().connect(t2);
  t2->outgoing<UndirectedSelfArc>().connect(t3);
  t3->outgoing<UndirectedSelfArc>().connect(t4);
  t4->outgoing<UndirectedSelfArc>().connect(t1);

  bool didAdd;
  didAdd = graph->arcs().insertRuntimeArcType(
    graph.get(),
    "RuntimeDirected",
    { { "Notable [long{'foo' = 7}]" } },
    { { "Notable [long{'bar' = 12}]" } },
    smtk::graph::IsDirected);
  ::test(didAdd, "Could not add directed runtime arc type.");

  didAdd = n1->outgoing("RuntimeDirected").connect(n2);
  ::test(didAdd, "Could not add directed runtime arc n1→n2.");

  didAdd = n1->outgoing("RuntimeDirected").connect(n3);
  ::test(!didAdd, "Should not have added directed runtime arc n1→n3.");

  nlohmann::json data = graph;
  {
    std::cout << data.dump(2) << "\n";
    std::shared_ptr<smtk::graph::Resource<RuntimeTraits>> graphB = data;
    auto* n1b = graphB->componentAs<Notable>(n1->id());
    auto* n2b = graphB->componentAs<Notable>(n2->id());
    ::test(!!n1b, "Expected to find n1 companion in copy of graph.");
    ::test(!!n2b, "Expected to find n2 companion in copy of graph.");
    ::test(
      n1b->outgoing("RuntimeDirected").contains(n2b),
      "Expected to find run-time arc n1→n2 preserved during copy.");
  }

  // graph->dump("");
  return ok ? 0 : 1;
}
