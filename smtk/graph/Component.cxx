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

#include "smtk/common/UUIDGenerator.h"

namespace smtk
{
namespace graph
{

Component::Component(const std::shared_ptr<smtk::graph::ResourceBase>& resource)
  : m_resource(resource)
  , m_id(smtk::common::UUIDGenerator::instance().random())
{
}

Component::Component(
  const std::shared_ptr<smtk::graph::ResourceBase>& resource,
  const smtk::common::UUID& uid)
  : m_resource(resource)
  , m_id(uid)
{
}

const smtk::resource::ResourcePtr Component::resource() const
{
  smtk::resource::ResourcePtr rsrc;
  try
  {
    rsrc = m_resource.get().shared_from_this();
  }
  catch (std::bad_weak_ptr&)
  {
    rsrc = nullptr;
  }
  return rsrc;
}

smtk::resource::Resource* Component::parentResource() const
{
  smtk::resource::Resource* rsrc = nullptr;
  try
  {
    rsrc = &m_resource.get();
  }
  catch (const std::bad_weak_ptr&)
  {
    rsrc = nullptr;
  }
  return rsrc;
}

bool Component::setId(const smtk::common::UUID& uid)
{
  if (auto* resource = static_cast<smtk::graph::ResourceBase*>(this->parentResource()))
  {
    auto thisPtr = std::static_pointer_cast<Component>(this->shared_from_this());
    auto count = resource->eraseNodes(thisPtr);
    smtk::common::UUID tmp = m_id;
    m_id = uid;
    // Not checking the count == 1 here. If more than one node was removed
    // that is considered an intended property of the resource NodeContainer.
    if (count > 0)
    {
      if (resource->insertNode(thisPtr))
      {
        return true;
      }
      else
      {
        m_id = tmp;
        return false;
      }
    }
    // Nothing was removed, so the id was successfully changed.
    else
    {
      // This Component does not belong to the resource, so remove the weak reference.
      m_resource.reset();
      return true;
    }
  }

  m_id = uid;
  return true;
}

bool Component::disconnect(bool onlyExplicit)
{
  auto* resource = this->parentResourceAs<smtk::graph::ResourceBase>();
  if (!resource)
  {
    return false;
  }
  return resource->disconnect(this, onlyExplicit);
}

RuntimeArcEndpoint<NonConstArc> Component::outgoing(smtk::string::Token arcType)
{
  if (auto* arcData = this->arcsOfType(arcType))
  {
    return arcData->outgoingRuntime(this);
  }
  return RuntimeArcEndpoint<NonConstArc>();
}

RuntimeArcEndpoint<ConstArc> Component::outgoing(smtk::string::Token arcType) const
{
  if (const auto* arcData = this->arcsOfType(arcType))
  {
    return arcData->outgoingRuntime(this);
  }
  return RuntimeArcEndpoint<ConstArc>();
}

RuntimeArcEndpoint<NonConstArc> Component::incoming(smtk::string::Token arcType)
{
  if (auto* arcData = this->arcsOfType(arcType))
  {
    return arcData->incomingRuntime(this);
  }
  return RuntimeArcEndpoint<NonConstArc>();
}

RuntimeArcEndpoint<ConstArc> Component::incoming(smtk::string::Token arcType) const
{
  if (const auto* arcData = this->arcsOfType(arcType))
  {
    return arcData->incomingRuntime(this);
  }
  return RuntimeArcEndpoint<ConstArc>();
}

const ArcImplementationBase* Component::arcsOfType(smtk::string::Token arcType) const
{
  const ArcImplementationBase* arcsOfType = nullptr;
  auto* resource = static_cast<smtk::graph::ResourceBase*>(this->parentResource());
  if (resource)
  {
    auto& arcs = resource->arcs();
    arcsOfType = arcs.at<ArcImplementationBase>(arcType);
    if (!arcsOfType)
    {
      // If we were passed the type-name of a compile-time traits type,
      // try the storage type of its arcs:
      arcsOfType =
        arcs.at<ArcImplementationBase>("smtk::graph::ArcImplementation<" + arcType.data() + ">");
    }
  }
  return arcsOfType;
}

ArcImplementationBase* Component::arcsOfType(smtk::string::Token arcType)
{
  ArcImplementationBase* arcsOfType = nullptr;
  auto* resource = static_cast<smtk::graph::ResourceBase*>(this->parentResource());
  if (resource)
  {
    auto& arcs = resource->arcs();
    arcsOfType = arcs.at<ArcImplementationBase>(arcType);
    if (!arcsOfType)
    {
      // If we were passed the type-name of a compile-time traits type,
      // try the storage type of its arcs:
      arcsOfType =
        arcs.at<ArcImplementationBase>("smtk::graph::ArcImplementation<" + arcType.data() + ">");
    }
  }
  return arcsOfType;
}

bool Component::assign(const ConstPtr& source, smtk::resource::CopyOptions& options)
{
  // By default, nodes have no state that needs to be copied.
  return true;
}

} // namespace graph
} // namespace smtk
