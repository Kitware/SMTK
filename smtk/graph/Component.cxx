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

bool Component::setId(const smtk::common::UUID& uid)
{
  if (auto resource = m_resource.lock())
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
      // This Component does not belong the the resource, so remove the weak reference.
      m_resource.reset();
      return true;
    }
  }

  m_id = uid;
  return true;
}

const smtk::resource::ResourcePtr Component::resource() const
{
  auto rsrc = m_resource.lock();
  return rsrc;
}
} // namespace graph
} // namespace smtk
