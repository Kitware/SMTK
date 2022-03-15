//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/graph/Resource.h"

#include "smtk/graph/arcs/Arc.h"
#include "smtk/graph/arcs/Arcs.h"
#include "smtk/graph/arcs/OrderedArcs.h"

#include <iostream>

/// To Demonstrate the API of SMTK's graph resource, this test constructs a toy
/// resource designed to describe 2-dimensional planar polygons. The resource
/// models Vertex, Edge and Face nodes. The defined arc types facilitate the
/// following queries:
///
/// vertex->get<Edges>();  // access the edges that contain this vertex
///
/// edge->get<Vertices>(); // access the vertices that comprise this edge
/// edge->get<Faces>();    // access the faces that contain this edge
///
/// face->get<Loop>();     // access the edges that comprise this face

namespace
{
// Vertex inherits from smtk::graph::Component and accepts a 2-dimensional
// position on construction.
class Vertex : public smtk::graph::Component
{
public:
  Vertex(const std::shared_ptr<smtk::graph::ResourceBase>& resource)
    : Component(resource)
  {
  }

  void initialize(double x, double y)
  {
    m_x = x;
    m_y = y;
  }

  const double& x() const { return m_x; }
  const double& y() const { return m_y; }

private:
  double m_x;
  double m_y;
};

// Edge inherits from smtk::graph::Component and accepts two vertices on
// construction.
class Edge : public smtk::graph::Component
{
public:
  Edge(const std::shared_ptr<smtk::graph::ResourceBase>& resource);

  void initialize(Vertex& v1, Vertex& v2);
};

// Face inherits from smtk::graph::Component and accepts an arbitrary number of
// edges on construction.
class Face : public smtk::graph::Component
{
public:
  Face(const std::shared_ptr<smtk::graph::ResourceBase>& resource);

  template<typename... T, typename = smtk::graph::detail::CompatibleTypes<Edge, T...>>
  void initialize(T&&... edges);

  // An example for data access.
  std::array<double, 2> centroid() const;
};

// Vertices describes a connection between and Edge and the Vertices that
// comprise it. It inherits from smtk::graph::OrderedArcs, since we need the
// ordering of the vertices to be defined by the user. The first template
// parameter describes the "from" type, and the second template parameter
// describes the "to" type. The underlying datastructure is a vector.
class Vertices : public smtk::graph::OrderedArcs<Edge, Vertex>
{
public:
  // We expose the parent class's constructor, which accepts vertices as
  // explicitly listed, as an iterator range or from an iterable collection.
  using smtk::graph::OrderedArcs<Edge, Vertex>::OrderedArcs;
};

// Edges describes a connection between a Vertex and the edges that contain it.
// The underlying datastructure is an unordered set.
class Edges : public smtk::graph::Arcs<Vertex, Edge>
{
public:
  // We expose the parent class's constructor, which accepts edges as explicitly
  // listed, as an iterator range or from an iterable collection.
  using smtk::graph::Arcs<Vertex, Edge>::Arcs;
};

// Loop describes a connection between a face and the edges that define
// it.
class Loop : public smtk::graph::OrderedArcs<Face, Edge>
{
public:
  using smtk::graph::OrderedArcs<Face, Edge>::OrderedArcs;
};

// Faces describes a connection between an edge and the faces that contain it.
class Faces : public smtk::graph::Arcs<Edge, Face>
{
public:
  using smtk::graph::Arcs<Edge, Face>::Arcs;
};

// Our model resource consists of the following node and arc types. These two
// tuples must be explicitly expressed at compile-time.
struct PlanarTraits
{
  typedef std::tuple<Vertex, Edge, Face> NodeTypes;
  typedef std::tuple<Vertices, Edges, Loop, Faces> ArcTypes;
};

// The constructor for our edge sets up the relationships between the newly
// created edge and the vertices that comprise it. Since this modeling session
// is for planar polygons, an edge is comprised of two vertices.
Edge::Edge(const std::shared_ptr<smtk::graph::ResourceBase>& resource)
  : Component(resource)
{
}

void Edge::initialize(Vertex& v1, Vertex& v2)
{
  // Create a Vertices arc that connects this edge to its two vertices.
  // Alternatively, we could have called the following:
  // ```
  // this->get<Vertices>().push_back(v1);
  // this->get<Vertices>().push_back(v2);
  // ```
  this->set<Vertices>(v1, v2);

  // Add the created edge to the vertices' edge list. Note that
  // const-correctness is observed (if we had a reference to a const Vertex&, we
  // would only be able to perform const queries on its edges).
  v1.get<Edges>().insert(*this);
  v2.get<Edges>().insert(*this);
}

// Similarly, the constructor for our face sets up the relationships between the
// newly created face and the edges that comprise it.
Face::Face(const std::shared_ptr<smtk::graph::ResourceBase>& resource)
  : Component(resource)
{
}

template<typename... T, typename>
void Face::initialize(T&&... edges)
{
  // Access our resource in its derived form, so we can take advantage of the
  // compile-time grammar checking we have put in place.
  auto planarResource =
    std::static_pointer_cast<smtk::graph::Resource<PlanarTraits>>(this->resource());

  // Create a Loop that connects this face to its edges.
  // Alternatively, we could have called the following:
  // ```
  // this->set<Loop>(std::forward<T>(edges)...);
  // ```
  planarResource->create<Loop>(*this, std::forward<T>(edges)...);

  this->visit<Loop>([&](Edge& edge) -> bool {
    edge.get<Faces>().insert(*this);
    return false;
  });
}

// As an example of the API, this is how to compute the centroid of a face.
std::array<double, 2> Face::centroid() const
{
  std::array<double, 2> c;

  // We can directly iterate over the face's edges with no allocation overhead.
  // Because the edges are stored as std::reference_wrappers, we must explicitly
  // state that our element is a 'const Edge&' (using 'auto&' returns the
  // wrapped element, which then needs to be unwrapped).
  for (const Edge& edge : this->get<Loop>())
  {
    // Similarly, we can acess each edge's vertices.
    const Vertex& vertex = edge.get<Vertices>().to().at(0);
    c[0] += vertex.x();
    c[1] += vertex.y();
  }
  for (auto& coordinate : c)
  {
    // We can perform all of the const calls we like on the arcs we can access,
    // since the method is marked const. To modify the arc information, the
    // containing method would need to be non-const.
    coordinate /= this->get<Loop>().count();
  }
  return c;
}
} // namespace

