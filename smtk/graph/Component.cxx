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
  const std::shared_ptr<smtk::graph::ResourceBase>& resource, const smtk::common::UUID& uid)
  : m_resource(resource)
  , m_id(uid)
{
}

bool Component::setId(const smtk::common::UUID& uid)
{
  if (auto resource = m_resource.lock())
  {
    const_cast<ResourceBase::NodeSet&>(resource->nodes()).erase(this->shared_from_this());
    smtk::common::UUID tmp = m_id;
    m_id = uid;
    if (const_cast<ResourceBase::NodeSet&>(resource->nodes())
          .insert(this->shared_from_this())
          .second)
    {
      return true;
    }
    else
    {
      m_id = tmp;
      return false;
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
}
}
