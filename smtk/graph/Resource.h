//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_Resource_h
#define smtk_graph_Resource_h

#include "smtk/geometry/Manager.h"
#include "smtk/geometry/Resource.h"

#include "smtk/graph/Component.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/TypeMap.h"

#include <memory>
#include <string>
#include <tuple>
#include <typeindex>

namespace smtk
{
namespace graph
{

/// A non-templated base class for graph resources. ResourceBase satisfies the
/// API of smtk::resource::Resource and isolates graph components from the traits
/// specification of smtk::graph::Resource.
class SMTKCORE_EXPORT ResourceBase
  : public smtk::resource::DerivedFrom<ResourceBase, smtk::geometry::Resource>
{
protected:
  struct Compare
  {
    bool operator()(const std::shared_ptr<smtk::resource::Component>& lhs,
      const std::shared_ptr<smtk::resource::Component>& rhs) const
    {
      return (!lhs ? true : (!rhs ? false : lhs->id() < rhs->id()));
    }
  };

public:
  smtkTypeMacro(smtk::graph::ResourceBase);
  smtkSuperclassMacro(smtk::resource::DerivedFrom<ResourceBase, smtk::geometry::Resource>);

  typedef std::set<std::shared_ptr<smtk::resource::Component>, Compare> NodeSet;
  typedef smtk::common::TypeMap<smtk::common::UUID> ArcSet;

  std::shared_ptr<smtk::resource::Component> find(const smtk::common::UUID&) const override;

  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string&) const override;

  void visit(std::function<void(const smtk::resource::ComponentPtr&)>& v) const override;

  const NodeSet& nodes() const { return m_nodes; }

  virtual const ArcSet& arcs() const = 0;
  virtual ArcSet& arcs() = 0;

protected:
  template <typename... Args>
  ResourceBase(Args&&... args)
    : Superclass(std::forward<Args>(args)...)
  {
  }

  NodeSet m_nodes;
};

/// smtk::graph::Resource is defined by a GraphTraits type that defines the node
/// types and arc types of a multipartite graph. The node types must all inherit
/// from smtk::resource::Component and comprise the elements of the resource.
/// The arc types can be any type, and can represent a 1-to-1 or 1-to-many
/// relationship between node types.
template <typename GraphTraits>
class SMTK_ALWAYS_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource<GraphTraits>, ResourceBase>
{
public:
  smtkTypedefs(smtk::graph::Resource<GraphTraits>);

  std::string typeName() const override
  {
    return "smtk::graph::Resource<" + smtk::common::typeName<GraphTraits>() + ">";
  }

  smtkCreateMacro(smtk::graph::Resource<GraphTraits>);
  smtkSuperclassMacro(smtk::resource::DerivedFrom<Resource<GraphTraits>, ResourceBase>);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  /// Create a node of type NodeType with additional constructor arguments.
  template <typename NodeType, typename... T>
  typename std::enable_if<smtk::tuple_contains<NodeType, typename GraphTraits::NodeTypes>::value,
    std::shared_ptr<NodeType> >::type
  create(T&&... parameters)
  {
    std::shared_ptr<smtk::resource::Component> created(
      new NodeType(this->shared_from_this(), std::forward<T>(parameters)...));
    auto node = std::static_pointer_cast<NodeType>(created);
    add(node);
    return node;
  }

  /// Add a node of type NodeType to the resource. Return true if the insertion
  /// took place.
  template <typename NodeType>
  typename std::enable_if<smtk::tuple_contains<NodeType, typename GraphTraits::NodeTypes>::value,
    bool>::type
  add(const std::shared_ptr<NodeType>& node)
  {
    return ResourceBase::m_nodes.insert(node).second;
  }

  /// Remove a node from the resource. Return true if the removal took place.
  template <typename NodeType>
  typename std::enable_if<smtk::tuple_contains<NodeType, typename GraphTraits::NodeTypes>::value,
    bool>::type
  remove(const std::shared_ptr<NodeType>& node)
  {
    return ResourceBase::m_nodes.erase(node) > 0;
  }

  /// Create an arc of type ArcType with additional constructor arguments.
  template <typename ArcType, typename... T>
  typename std::enable_if<smtk::tuple_contains<ArcType, typename GraphTraits::ArcTypes>::value,
    const ArcType&>::type
  create(T&&... parameters)
  {
    ArcType arc(std::forward<T>(parameters)...);

    smtk::common::UUID id = arc.from().id();
    add(std::move(arc));
    return m_arcs.at<ArcType>(id);
  }

  /// Add an arc of type ArcType to the resource. Return true if the insertion
  /// took place.
  template <typename ArcType>
  typename std::enable_if<smtk::tuple_contains<ArcType, typename GraphTraits::ArcTypes>::value,
    bool>::type
  add(ArcType&& arc)
  {
    smtk::common::UUID id = arc.from().id();
    return m_arcs.emplace<ArcType>(id, std::forward<ArcType>(arc));
  }

  /// Remove an arc from the resource. Return true if the removal took place.
  template <typename ArcType>
  typename std::enable_if<smtk::tuple_contains<ArcType, typename GraphTraits::ArcTypes>::value,
    bool>::type
  remove(const std::shared_ptr<ArcType>& arc)
  {
    return m_arcs.erase<ArcType>(arc.from().id());
  }

  /// Access the arcs of the graph resource.
  const ResourceBase::ArcSet& arcs() const override { return m_arcs; }
  ResourceBase::ArcSet& arcs() override { return m_arcs; }

protected:
  template <typename... Args>
  Resource(Args&&... args)
    : Superclass(std::forward<Args>(args)...)
    , m_arcs(identity<typename GraphTraits::ArcTypes>())
  {
  }

  ResourceBase::ArcSet m_arcs;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_Resource_h
