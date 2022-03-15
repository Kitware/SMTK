//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_Component_h
#define smtk_graph_Component_h

#include "smtk/resource/Component.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/graph/ResourceBase.h"
#include "smtk/graph/detail/TypeTraits.h"

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

template<typename T>
using remove_cvref_t = typename std::remove_reference<typename std::remove_cv<T>::type>::type;

template<typename T, typename = void>
struct TestValid
{
  bool operator()(const T&) const { return true; }
};

template<typename T>
struct TestValid<T, smtk::common::void_t<decltype(&T::expired)>>
{
  bool operator()(const T& node) const { return !node.expired(); }
};
} // namespace detail

template<typename GraphTraits>
class Resource;

/// Graph Component represents a node in the graph resource. Internally, all of
/// the nodes and graphs that comprise a resource are stored internally within
/// the resource itself. The API presented in Component is a forwarding API,
/// designed to redirect arc queries through the resource. As a result, graph
/// components themselves are extremely lightweight and contain a full API for
/// accessing their connections.
class SMTKCORE_EXPORT Component : public smtk::resource::Component
{
  // Only graph resources can construct graph components, ensuring that the
  // component's API can be satisfied.
  template<typename GraphTraits>
  friend class Resource;

public:
  smtkTypeMacro(smtk::graph::Component);
  smtkSuperclassMacro(smtk::resource::Component);

  /// Access the containing resource.
  const smtk::resource::ResourcePtr resource() const override;

  /// Access the component's id.
  const smtk::common::UUID& id() const override { return m_id; }

  /// Set the component's id.
  bool setId(const smtk::common::UUID& uid) override;

  /// Check if this node has any arcs of type ArcType.
  template<typename ArcType>
  bool contains() const
  {
    typedef const typename ArcType::template API<ArcType> API;
    return API().contains(*static_cast<const typename ArcType::FromType*>(this));
  }

  /// Set the node (or nodes) associated with this ArcType. If there is a
  /// preexisting value associated with this ArcType, it is replaced. Return true
  /// on success.
  ///
  /// NOTE: This method cannot be guarded by compile-time checks, so it is
  ///       instead allowed to fail at runtime. For compile-time checking, prefer
  ///       smtk::graph::Resource's API for creating ArcTypes.
  template<typename ArcType, typename... Args>
  bool set(Args&&... args)
  {
    // Since this method exists in the base Component class, we must check RTTI
    // to ensure the node type is appropriate for the ArcType's LHS.
    if (!dynamic_cast<typename ArcType::FromType*>(this))
    {
      return false;
    }

    auto resource = std::static_pointer_cast<smtk::graph::ResourceBase>(this->resource());

    auto& arcs = resource->arcs();
    if (arcs.containsType<ArcType>())
    {
      arcs.erase<ArcType>(this->id());
      arcs.emplace<ArcType>(
        this->id(),
        ArcType(static_cast<typename ArcType::FromType&>(*this), std::forward<Args>(args)...));
      return true;
    }
    return false;
  }

  /// The get() method returns the node (or nodes) connected to this node via
  /// the input arc type. The return type of this method is determined by the API
  /// of ArcType.
  // This overload handles const access to nodes
  template<typename ArcType>
  auto get() const -> decltype(std::declval<const typename ArcType::template API<ArcType>>().get(
    std::declval<const typename ArcType::FromType&>()))
  {
    typedef const typename ArcType::template API<ArcType> API;
    return API().get(*static_cast<const typename ArcType::FromType*>(this));
  }
  // This overload handles non-const access to nodes
  template<typename ArcType>
  auto get() -> decltype(std::declval<typename ArcType::template API<ArcType>>().get(
    std::declval<const typename ArcType::FromType&>()))
  {
    typedef typename ArcType::template API<ArcType> API;
    return API().get(*static_cast<const typename ArcType::FromType*>(this));
  }

