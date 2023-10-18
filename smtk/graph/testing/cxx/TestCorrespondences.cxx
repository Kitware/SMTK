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

#include "smtk/graph/Functions.h"

namespace smtk
{
namespace test
{

class CorrespondenceTraits
{
public:
  using NodeTypes = std::tuple<Thingy, Notable, Comment>;
  using ArcTypes = std::tuple<DirectedDistinctArc, UndirectedSelfArc, DirectedSelfArc>;
};

} // namespace test
} // namespace smtk

namespace
{

template<typename T>
bool checkCorrespondences(
  const std::set<std::pair<T*, T*>>& actual,
  const std::set<std::pair<T*, T*>>& expected,
  const std::string& msg)
{
  std::cout << msg << "\n";
  for (const auto& entry : actual)
  {
    std::cout << "  " << (entry.first ? entry.first->name() : "null") << ", "
              << (entry.second ? entry.second->name() : "null") << "\n";
  }
  std::cout << (actual == expected ? "Good match." : "ERROR: Mismatch!") << "\n";
  return actual == expected;
}

} // anonymous namespace

int TestCorrespondences(int, char*[])
{
  bool ok = true;
  using namespace smtk::test;
  using namespace smtk::graph;

  // DirectedDistinct: Notable → Comment
  // DirectedSelfArc: Thingy → Thingy
  // UndirectedSelfArc: Thingy — Thingy

  auto graph = smtk::graph::Resource<CorrespondenceTraits>::create();
  auto n1 = graph->create<Notable>();
  auto n2 = graph->create<Notable>();
  auto n3 = graph->create<Notable>();
  auto n4 = graph->create<Notable>();

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

  t1->outgoing<DirectedSelfArc>().connect(t3);
  t1->outgoing<DirectedSelfArc>().connect(t2);
  t2->outgoing<DirectedSelfArc>().connect(t3);
  t3->outgoing<DirectedSelfArc>().connect(t4);
  t4->outgoing<DirectedSelfArc>().connect(t1);

  t1->outgoing<UndirectedSelfArc>().connect(t2);
  t2->outgoing<UndirectedSelfArc>().connect(t3);
  t3->outgoing<UndirectedSelfArc>().connect(t4);
  t4->outgoing<UndirectedSelfArc>().connect(t1);
  // We would like to insert this, but the example arcs have a maximum degree of 2:
  // t2->outgoing<UndirectedSelfArc>().connect(t4);

  auto commentMatches =
    findArcCorrespondences<DirectedDistinctArc>(n1.get(), n2.get(), commentComparator);
  ok &= checkCorrespondences(
    commentMatches,
    { { nullptr, c3.get() }, { c1.get(), nullptr }, { c2.get(), c2.get() } },
    "Forward directed arc, distinct types. Matches are:");
  auto notableMatches =
    findArcCorrespondences<DirectedDistinctArc>(c1.get(), c2.get(), notableComparator);
  ok &= checkCorrespondences(
    notableMatches,
    { { nullptr, n2.get() }, { n1.get(), n1.get() } },
    "Reverse directed arc, distinct types. Matches are:");
  commentMatches =
    findArcCorrespondences<DirectedDistinctArc>(n3.get(), n4.get(), commentComparator);
  ok &= checkCorrespondences(
    commentMatches,
    { { nullptr, c4.get() }, { c3.get(), nullptr } },
    "Forward directed arc, distinct types. Matches are:");

  std::set<std::pair<Thingy*, Thingy*>> thingyMatches;
  thingyMatches = findArcCorrespondences<DirectedSelfArc>(t1.get(), t2.get(), thingyComparator);
  ok &= checkCorrespondences(
    thingyMatches,
    { { t2.get(), nullptr }, { t3.get(), t3.get() } },
    "Forward directed arc, equivalent types. Matches are:");
  thingyMatches = findArcCorrespondences<DirectedSelfArc, Thingy, smtk::graph::IncomingArc>(
    t3.get(), t4.get(), thingyComparator);
  ok &= checkCorrespondences(
    thingyMatches,
    { { t1.get(), nullptr }, { t2.get(), nullptr }, { nullptr, t3.get() } },
    "Reverse directed arc, equivalent types. Matches are:");

  thingyMatches = findArcCorrespondences<UndirectedSelfArc>(t2.get(), t4.get(), thingyComparator);
  ok &= checkCorrespondences(
    thingyMatches,
    { { t1.get(), t1.get() }, { t3.get(), t3.get() } },
    "Undirected arc, equivalent types. Matches are:");
  thingyMatches = findArcCorrespondences<UndirectedSelfArc>(t1.get(), t3.get(), thingyComparator);
  ok &= checkCorrespondences(
    thingyMatches,
    { { t2.get(), t2.get() }, { t4.get(), t4.get() } },
    "Undirected arc, equivalent types. Matches are:");

  // graph->dump("");
  return ok ? 0 : 1;
}
