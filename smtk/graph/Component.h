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

#include <memory>
#include <string>
#include <tuple>
#include <typeindex>

namespace smtk
{
namespace graph
{

class ResourceBase;

template <typename GraphTraits>
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
  template <typename GraphTraits>
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
  template <typename ArcType>
  bool contains() const
  {
    typedef const typename ArcType::template API<ArcType> API;
    return API().contains(*static_cast<const typename ArcType::FromType*>(this));
  }

  /// Access nodes connected from this node via an arc of type ArcType. The
  /// return type of this method is determined by the API of ArcTYpe.
  template <typename ArcType, typename... T>
  auto get(T&&... parameters) const
    -> decltype(std::declval<const typename ArcType::template API<ArcType> >().get(
      std::declval<const typename ArcType::FromType&>(), std::forward<T>(parameters)...))
  {
    typedef const typename ArcType::template API<ArcType> API;
    return API().get(
      *static_cast<const typename ArcType::FromType*>(this), std::forward<T>(parameters)...);
  }

  /// Access nodes connected from this node via an arc of type ArcType. The
  /// return type of this method is determined by the API of ArcTYpe.
  template <typename ArcType, typename... T>
  auto get(T&&... parameters)
    -> decltype(std::declval<typename ArcType::template API<ArcType> >().get(
      std::declval<const typename ArcType::FromType&>(), std::forward<T>(parameters)...))
  {
    typedef typename ArcType::template API<ArcType> API;
    return API().get(
      *static_cast<const typename ArcType::FromType*>(this), std::forward<T>(parameters)...);
  }

protected:
  Component(const std::shared_ptr<smtk::graph::ResourceBase>&);

  Component(const std::shared_ptr<smtk::graph::ResourceBase>&, const smtk::common::UUID&);

  std::weak_ptr<smtk::graph::ResourceBase> m_resource;
  smtk::common::UUID m_id;
};
}
}

#endif
