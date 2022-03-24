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

#include "smtk/common/Visit.h"

#include <memory>
#include <set>

namespace smtk
{
namespace test
{

// ## Node types ##
//
// Define some dummy node types for a graph resource.
// These nodes hold a string name to make dumps more
// informative (than UUIDs) and include two subclasses
// that cannot be cast to one another to test the
// ExplicitArcs implementation:
//
//           Thingy
//           ↑    ↑
//      Comment  Notable
//
// Because std::is_same<Comment,Notable> evaluates to
// std::false_type, undirected arcs between the two
// will not need to check both lookup structures, forcing
// different code branches to be tested than when the
// two arc endpoint types are interchangeable.

// ++ 6 ++
class Thingy : public smtk::graph::Component
{
public:
  smtkTypeMacro(Thingy);
  smtkSuperclassMacro(smtk::graph::Component);

  using Serialize = std::true_type; // Mark this node for JSON serialization.
  // -- 6 --
  Thingy(const std::shared_ptr<smtk::graph::ResourceBase>& parent)
    : smtk::graph::Component(parent)
  {
  }

  Thingy(const std::shared_ptr<smtk::graph::ResourceBase>& parent, const smtk::common::UUID& uid)
    : smtk::graph::Component(parent, uid)
  {
  }

  virtual void setName(const std::string& name) { m_name = name; }
  std::string name() const override { return m_name; }

protected:
  std::string m_name;
};

class Comment : public Thingy
{
public:
  smtkTypeMacro(Comment);
  smtkSuperclassMacro(Thingy);

  Comment(const std::shared_ptr<smtk::graph::ResourceBase>& parent)
    : Thingy(parent)
  {
  }

  Comment(const std::shared_ptr<smtk::graph::ResourceBase>& parent, const smtk::common::UUID& uid)
    : Thingy(parent, uid)
  {
  }
};

class Notable : public Thingy
{
public:
  smtkTypeMacro(Notable);
  smtkSuperclassMacro(Thingy);

  Notable(const std::shared_ptr<smtk::graph::ResourceBase>& parent)
    : Thingy(parent)
  {
  }

  Notable(const std::shared_ptr<smtk::graph::ResourceBase>& parent, const smtk::common::UUID& uid)
    : Thingy(parent, uid)
  {
  }
};

// ## Arc types ##
//
// Declare a variety of arcs using the node types above
// for testing different aspects of the graph resource.

/**\brief Example of an implicit arc class.
  *
  * Arcs of this type are not stored directly but generated as requested.
  * In particular, this class will present every component in the resource
  * that is of the proper type as connected (excepting self-loops) making
  * the resulting graph fully connected (excepting self-loops, and noting
  * that it is fully connected only in the multipartite sense – meaning
  * that arcs connect all nodes of the proper types together).
  */
class ImplicitArc
{
public:
  using FromType = Thingy;
  using ToType = Comment;
  using Directed = std::true_type;
  using ForwardIndexOnly = std::true_type;
  // Components may reference any number of comments:
  static constexpr std::size_t MaxOutDegree = smtk::graph::unconstrained();
  static constexpr std::size_t MaxInDegree = 1; // Comments may apply to at most one component.

  // ++ 3 ++
  template<typename Functor>
  smtk::common::Visited outVisitor(const Thingy* component, Functor ff) const
  {
    auto resource = component ? component->resource() : smtk::resource::Resource::Ptr();
    if (!component || !resource)
    {
      return smtk::common::Visited::Empty;
    }
    smtk::common::VisitorFunctor<Functor> visitor(ff);
    // Implicitly generate fully connected graph (except for self-connections
    // which are impossible given the FromType and ToType):
    std::string filter = ToType::type_name;
    auto others = resource->filterAs<std::set<std::shared_ptr<ToType>>>(filter);
    for (const auto& other : others)
    {
      ++s_outVisitorCount;
      if (other && visitor(other.get()) == smtk::common::Visit::Halt)
      {
        return smtk::common::Visited::Some;
      }
    }
    return smtk::common::Visited::All;
  }
  // -- 3 --

  bool contains(const Thingy* from, const Comment* to) const
  {
    return !!from && !!to && from != to;
  }