  /// While get() returns the nodes connected to this component via the input arc
  /// type, visit() allows you to pass your calling code to each connected node
  /// without having to return a reference to each node. Input lambdas return a
  /// boolean value; when they return true, visitation is terminated early.
  // This overload handles const access to arcs that have no explicit
  // API::visit defined and return a container from get().
  template<typename ArcType, typename Visitor>
  typename std::enable_if<
    !detail::has_custom_visit<
      typename ArcType::template API<ArcType>,
      Visitor,
      const typename ArcType::ToType&>::value &&
      detail::is_container<decltype(std::declval<const typename ArcType::template API<ArcType>>()
                                      .get(std::declval<const typename ArcType::FromType&>())
                                      .to())>::value &&
      detail::accepts<Visitor, const typename ArcType::ToType&>::value,
    bool>::type
  visit(const Visitor& visitor) const
  {
    typedef const typename ArcType::template API<ArcType> API;
    // NOLINTNEXTLINE(readability-use-anyofallof)
    for (const auto& toType : API().get(*static_cast<const typename ArcType::FromType*>(this)).to())
    {
      if (detail::TestValid<detail::remove_cvref_t<decltype(toType)>>()(toType) && visitor(toType))
      {
        return true;
      }
    }
    return false;
  }
  // This overload handles non-const access to arcs that have no explicit
  // API::visit defined and return a container from get().
  template<typename ArcType, typename Visitor>
  typename std::enable_if<
    !detail::has_custom_visit<
      typename ArcType::template API<ArcType>,
      Visitor,
      typename ArcType::ToType&>::value &&
      detail::is_container<decltype(std::declval<typename ArcType::template API<ArcType>>()
                                      .get(std::declval<const typename ArcType::FromType&>())
                                      .to())>::value &&
      detail::accepts<Visitor, typename ArcType::ToType&>::value,
    bool>::type
  visit(const Visitor& visitor)
  {
    typedef typename ArcType::template API<ArcType> API;
    // NOLINTNEXTLINE(readability-use-anyofallof)
    for (auto& toType : API().get(*static_cast<typename ArcType::FromType*>(this)).to())
    {
      if (detail::TestValid<detail::remove_cvref_t<decltype(toType)>>()(toType) && visitor(toType))
      {
        return true;
      }
    }
    return false;
  }
  // This overload handles const access to arcs that have no explicit
  // API::visit defined and return a single node from get().
  template<typename ArcType, typename Visitor>
  typename std::enable_if<
    !detail::has_custom_visit<
      typename ArcType::template API<ArcType>,
      Visitor,
      const typename ArcType::ToType&>::value &&
      !detail::is_container<decltype(std::declval<const typename ArcType::template API<ArcType>>()
                                       .get(std::declval<const typename ArcType::FromType&>())
                                       .to())>::value &&
      detail::accepts<Visitor, const typename ArcType::ToType&>::value,
    bool>::type
  visit(const Visitor& visitor) const
  {
    typedef const typename ArcType::template API<ArcType> API;
    return visitor(API().get(*static_cast<const typename ArcType::FromType*>(this)).to());
  }
  // This overload handles non-const access to arcs that have no explicit
  // API::visit defined and return a single node from get().
  template<typename ArcType, typename Visitor>
  typename std::enable_if<
    !detail::has_custom_visit<
      typename ArcType::template API<ArcType>,
      Visitor,
      typename ArcType::ToType&>::value &&
      !detail::is_container<decltype(std::declval<typename ArcType::template API<ArcType>>()
                                       .get(std::declval<const typename ArcType::FromType&>())
                                       .to())>::value &&
      detail::accepts<Visitor, typename std::remove_const<typename ArcType::ToType&>::type>::value,
    bool>::type
  visit(const Visitor& visitor)
  {
    typedef typename ArcType::template API<ArcType> API;
    return visitor(API().get(*static_cast<typename ArcType::FromType*>(this)).to());
  }
  // This overload handles const access to arcs that have an explicit
  // API::visit defined.
  template<typename ArcType, typename Visitor>
  auto visit(const Visitor& visitor) const -> typename std::enable_if<
    detail::has_custom_visit<
      typename ArcType::template API<ArcType>,
      Visitor,
      const typename ArcType::ToType&>::value &&
      detail::accepts<Visitor, const typename ArcType::ToType&>::value,
    decltype(std::declval<const typename ArcType::template API<ArcType>>()
               .visit(std::declval<const typename ArcType::FromType&>(), visitor))>::type
  {
    typedef const typename ArcType::template API<ArcType> API;
    return API().visit(*static_cast<const typename ArcType::FromType*>(this), visitor);
  }
  // This overload handles non-const access to arcs that have an explicit
  // API::visit defined.
  template<typename ArcType, typename Visitor>
  auto visit(const Visitor& visitor) -> typename std::enable_if<
    detail::has_custom_visit<
      typename ArcType::template API<ArcType>,
      Visitor,
      const typename ArcType::ToType&>::value &&
      detail::accepts<Visitor, typename ArcType::ToType&>::value,
    decltype(std::declval<typename ArcType::template API<ArcType>>()
               .visit(std::declval<typename ArcType::FromType&>(), visitor))>::type
  {
    typedef typename ArcType::template API<ArcType> API;
    return API().visit(*static_cast<const typename ArcType::FromType*>(this), visitor);
  }

protected:
  Component(const std::shared_ptr<smtk::graph::ResourceBase>&);

  Component(const std::shared_ptr<smtk::graph::ResourceBase>&, const smtk::common::UUID&);

  virtual void createDependentArcs() {}

  std::weak_ptr<smtk::graph::ResourceBase> m_resource;
  smtk::common::UUID m_id;
};
} // namespace graph
} // namespace smtk

#endif
