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

#include "smtk/common/WeakReferenceWrapper.h"
#include "smtk/resource/Component.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/graph/ArcMap.h"
#include "smtk/graph/ResourceBase.h"
#include "smtk/graph/detail/TypeTraits.h"

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <typeindex>

namespace smtk
{
namespace resource
{
class CopyOptions;
}
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
  ~Component() override = default; // Ensure this class is polymorphic for pybind11 downcasting.

  /// Access the containing resource.
  const smtk::resource::ResourcePtr resource() const override;
  smtk::resource::Resource* parentResource() const override;

  /// Access the component's id.
  const smtk::common::UUID& id() const override { return m_id; }

  /// Set the component's id.
  bool setId(const smtk::common::UUID& uid) override;

  /**\brief Return an endpoint-interface object for arcs of \a ArcType
    *       outgoing from this node.
    *
    * These methods will throw a `std::logic_error` if \a ArcType arcs
    * may not originate from nodes of this type.
    *
    * It would be preferable to prevent compilation, but the base
    * graph-component class can not have access to the resource's
    * traits object.
    */
  //@{
  template<typename ArcType>
  ArcEndpointInterface<ArcType, ConstArc, OutgoingArc> outgoing() const
  {
    // const version
    if (const auto* self = dynamic_cast<const typename ArcType::FromType*>(this))
    {
      auto* resource = static_cast<smtk::graph::ResourceBase*>(this->parentResource());
      if (resource)
      {
        const auto& arcs = resource->arcs();
        auto* arcsOfType = arcs.at<ArcType>();
        if (arcsOfType)
        {
          return arcsOfType->outgoing(self);
        }
        std::ostringstream errorMessage;
        errorMessage << "No arcs of the requested type (" << smtk::common::typeName<ArcType>()
                     << ", " << typeid(ArcType).hash_code() << ") are allowed.";
        throw std::logic_error(errorMessage.str());
      }
      throw std::logic_error("Component has no parent resource.");
    }
    else
    {
      std::ostringstream msg;
      msg << "This component is not the proper type (" << this->typeName()
          << ") for the arc's endpoint (" << smtk::common::typeName<typename ArcType::FromType>()
          << ").";
      throw std::logic_error(
        msg.str()); // "This component is not the proper type for the arc's endpoint.");
    }
    ArcEndpointInterface<ArcType, ConstArc, OutgoingArc> dummy;
    return dummy;
  }

  template<typename ArcType>
  ArcEndpointInterface<ArcType, NonConstArc, OutgoingArc> outgoing()
  {
    // non-const version
    if (auto* self = dynamic_cast<typename ArcType::FromType*>(this))
    {
      auto* resource = static_cast<smtk::graph::ResourceBase*>(this->parentResource());
      if (resource)
      {
        auto& arcs = resource->arcs();
        auto* arcsOfType = arcs.at<ArcType>();
        if (arcsOfType)
        {
          return arcsOfType->outgoing(self);
        }
        std::ostringstream errorMessage;
        errorMessage << "No arcs of the requested type (" << smtk::common::typeName<ArcType>()
                     << ", " << typeid(ArcType).hash_code() << ") are allowed.";
        throw std::logic_error(errorMessage.str());
      }
      throw std::logic_error("Component has no parent resource.");
    }
    else
    {
      std::ostringstream msg;
      msg << "This component is not the proper type (" << this->typeName()
          << ") for the arc's endpoint (" << smtk::common::typeName<typename ArcType::FromType>()
          << ").";
      throw std::logic_error(
        msg.str()); // "This component is not the proper type for the arc's endpoint.");
    }
    ArcEndpointInterface<ArcType, NonConstArc, OutgoingArc> dummy;
    return dummy;
  }
  //@}

