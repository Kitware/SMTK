//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_RuntimeArcEndpoint_h
#define smtk_graph_RuntimeArcEndpoint_h

#include "smtk/common/Visit.h"
#include "smtk/graph/ArcTraits.h"

namespace smtk
{
namespace graph
{

class Component;

/**\brief An object that can query and manipulate arcs at run-time.
  *
  * This class is a run-time analog to ArcEndpointInterface<>; it uses functors
  * provided by templated subclasses to implement operations independent of
  * arc- and node-type.
  *
  * This allows fast, compile-time, non-virtual graph code to run with low overhead
  * while providing a slower API that is more usable.
  *
  * It accepts a template parameter, \a Const, that should be ConstArc if
  * the endpoint is considered "const" and NonConstArc otherwise.
  */
template<typename Const>
class SMTK_ALWAYS_EXPORT RuntimeArcEndpoint
{
public:
  using Constness = Const;
  using DegreeLimitsFunctor = std::function<std::size_t(bool)>;
  using ContainsFunctor = std::function<bool(const Component*)>;
  using NodeValenceFunctor = std::function<std::size_t()>;
  using InsertArcFunctor =
    std::function<bool(const Component*, const Component*, const Component*)>;
  using EraseArcFunctor = std::function<bool(const Component*)>;
  using FullVisitor = std::function<smtk::common::Visit(const Component*)>;
  using FullVisitFunctor = std::function<smtk::common::Visited(FullVisitor)>;

  /// Create an "invalid" runtime arc endpoint.
  ///
  /// When you ask for outgoing arcs of a non-existent or improper type,
  /// this object is returned to indicate that no arcs of that type exist.
  /// Note in particular that minDegree() > maxDegree() for this endpoint
  /// (which you can use to test endpoint validity).
  RuntimeArcEndpoint()
    : m_degreeLimitsFunctor([](bool isMax) { return isMax ? 0 : ~0; })
    , m_nodeValenceFunctor([]() { return 0; })
    , m_containsFunctor([](const Component*) { return false; })
    , m_insertArcFunctor([](const Component*, const Component*, const Component*) { return false; })
    , m_eraseArcFunctor([](const Component*) { return false; })
    , m_fullVisitFunctor([](FullVisitor) { return smtk::common::Visited::Empty; })
  {
  }
  /// Allow objects of this type to be copied.
  RuntimeArcEndpoint(const RuntimeArcEndpoint<Const>&) = default;
  RuntimeArcEndpoint& operator=(const RuntimeArcEndpoint<Const>&) = default;

  RuntimeArcEndpoint(
    smtk::string::Token arcType,
    const Component* self,
    bool isOutgoing,
    DegreeLimitsFunctor degreeLimitsFunctor,
    NodeValenceFunctor nodeValenceFunctor,
    ContainsFunctor containsFunctor,
    InsertArcFunctor insertArcFunctor,
    EraseArcFunctor eraseArcFunctor,
    FullVisitFunctor fullVisitFunctor)
    : m_arcTypeName(arcType)
    , m_endpoint(const_cast<Component*>(self))
    , m_outgoing(isOutgoing)
    , m_degreeLimitsFunctor(degreeLimitsFunctor)
    , m_nodeValenceFunctor(nodeValenceFunctor)
    , m_containsFunctor(containsFunctor)
    , m_insertArcFunctor(insertArcFunctor)
    , m_eraseArcFunctor(eraseArcFunctor)
    , m_fullVisitFunctor(fullVisitFunctor)
  {
  }

  /// A run-time interface may be invalid due to run-time constraints.
  /// This method should be used to test that the node is allowed to
  /// be an endpoint for arcs of this type.
  bool valid() const { return m_arcTypeName.valid(); }

  Component* self() { return m_endpoint; }
  std::size_t maxDegree() const { return m_degreeLimitsFunctor(true); }
  std::size_t minDegree() const { return m_degreeLimitsFunctor(false); }

  template<typename Functor>
  smtk::common::Visited visit(Functor visitor) const
  {
    smtk::common::VisitorFunctor<Functor> vv(visitor);
    return m_fullVisitFunctor(vv);
  }

  bool contains(const Component* node) const { return m_containsFunctor(node); }
  bool contains(const std::shared_ptr<Component>& node) const { return this->contains(node.get()); }

  std::size_t degree() const { return m_nodeValenceFunctor(); }
  std::size_t size() const { return this->degree(); }
  bool empty() const { return this->size() == 0; }

  /// A convenience to return the first node.
  const Component* node() const
  {
    const Component* result = nullptr;
    this->visit([&result](const Component* node) {
      result = node;
      return result ? smtk::common::Visit::Halt : smtk::common::Visit::Continue;
    });
    return result;
  }
  /// STL-container synonym for node():
  const Component* front() const { return this->node(); }

  bool insert(
    const Component* other,
    const Component* beforeOther = nullptr,
    const Component* beforeThis = nullptr)
  {
    static_assert(!Const::value, "Attempt to modify a const endpoint.");
    // Read-only arcs will have a null functor and always return false:
    if (!m_insertArcFunctor)
    {
      return false;
    }

    return m_insertArcFunctor(other, beforeOther, beforeThis);
  }
  bool insert(
    const std::shared_ptr<Component>& other,
    const std::shared_ptr<Component>& beforeOther = nullptr,
    const std::shared_ptr<Component>& beforeThis = nullptr)
  {
    return this->insert(other.get(), beforeOther.get(), beforeThis.get());
  }
  bool connect(const Component* other)
  {
    // Read-only arcs will have a null functor and always return false:
    if (!m_insertArcFunctor)
    {
      return false;
    }

    return m_insertArcFunctor(other, nullptr, nullptr);
  }
  bool connect(const std::shared_ptr<Component>& other) { return this->connect(other.get()); }

  bool disconnect(const Component* other)
  {
    static_assert(!Const::value, "Attempt to modify a const endpoint.");
    // Read-only arcs will have a null functor and always return false:
    if (!m_eraseArcFunctor)
    {
      return false;
    }

    return m_eraseArcFunctor(other);
  }
  bool disconnect(const std::shared_ptr<Component>& other) { return this->disconnect(other.get()); }

  bool erase(const Component* other)
  {
    static_assert(!Const::value, "Attempt to modify a const endpoint.");
    // Read-only arcs will have a null functor and always return false:
    if (!m_eraseArcFunctor)
    {
      return false;
    }

    return m_eraseArcFunctor(other);
  }
  bool erase(const std::shared_ptr<Component>& other) { return this->erase(other.get()); }

protected:
  smtk::string::Token m_arcTypeName;
  Component* m_endpoint{ nullptr };
  bool m_outgoing{ true }; // Is m_endpoint the head or tail of the arc?

  DegreeLimitsFunctor m_degreeLimitsFunctor;
  NodeValenceFunctor m_nodeValenceFunctor;
  ContainsFunctor m_containsFunctor;
  InsertArcFunctor m_insertArcFunctor;
  EraseArcFunctor m_eraseArcFunctor;
  FullVisitFunctor m_fullVisitFunctor;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_RuntimeArcEndpoint_h
