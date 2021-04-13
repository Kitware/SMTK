//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/ItemDefinitionManager.h"

namespace smtk
{
namespace attribute
{

ItemDefinitionManager::ItemDefinitionManager() = default;
ItemDefinitionManager::~ItemDefinitionManager() = default;

bool ItemDefinitionManager::registerResourceManager(smtk::resource::ManagerPtr& manager)
{
  // For each register function...
  for (auto& registerFunction : m_registerFunctions)
  {
    // ...add an observer that adds the new definition to all current and
    // future attribute resources associated with this manager.
    auto registerCustomTypeObserver =
      [=](const smtk::resource::Resource& resource, smtk::resource::EventType eventType) -> void {
      if (eventType == smtk::resource::EventType::ADDED)
      {
        if (
          const smtk::attribute::Resource* attributeResource =
            dynamic_cast<const smtk::attribute::Resource*>(&resource))
        {
          registerFunction.second(const_cast<smtk::attribute::Resource&>(*attributeResource));
        }
      }
    };

    // Associate the observer key with the definition type, so we can remove it
    // later if requested.
    m_observers.insert(std::make_pair(
      registerFunction.first,
      manager->observers().insert(registerCustomTypeObserver, "Register custom attribute type.")));
  }

  m_manager = manager;

  return true;
}
} // namespace attribute
} // namespace smtk
