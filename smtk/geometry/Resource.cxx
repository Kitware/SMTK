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
  : DirectSuperclass(myID, manager)
{
}

Resource::Resource(const smtk::common::UUID& myID)
  : DirectSuperclass(myID)
{
}

Resource::Resource(resource::ManagerPtr manager)
  : DirectSuperclass(manager)
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
    std::tuple<std::shared_ptr<smtk::geometry::Resource>, const smtk::geometry::Backend&> instance(
      this->shared_from_this(), backend);
    std::unique_ptr<Geometry> provider = geomGen(instance);
    if (provider)
    {
      // Now, because the resource may not be empty at this point,
      // we want to discover which objects have geometry without
      // actually generating geometry...  so mark every component
      // plus the resource itself as modified.
      provider->markModified(shared_from_this());
      smtk::resource::Component::Visitor visitor =
        [&provider](const resource::ComponentPtr& component) { provider->markModified(component); };
      this->visit(visitor);
      // Move ownership of the geometry provider to the resource.
      m_geometry[backend.index()] = std::move(provider);
      it = m_geometry.find(backend.index());
    }
    else
    {
      return empty;
    }
    return it->second;
  }
  return it->second;
}

std::unique_ptr<Geometry>& Resource::geometry()
{
  auto it = m_geometry.begin();
  if (it != m_geometry.end())
  {
    return it->second;
  }

  static std::unique_ptr<Geometry> empty;
  return empty;
}

void Resource::visitGeometry(std::function<void(std::unique_ptr<Geometry>&)> visitor)
{
  for (auto& entry : m_geometry)
  {
    visitor(entry.second);
  }
}

} // namespace geometry
} // namespace smtk
