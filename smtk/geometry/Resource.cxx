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
    std::tuple<std::shared_ptr<smtk::geometry::Resource>, const smtk::geometry::Backend&> instance(
      this->shared_from_this(), backend);
    std::unique_ptr<Geometry> provider = geomGen(instance);
    if (provider)
    {
      std::cout << "Adding provider for " << backend.name() << " " << backend.index() << " to "
                << this->name() << "\n";
      // Now, because the resource may not be empty at this point,
      // we want to discover which objects have geometry without
      // actually generating geometry...  so mark every component
      // plus the resource itself as modified.
      provider->markModified(shared_from_this());
      smtk::resource::Component::Visitor visitor = [&provider](
        const resource::ComponentPtr& component) {
        std::cout << "  Does " << component->name() << " have geom?\n";
        provider->markModified(component);
      };
      this->visit(visitor);
      // Move ownership of the geometry provider to the resource.
      m_geometry[backend.index()] = std::move(provider);
      it = m_geometry.find(backend.index());
      std::cout << "  Did add ? " << (it == m_geometry.end() ? "N" : "Y") << "\n";
    }
    else
    {
      return empty;
    }
    return it->second;
  }
  return it->second;
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
