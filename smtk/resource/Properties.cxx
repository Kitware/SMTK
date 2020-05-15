//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/Properties.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace resource
{
namespace detail
{
ResourceProperties::ResourceProperties(Resource* resource)
  : m_resource(resource)
  , m_data(identity<PropertyTypes>())
{
  // NOTE: When modifying this constructor, do not use the resource parameter
  // or m_resource field! The parent Resouce is still in construction and is in
  // an indeterminate state.
}

const smtk::common::UUID& ResourceProperties::id() const
{
  return m_resource->id();
}

ComponentProperties::ComponentProperties(const Component* component)
  : m_component(component)
{
  // NOTE: When modifying this constructor, do not use the component parameter
  // or m_component field! The parent Component is still in construction and is
  // in an indeterminate state.
}

ComponentProperties::~ComponentProperties() = default;

const smtk::common::UUID& ComponentProperties::id() const
{
  return m_component->id();
}

smtk::common::TypeMapBase<std::string>& ComponentProperties::properties()
{
  return m_component->resource()->properties().data();
}

const smtk::common::TypeMapBase<std::string>& ComponentProperties::properties() const
{
  return m_component->resource()->properties().data();
}
}
}
}
