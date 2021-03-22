//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/geometry/Manager.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace geometry
{

void Manager::visitBackends(std::function<void(const Backend&)> visitor) const
{
  for (const auto& entry : m_backends)
  {
    visitor(*entry.second);
  }
}

void Manager::registerResourceManager(const smtk::resource::Manager::Ptr& manager)
{
  auto oldManager = m_resourceManager.lock();
  if (oldManager && oldManager == manager)
  {
    return; // old == new => no-op
  }

  if (oldManager)
  {
    oldManager->observers().erase(m_resourceObserverKey);
  }

  m_resourceManager = manager;
  if (manager)
  {
    m_resourceObserverKey = manager->observers().insert(
      [this](const resource::Resource& resource, resource::EventType event) {
        if (event != resource::EventType::ADDED)
        {
          return;
        }
        auto* mutableResource = const_cast<resource::Resource*>(&resource);
        auto* geomResource = dynamic_cast<smtk::geometry::Resource*>(mutableResource);
        if (geomResource)
        {
          // If there are geometry backends registered,
          // have the resource generate a provider for
          // each backend that is possible.
          // Whether this is possible depends on which plugins
          // have been loaded at the time the resource is added.
          this->visitBackends([&geomResource](const Backend& backend) {
            const auto& geom = geomResource->geometry(backend);
            (void)geom;
          });
        }
      },
      /* priority */ 0,
      /* initialize */ true,
      "Add geometry objects to geometric resources.");
  }
}

void Manager::constructGeometry(
  const std::shared_ptr<smtk::resource::Manager>& resourceManager,
  Backend& backend)
{
  if (!resourceManager)
  {
    return;
  }

  resourceManager->visit([&backend](smtk::resource::Resource& resource) {
    if (auto* geomResource = dynamic_cast<smtk::geometry::Resource*>(&resource))
    {
      geomResource->geometry(backend);
    }
    return common::Processing::CONTINUE;
  });
}

} // namespace geometry
} // namespace smtk
