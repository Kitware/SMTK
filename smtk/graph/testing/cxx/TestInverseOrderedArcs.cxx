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

class Vertex : public Node
{
  using Node::Node;
};

class Face : public Node
{
  using Node::Node;

  template<typename... T, typename = smtk::graph::detail::CompatibleTypes<Vertex, T...>>
  void initialize(std::string /*name*/, int index, T&&... verts);
};

///////////////////////////////////////////////////////////////////////////////
class Loop;
class Faces;

class Loop : public smtk::graph::OrderedArcs<Face, Vertex, Loop>
{
public:
  using smtk::graph::OrderedArcs<Face, Vertex, Loop>::OrderedArcs;
};

class Faces : public smtk::graph::OrderedArcs<Vertex, Face, Faces>
{
public:
  using smtk::graph::OrderedArcs<Vertex, Face, Faces>::OrderedArcs;
};

template<typename... T, typename>
void Face::initialize(std::string name, int index, T&&... verts)
{
  Node::initialize(name, index);
  this->get<Loop>().insert({ verts... });
}

///////////////////////////////////////////////////////////////////////////////
/// Graph Traits
struct OrderedInverseArcsTraits
{
  using NodeTypes = std::tuple<Vertex, Face>;
  using ArcTypes = std::tuple<Loop, Faces>;
};
///////////////////////////////////////////////////////////////////////////////
} // namespace

namespace smtk
{
namespace graph
{

template<>
class Inverse<Loop>
{
  using API = Faces::template API<Faces>;

public:
  static bool insert(const Vertex& v, Face& f) { return API().get(v).insert_back(f, false).second; }
  static std::size_t erase(const Vertex& v, const Face& f) { return API().get(v).erase(f, false); }
};

template<>
class Inverse<Faces>
{
  using API = Loop::template API<Loop>;

public:
  static bool insert(const Face&, Vertex&)
  {
    throw "";
    return false;
  }
  static std::size_t erase(const Face& f, const Vertex& v) { return API().get(f).erase(v, false); }
};

} // namespace graph
} // namespace smtk

/// Test inserting arc types that have inverses.
/// TODO (ryan.krattiger) Add erase and assignment cases for each set of arc types.
int TestInverseOrderedArcs(int, char*[])
{
  auto graph = smtk::graph::Resource<OrderedInverseArcsTraits>::create();

  /*
   *         * v3
   *       / | \
   *      /  |  \
   * v1 *    |    * v4
   *      \  |  /
   *       \ | /
   *         *v2
   */

  auto v1 = graph->create<Vertex>("vertex1", 0);
  auto v2 = graph->create<Vertex>("vertex2", 1);
  auto v3 = graph->create<Vertex>("vertex3", 2);
  auto v4 = graph->create<Vertex>("vertex4", 3);

  auto f1 = graph->create<Face>("face1", 0, v1, v2, v3);
  auto f2 = graph->create<Face>("face2", 1, v4, v3, v2);

  FINISH_TEST();
}
