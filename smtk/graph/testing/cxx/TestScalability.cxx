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

#include <chrono>
#include <iostream>

template<class Duration>
struct duration_units;

template<class Rep>
struct duration_units<std::chrono::duration<Rep, std::ratio<1, 1>>>
{
  static std::string str() { return "s"; }
};

template<class Rep>
struct duration_units<std::chrono::duration<Rep, std::ratio<1, 1000>>>
{
  static std::string str() { return "ms"; }
};

template<class Rep>
struct duration_units<std::chrono::duration<Rep, std::ratio<1, 1000000>>>
{
  static std::string str() { return "micro-s"; }
};

template<class Rep>
struct duration_units<std::chrono::duration<Rep, std::ratio<1, 1000000000>>>
{
  static std::string str() { return "ns"; }
};

template<class Duration, class Clock = std::chrono::high_resolution_clock>
struct Timer
{
public:
  using clock = Clock;
  using duration = Duration;

  void tic() { _tic = clock::now(); }

  void toc() { _toc = clock::now(); }

  typename duration::rep elapsed() const { return this->elapsed(duration()); }

  template<class D>
  typename D::rep elapsed(const D&) const
  {
    return std::chrono::duration_cast<D>(_toc - _tic).count();
  }

  std::string units() const { return duration_units<duration>::str(); }

private:
  typename clock::time_point _tic, _toc;
};

class Node : public smtk::graph::Component
{
public:
  Node(const std::shared_ptr<smtk::graph::ResourceBase>& resource)
    : smtk::graph::Component(resource)
  {
  }
};

class Adjacent
{
public:
  using FromType = Node;
  using ToType = Node;
  using Directed = std::true_type;
};

struct AdjacencyTraits
{
  typedef std::tuple<Node> NodeTypes;
  typedef std::tuple<Adjacent> ArcTypes;
};

int TestScalability(int argc, char* argv[])
{
  Timer<std::chrono::milliseconds> timer;

  // Default to 5 nodes with a degree of 2 for each
  int num_node = 5, degree_node = 2;
  // Parse CLI arguments
  int shift = 0;
  for (int ii = 1; ii < argc; ++ii)
  {
    if ((!strcmp(argv[ii], "-n") || !strcmp(argv[ii], "--num-node")) && ii + 1 < argc)
    {
      shift += 2;
      num_node = std::stoi(argv[ii + 1]);
    }
    if ((!strcmp(argv[ii], "-d") || !strcmp(argv[ii], "--degree-per-node")) && ii + 1 < argc)
    {
      shift += 2;
      degree_node = std::stoi(argv[ii + 1]);
    }
    else
    {
      argv[ii - shift] = argv[ii];
    }
  }
  argc -= shift;
  if (argc > 1)
  {
    std::cerr << "Unrecognized arguments... \n";
    for (int ii = 1; ii < argc; ++ii)
    {
      std::cerr << argv[ii] << " ";
    }
    return 1;
  }

  if (degree_node <= 0)
  {
    std::cerr << "The degree for the nodes must be greater than 0\n";
  }
  if (num_node < degree_node)
  {
    std::cerr << "The degree of the nodes must be less than the number of nodes. \n"
              << "The max degree for " << num_node << " nodes is " << num_node - 1 << " degrees\n";
    return 1;
  }

  timer.tic();
  auto resource = smtk::graph::Resource<AdjacencyTraits>::create();

  std::vector<std::shared_ptr<Node>> nodes(num_node);
  for (int ii = 0; ii < num_node; ii++)
  {
    nodes[ii] = resource->create<Node>();
  }
  timer.toc();
  std::cout << "Created " << num_node << " node(s) in " << timer.elapsed() << "(" << timer.units()
            << ")\n";

  timer.tic();
  for (int ii = 0; ii < num_node; ii++)
  {
    auto adjacentNodes = nodes[ii]->outgoing<Adjacent>();
    for (int jj = 0; jj < degree_node; jj++)
    {
      adjacentNodes.connect(nodes[(ii + jj + 1) % num_node].get());
    }
  }
  timer.toc();
  std::cout << "Created " << num_node * degree_node << " arc(s) in " << timer.elapsed() << "("
            << timer.units() << ")\n";

  timer.tic();
  volatile smtk::common::Visit nooptimize = smtk::common::Visit::Continue;
  for (const auto& node : resource->nodes())
  {
    std::dynamic_pointer_cast<Node>(node)->outgoing<Adjacent>().visit(
      [&](const Node * /* to */) -> smtk::common::Visit { return nooptimize; });
  }
  timer.toc();
  std::cout << "Visited " << num_node * degree_node << " arc(s) in " << timer.elapsed() << "("
            << timer.units() << ")\n";

  std::cout << std::endl;
  // Remove half of the nodes
  timer.tic();
  int k = 0;
  for (int ii = 0; ii < num_node; ii += 2, k++)
  {
    nodes[ii]->outgoing<Adjacent>().disconnect(nullptr); // Remove all arcs attached to nodes[ii].
    resource->remove(nodes[ii]);                         // Then remove nodes[ii].
  }
  timer.toc();
  std::cout << "Removed " << k << " node(s) in " << timer.elapsed() << "(" << timer.units()
            << ")\n";

  nodes.resize(0);
  nodes.shrink_to_fit();

  timer.tic();
  int narc = 0;
  for (const auto& node : resource->nodes())
  {
    std::dynamic_pointer_cast<Node>(node)->outgoing<Adjacent>().visit(
      [&](const Node * /* to */) -> smtk::common::Visit {
        narc++;
        return nooptimize;
      });
  }
  timer.toc();
  std::cout << "Visited " << narc << " arc(s) in " << timer.elapsed() << "(" << timer.units()
            << ")\n";

  timer.tic();
  narc = 0;
  for (const auto& node : resource->nodes())
  {
    std::dynamic_pointer_cast<Node>(node)->outgoing<Adjacent>().visit(
      [&](const Node * /* to */) -> smtk::common::Visit {
        narc++;
        return nooptimize;
      });
  }
  timer.toc();
  std::cout << "Visited " << narc << " arc(s) in " << timer.elapsed() << "(ms)\n";

  return 0;
}
