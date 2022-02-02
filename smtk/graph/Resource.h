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

#include "smtk/PublicPointerDefs.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/TypeMap.h"
#include "smtk/common/TypeTraits.h"

#include "smtk/geometry/Manager.h"
#include "smtk/geometry/Resource.h"

#include "smtk/graph/Component.h"
#include "smtk/graph/NodeSet.h"
#include "smtk/graph/filter/Grammar.h"

#include "smtk/resource/filter/Filter.h"

#include <memory>
#include <string>
#include <tuple>
#include <typeindex>

namespace smtk
{
namespace graph
{
namespace detail
{

template<typename T, typename = void>
struct has_initializer : std::false_type
{
};

template<class T, class... Args>
struct has_initializer<
  T(Args...),
  smtk::common::void_t<decltype(std::declval<T>().initialize(std::declval<Args>()...))>>
  : std::true_type
{
};

template<class T, class... Args>
typename std::enable_if<detail::has_initializer<T(Args...)>::value>::type initialize(
  T& t,
  Args&&... args)
{
  t.initialize(std::forward<Args>(args)...);
}

template<class T, class... Args>
typename std::enable_if<!detail::has_initializer<T(Args...)>::value>::type initialize(
  T& t,
  Args&&... args)
{
}
} // namespace detail

/// smtk::graph::Resource is defined by a Traits type that defines the node
/// types and arc types of a multipartite graph. The node types must all inherit
/// from smtk::graph::Component and comprise the elements of the resource.
/// The arc types can be any type, and can represent a 1-to-1 or 1-to-many
/// relationship between node types.
template<typename Traits>
class SMTK_ALWAYS_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource<Traits>, ResourceBase>
  , public detail::GraphTraits<Traits>::NodeContainer
{

public:
  using NodeContainer = typename detail::GraphTraits<Traits>::NodeContainer;

  template<typename NodeType>
  using is_node = smtk::tuple_contains<NodeType, typename detail::GraphTraits<Traits>::NodeTypes>;
  template<typename ArcType>
  using is_arc = smtk::tuple_contains<ArcType, typename detail::GraphTraits<Traits>::ArcTypes>;

  smtkTypedefs(smtk::graph::Resource<Traits>);

  std::string typeName() const override
  {
    return "smtk::graph::Resource<" + smtk::common::typeName<Traits>() + ">";
  }

  smtkCreateMacro(smtk::graph::Resource<Traits>);
  smtkSuperclassMacro(smtk::resource::DerivedFrom<Resource<Traits>, ResourceBase>);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  /// Create a node of type NodeType with additional constructor arguments.
  template<typename NodeType, typename... T>
  typename std::enable_if<is_node<NodeType>::value, std::shared_ptr<NodeType>>::type create(
    T&&... parameters)
  {
    std::shared_ptr<smtk::graph::Component> created(new NodeType(this->shared_from_this()));

    auto node = std::static_pointer_cast<NodeType>(created);
    detail::initialize(*node, std::forward<T>(parameters)...);

    add(node);
    return node;
  }

  /// Add a node of type NodeType to the resource. Return true if the insertion
  /// took place.
  template<typename NodeType>
  typename std::enable_if<is_node<NodeType>::value, bool>::type add(
    const std::shared_ptr<NodeType>& node)
  {
    return NodeContainer::insertNode(node);
  }

  /// Remove a node from the resource. Return true if the removal took place.
  template<typename NodeType>
  typename std::enable_if<is_node<NodeType>::value, bool>::type remove(
    const std::shared_ptr<NodeType>& node)
  {
    return NodeContainer::eraseNodes(node) > 0;
  }

  /// Create an arc of type ArcType with additional constructor arguments.
  template<typename ArcType, typename... T>
  typename std::enable_if<is_arc<ArcType>::value, const ArcType&>::type create(T&&... parameters)
  {
    ArcType arc(std::forward<T>(parameters)...);

    smtk::common::UUID id = arc.from().id();
    add(std::move(arc));
    return m_arcs.at<ArcType>(id);
  }

  template<typename ArcType, typename... T>
  typename std::enable_if<is_arc<ArcType>::value, const ArcType&>::type connect(T&&... parameters)
  {
    ArcType arc(std::forward<T>(parameters)...);

    smtk::common::UUID id = arc.from().id();
    if (m_arcs.contains<ArcType>(id))
    {
      m_arcs.at<ArcType>(id).insert(arc.to());
    }
    else
    {
      add(std::move(arc));
    }
    return m_arcs.at<ArcType>(id);
  }

  /// Add an arc of type ArcType to the resource. Return true if the insertion
  /// took place.
  template<typename ArcType>
  typename std::enable_if<is_arc<ArcType>::value, bool>::type add(ArcType&& arc)
  {
    smtk::common::UUID id = arc.from().id();
    return m_arcs.emplace<ArcType>(id, std::forward<ArcType>(arc));
  }

  /// Remove an arc from the resource. Return true if the removal took place.
  template<typename ArcType>
  typename std::enable_if<is_arc<ArcType>::value, bool>::type remove(
    const std::shared_ptr<ArcType>& arc)
  {
    return m_arcs.erase<ArcType>(arc.from().id());
  }

  /// Access the arcs of the graph resource.
  const ArcMap& arcs() const override { return m_arcs; }
  ArcMap& arcs() override { return m_arcs; }

  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& filterString) const override
  {
    return smtk::resource::filter::Filter<smtk::graph::filter::Grammar>(filterString);
  }

  Resource(const Resource&) noexcept = delete;

  std::shared_ptr<smtk::resource::Component> find(const smtk::common::UUID& uuid) const override
  {
    return NodeContainer::find(uuid);
  }
  void visit(std::function<void(const smtk::resource::ComponentPtr&)>& v) const override
  {
    NodeContainer::visit(v);
  }

protected:
  std::size_t eraseNodes(const smtk::graph::ComponentPtr& node) override
  {
    return NodeContainer::eraseNodes(node) > 0;
  }

  bool insertNode(const smtk::graph::ComponentPtr& node) override
  {
    return NodeContainer::insertNode(node);
  }

  Resource(smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(manager)
    , m_arcs(identity<typename detail::GraphTraits<Traits>::ArcTypes>())
  {
  }

  Resource(const smtk::common::UUID& uid, smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(uid, manager)
    , m_arcs(identity<typename detail::GraphTraits<Traits>::ArcTypes>())
  {
  }

  ArcMap m_arcs;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_Resource_h