int TestPlanarResource(int, char*[])
{
  // Construct our planar-graph resource.
  auto resource = smtk::graph::Resource<PlanarTraits>::create();

  // Construct three vertices with coordinates.
  auto vertex1 = resource->create<Vertex>(0, 0);
  auto vertex2 = resource->create<Vertex>(0, 1);
  auto vertex3 = resource->create<Vertex>(1, 0);

  // Construct three edges from these three vertices.
  auto edge1 = resource->create<Edge>(*vertex1, *vertex2);
  auto edge2 = resource->create<Edge>(*vertex2, *vertex3);
  auto edge3 = resource->create<Edge>(*vertex3, *vertex1);

  // Construct a face from the three edges.
  auto face1 = resource->create<Face>(*edge1, *edge2, *edge3);

  // For convenience, store these elements in arrays.
  std::array<const Vertex*, 3> vertices{ vertex1.get(), vertex2.get(), vertex3.get() };
  std::array<const Edge*, 3> edges{ edge1.get(), edge2.get(), edge3.get() };

  // Print the resulting connectivity information.
  std::cout << std::endl;
  for (auto i = 0; i < 3; i++)
  {
    std::cout << "vertex " << (i + 1) << ": " << vertices[i]->id() << " ( " << vertices[i]->x()
              << ", " << vertices[i]->y() << " )" << std::endl;
    std::cout << "  edges: (";
    for (const Edge& edge : vertices[i]->get<Edges>())
    {
      std::cout << " " << edge.id();
    }
    std::cout << " )" << std::endl;
  }
  std::cout << std::endl;
  for (auto i = 0; i < 3; i++)
  {
    std::cout << "edge " << (i + 1) << ": " << edges[i]->id() << std::endl;
    std::cout << "  vertices: (";
    for (const Vertex& vertex : edges[i]->get<Vertices>())
    {
      std::cout << " " << vertex.id();
    }
    std::cout << " )" << std::endl;
    std::cout << "  faces: (";
    for (const Face& face : edges[i]->get<Faces>())
    {
      std::cout << " " << face.id();
    }
    std::cout << " )" << std::endl;
  }
  std::cout << std::endl;
  std::cout << "face: " << face1->id() << std::endl;
  std::cout << "  edges: (";
  for (const Edge& edge : face1->get<Loop>())
  {
    std::cout << " " << edge.id();
  }
  std::cout << " )" << std::endl;
  auto centroid = face1->centroid();
  std::cout << "  centroid: ( " << centroid[0] << ", " << centroid[1] << " )" << std::endl;
  std::cout << std::endl;

  return 0;
}
