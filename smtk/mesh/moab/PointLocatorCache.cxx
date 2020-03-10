//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/moab/PointLocatorCache.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

namespace smtk
{
namespace mesh
{
namespace moab
{

void PointLocatorCache::synchronize(
  const smtk::operation::Operation&, const smtk::operation::Operation::Result& result)
{
  for (auto& component : { result->findComponent("expunged"), result->findComponent("modified") })
  {
    for (std::size_t i = 0; i < component->numberOfValues(); ++i)
    {
      m_caches.erase(component->value(i)->id());
    }
  }
}
}
}
}
