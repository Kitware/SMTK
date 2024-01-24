//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_ArcImplementationBase_h
#define smtk_graph_ArcImplementationBase_h

#include "smtk/CoreExports.h"
#include "smtk/graph/ArcProperties.h" // for unconstrained()
#include "smtk/graph/ArcTraits.h"     // for ConstArc/NonConstArc
#include "smtk/graph/Directionality.h"
#include "smtk/resource/Component.h"
#include "smtk/string/Token.h"

namespace smtk
{
namespace graph
{

class ResourceBase;
template<typename Const>
class RuntimeArcEndpoint;

/**\brief A base class for all arc implementations.
  *
  * This class provides a runtime API for arc endpoints at a given "self" node.
  */
class SMTKCORE_EXPORT ArcImplementationBase
{
public:
  /// The signature for functors that are invoked on a visit.
  using RuntimeNodeVisitor = std::function<smtk::common::Visit(const Component* node)>;

  ArcImplementationBase() = default;

  /// Return the type of arc this class implements.
  virtual std::string typeName() const { return std::string(); }

  virtual Directionality directionality() const { return Directionality::IsDirected; }
  virtual bool mutability() const { return false; }

  virtual std::unordered_set<smtk::string::Token> fromTypes() const { return {}; }
  virtual std::unordered_set<smtk::string::Token> toTypes() const { return {}; }

  /// The minimum out-degree of a FromType node. This is not enforced.
  virtual std::size_t minimumOutDegree() const { return unconstrained(); }
  /// The minimum in-degree of a ToType node. This is not enforced.
  virtual std::size_t minimumInDegree() const { return unconstrained(); }
  /// The maximum out-degree of a FromType node. This is enforced.
  virtual std::size_t maximumOutDegree() const { return unconstrained(); }
  /// The maximum in-degree of a ToType node. This is enforced.
  virtual std::size_t maximumInDegree() const { return unconstrained(); }

  virtual bool acceptsRuntime(
    Component* from,
    Component* to,
    Component* beforeFrom = nullptr,
    Component* beforeTo = nullptr) const
  {
    (void)from;
    (void)to;
    (void)beforeFrom;
    (void)beforeTo;
    return false;
  }

  /// Return a "container" of outgoing arcs of the given \a from node.
  virtual RuntimeArcEndpoint<ConstArc> outgoingRuntime(const Component* from) const
  {
    (void)from;
    return RuntimeArcEndpoint<ConstArc>();
  }
  virtual RuntimeArcEndpoint<NonConstArc> outgoingRuntime(const Component* from)
  {
    (void)from;
    return RuntimeArcEndpoint<NonConstArc>();
  }

  /// Return a "container" of incoming arcs of the given \a to node.
  virtual RuntimeArcEndpoint<ConstArc> incomingRuntime(const Component* to) const
  {
    (void)to;
    return RuntimeArcEndpoint<ConstArc>();
  }
  virtual RuntimeArcEndpoint<NonConstArc> incomingRuntime(const Component* to)
  {
    (void)to;
    return RuntimeArcEndpoint<NonConstArc>();
  }

  /// Invoke the visitor on all arcs (in their outgoing sense).
  virtual smtk::common::Visited visitOutgoingNodes(
    const smtk::graph::ResourceBase* resource,
    smtk::string::Token arcTypeName,
    RuntimeNodeVisitor visitor) const
  {
    (void)resource;
    (void)arcTypeName;
    (void)visitor;
    return smtk::common::Visited::Empty;
  }
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_ArcImplementationBase_h
