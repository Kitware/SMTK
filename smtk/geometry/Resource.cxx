//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkResource.cxx - Abstract base class for CMB resources
// .SECTION Description
// .SECTION See Also

#include "smtk/geometry/Resource.h"

#include "smtk/common/Paths.h"
#include "smtk/common/UUIDGenerator.h"

#include "smtk/geometry/Backend.h"
#include "smtk/geometry/Generator.h"
#include "smtk/geometry/Geometry.h"
#include "smtk/geometry/Manager.h"

namespace smtk
{
namespace geometry
{

Resource::Resource(const smtk::common::UUID& myID, resource::ManagerPtr manager)
  : Superclass(myID, manager)
{
}

Resource::Resource(const smtk::common::UUID& myID)
  : Superclass(myID)
{
}

Resource::Resource(resource::ManagerPtr manager)
  : Superclass(manager)
{
}

Resource::~Resource() = default;

std::unique_ptr<Geometry>& Resource::geometry(const Backend& backend)
{
  static std::unique_ptr<Geometry> empty;
  auto it = m_geometry.find(backend.index());
  if (it == m_geometry.end())
  {
    geometry::Generator geomGen;
    std::unique_ptr<Geometry> provider = geomGen({ this->shared_from_this(), backend });
    if (provider)
    {
      m_geometry[backend.index()] = std::move(provider);
      it = m_geometry.find(backend.index());
    }
    else
    {
      return empty;
    }
    return it->second;
  }
  return empty;
}

void Resource::visitGeometry(std::function<void(std::unique_ptr<Geometry>&)> visitor)
{
  for (auto& entry : m_geometry)
  {
    visitor(entry.second);
  }
}

} // namespace resource
} // namespace smtk
