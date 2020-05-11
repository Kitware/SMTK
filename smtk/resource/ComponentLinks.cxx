//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/ComponentLinks.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace resource
{
namespace detail
{
ComponentLinks::ComponentLinks(const Component* component)
  : m_component(component)
{
  // NOTE: When modifying this constructor, do not use the component parameter
  // or m_component field! The parent Component is still in construction and is
  // in an indeterminate state.
}

Resource* ComponentLinks::leftHandSideResource()
{
  return m_component->resource().get();
}

const Resource* ComponentLinks::leftHandSideResource() const
{
  return m_component->resource().get();
}

const smtk::common::UUID& ComponentLinks::leftHandSideComponentId() const
{
  return m_component->id();
}
}
}
}