  static int outVisitorCount() { return s_outVisitorCount; }

protected:
  static int s_outVisitorCount;
};

/**\brief An example implicit arc that allows bidirectional traversal.
  *
  * This class simply makes the ImplicitArc above a fully-connected
  * graph in the reverse sense (all arcs from ToType to FromType exist
  * as well as all arcs from FromType to ToType).
  */
class InvertibleImplicitArc : public ImplicitArc
{
public:
  using FromType = typename ImplicitArc::FromType;
  using ToType = typename ImplicitArc::ToType;
  using Directed = typename ImplicitArc::Directed;
  using ForwardIndexOnly = std::false_type;
  // Components may reference any number of comments.
  static constexpr std::size_t MaxOutDegree = smtk::graph::unconstrained();
  static constexpr std::size_t MaxInDegree = 1; // Comments may apply to at most one component.
  using ImplicitArc::contains;
  using ImplicitArc::outVisitor;
  // using ImplicitArc::connect;

  template<typename Functor>
  smtk::common::Visited inVisitor(const Comment* comment, Functor ff) const
  {
    auto resource = comment ? comment->resource() : smtk::resource::Resource::Ptr();
    if (!comment || !resource)
    {
      return smtk::common::Visited::Empty;
    }
    smtk::common::VisitorFunctor<Functor> visitor(ff);
    // Implicitly generate fully connected graph except for self-connections:
    std::string filter = FromType::type_name;
    auto others = resource->filterAs<std::set<std::shared_ptr<FromType>>>(filter);
    for (const auto& other : others)
    {
      if (other && other.get() != comment)
      {
        ++s_inVisitorCount;
        if (visitor(other.get()) == smtk::common::Visit::Halt)
        {
          return smtk::common::Visited::Some;
        }
      }
    }
    return smtk::common::Visited::All;
  }

  static int inVisitorCount() { return s_inVisitorCount; }

protected:
  static int s_inVisitorCount;
};

// ++ 2 ++
class ExplicitArc
{
public:
  using FromType = Thingy;
  using ToType = Comment;
  using Directed = std::false_type;
  // using ForwardIndexOnly = std::false_type; // false is the assumed default.
  // MaxOutDegree is implicitly smtk::graph::unconstrained().
  static constexpr std::size_t MaxInDegree = 1; // Comments may apply to at most one component.
};
// -- 2 --

class DirectedDistinctArc
{
public:
  using FromType = Notable;
  using ToType = Comment;
  using Directed = std::true_type;
  // Omit MaxOutDegree, MaxInDegree to verify that they default to unconstrained().
};

class UndirectedSelfArc
{
public:
  using FromType = Thingy;
  using ToType = Thingy;
  using Directed = std::false_type;
  static constexpr std::size_t MaxOutDegree = 2; // Thingy objects may have at most 2 outgoing arcs.
  static constexpr std::size_t MaxInDegree = 2;  // Thingy objects may have at most 2 incoming arcs.
  static constexpr std::size_t MinOutDegree = 2; // ... and should have at least 2 outgoing arcs.
  static constexpr std::size_t MinInDegree = 2;  // ... and should have at least 2 incoming arcs.
};

class DirectedSelfArc
{
public:
  using FromType = Thingy;
  using ToType = Thingy;
  using Directed = std::true_type;
  static constexpr std::size_t MaxOutDegree = 2; // Thingy objects may have at most 2 outgoing arcs.
  static constexpr std::size_t MaxInDegree = 2;  // Thingy objects may have at most 2 incoming arcs.
};

// ++ 1 ++
class ExampleTraits
{
public:
  using NodeTypes = std::tuple<Thingy, Comment>;
  using ArcTypes = std::tuple<InvertibleImplicitArc, ImplicitArc, ExplicitArc>;
};
// -- 1 --

class Example2Traits
{
public:
  using NodeTypes = std::tuple<Notable, Comment>;
  using ArcTypes = std::tuple<InvertibleImplicitArc, ImplicitArc, ExplicitArc>;
};

class Example3Traits
{
public:
  using NodeTypes = std::tuple<Thingy, Notable, Comment>;
  using ArcTypes = std::tuple<DirectedDistinctArc, UndirectedSelfArc>;
};

} // namespace test
} // namespace smtk