  /**\brief Return an endpoint-interface object for arcs of \a ArcType
    *       incoming to this node.
    *
    * These methods will throw a `std::logic_error` if \a ArcType arcs
    * may not arrive into nodes of this type.
    *
    * It would be preferable to prevent compilation, but the base
    * graph-component class can not have access to the resource's
    * traits object.
    */
  //@{
  template<typename ArcType>
  ArcEndpointInterface<ArcType, ConstArc, IncomingArc> incoming() const
  {
    // const version
    if (const auto* self = dynamic_cast<const typename ArcType::ToType*>(this))
    {
      auto* resource = static_cast<smtk::graph::ResourceBase*>(this->parentResource());
      if (resource)
      {
        const auto& arcs = resource->arcs();
        auto* arcsOfType = arcs.at<ArcType>();
        if (arcsOfType)
        {
          return arcsOfType->incoming(self);
        }
        std::ostringstream errorMessage;
        errorMessage << "No arcs of the requested type (" << smtk::common::typeName<ArcType>()
                     << ", " << typeid(ArcType).hash_code() << ") are allowed.";
        throw std::logic_error(errorMessage.str());
      }
      throw std::logic_error("Component has no parent resource.");
    }
    else
    {
      std::ostringstream msg;
      msg << "This component is not the proper type (" << this->typeName()
          << ") for the arc's endpoint (" << smtk::common::typeName<typename ArcType::ToType>()
          << ").";
      throw std::logic_error(
        msg.str()); // "This component is not the proper type for the arc's endpoint.");
    }
    ArcEndpointInterface<ArcType, ConstArc, IncomingArc> dummy;
    return dummy;
  }

  template<typename ArcType>
  ArcEndpointInterface<ArcType, NonConstArc, IncomingArc> incoming()
  {
    // non-const version
    if (auto* self = dynamic_cast<typename ArcType::ToType*>(this))
    {
      auto* resource = static_cast<smtk::graph::ResourceBase*>(this->parentResource());
      if (resource)
      {
        auto& arcs = resource->arcs();
        auto* arcsOfType = arcs.at<ArcType>();
        if (arcsOfType)
        {
          return arcsOfType->incoming(self);
        }
        std::ostringstream errorMessage;
        errorMessage << "No arcs of the requested type (" << smtk::common::typeName<ArcType>()
                     << " id " << typeid(ArcType).hash_code() << ", " << typeid(ArcType).hash_code()
                     << ") are allowed.";
        throw std::logic_error(errorMessage.str());
      }
      throw std::logic_error("Component has no parent resource.");
    }
    else
    {
      std::ostringstream msg;
      msg << "This component is not the proper type (" << this->typeName()
          << ") for the arc's endpoint (" << smtk::common::typeName<typename ArcType::ToType>()
          << ").";
      throw std::logic_error(
        msg.str()); // "This component is not the proper type for the arc's endpoint.");
    }
    ArcEndpointInterface<ArcType, NonConstArc, IncomingArc> dummy;
    return dummy;
  }
  //@}

  /**\brief Remove all (editable) arcs from this node.
    *
    * Note that implicit arcs which do not provide a disconnect method
    * will not be removed.
    *
    * This method is used by the Delete operation to ensure that
    * nodes do not leave dangling references to removed nodes.
    */
  bool disconnect(bool onlyExplicit = false);

  /// Given a runtime arc type-name, return a runtime endpoint interface.
  ///
  /// If the arc type does not exist or this component has no such endpoints,
  /// this method will return an invalid endpoint interface.
  RuntimeArcEndpoint<NonConstArc> outgoing(smtk::string::Token arcType);
  RuntimeArcEndpoint<ConstArc> outgoing(smtk::string::Token arcType) const;
  RuntimeArcEndpoint<NonConstArc> incoming(smtk::string::Token arcType);
  RuntimeArcEndpoint<ConstArc> incoming(smtk::string::Token arcType) const;

  /// Assign state data from the \a source node to this instance.
  ///
  /// This method is only intended to be called from a graph resource's
  /// copyInitialize() method to assign node state from the \a source.
  /// At the time this is called, \a options will have a mapping from
  /// every node in \a source's parent resource to every node in this
  /// node's parent resource.
  ///
  /// The order in which assign() is invoked on nodes is not specified,
  /// so you may not rely on other nodes having their state set properly.
  virtual bool assign(const ConstPtr& source, smtk::resource::CopyOptions& options);

protected:
  Component(const std::shared_ptr<smtk::graph::ResourceBase>&);

  Component(const std::shared_ptr<smtk::graph::ResourceBase>&, const smtk::common::UUID&);

  const ArcImplementationBase* arcsOfType(smtk::string::Token arcType) const;
  ArcImplementationBase* arcsOfType(smtk::string::Token arcType);

  virtual void createDependentArcs() {}

  smtk::WeakReferenceWrapper<smtk::graph::ResourceBase> m_resource;
  smtk::common::UUID m_id;
};
} // namespace graph
} // namespace smtk

#endif
