//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/AssociationRuleManager.h"

namespace smtk
{
namespace attribute
{

AssociationRuleManager::AssociationRuleManager() = default;
AssociationRuleManager::~AssociationRuleManager() = default;

bool AssociationRuleManager::registerResourceManager(smtk::resource::ManagerPtr& manager)
{
  // For each association register function...
  for (auto& registerFunction : m_associationRegisterFunctions)
  {
    // ...add an observer that adds the new rule to all current and
    // future attribute resources associated with this manager.
    auto registerCustomTypeObserver = [=](
      const smtk::resource::Resource& resource, smtk::resource::EventType eventType) -> void {
      if (eventType == smtk::resource::EventType::ADDED)
      {
        if (const smtk::attribute::Resource* attributeResource =
              dynamic_cast<const smtk::attribute::Resource*>(&resource))
        {
          registerFunction.second(const_cast<smtk::attribute::Resource&>(*attributeResource));
        }
      }
    };

    // Associate the observer key with the rule type, so we can remove it
    // later if requested.
    m_associationObservers.insert(
      std::make_pair(registerFunction.first, manager->observers().insert(registerCustomTypeObserver,
                                               "Register attribute association rule.")));
  }

  // For each dissociation register function...
  for (auto& registerFunction : m_dissociationRegisterFunctions)
  {
    // ...add an observer that adds the new rule to all current and
    // future attribute resources associated with this manager.
    auto registerCustomTypeObserver = [=](
      const smtk::resource::Resource& resource, smtk::resource::EventType eventType) -> void {
      if (eventType == smtk::resource::EventType::ADDED)
      {
        if (const smtk::attribute::Resource* attributeResource =
              dynamic_cast<const smtk::attribute::Resource*>(&resource))
        {
          registerFunction.second(const_cast<smtk::attribute::Resource&>(*attributeResource));
        }
      }
    };

    // Associate the observer key with the rule type, so we can remove it
    // later if requested.
    m_dissociationObservers.insert(
      std::make_pair(registerFunction.first, manager->observers().insert(registerCustomTypeObserver,
                                               "Register attribute dissociation rule.")));
  }

  m_manager = manager;

  return true;
}
}
}
