//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_AssociationRuleManager_h
#define __smtk_attribute_AssociationRuleManager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/AssociationRule.h"
#include "smtk/attribute/AssociationRuleFactory.h"
#include "smtk/attribute/AssociationRules.h"
#include "smtk/attribute/Resource.h"

#include <smtk/common/TypeName.h>

#include "smtk/resource/Manager.h"
#include "smtk/resource/Observer.h"

#include <numeric>
#include <string>

namespace smtk
{
namespace attribute
{

/// A Manager for registering/unregistering Association and Dissocation rule
/// types.
class SMTKCORE_EXPORT AssociationRuleManager
  : public std::enable_shared_from_this<AssociationRuleManager>
{
  template<typename BaseRuleType, bool = true>
  struct Trait;

  template<typename BaseRuleType, bool>
  friend struct Trait;

  template<bool dummy>
  struct Trait<AssociationRule, dummy>
  {
    static AssociationRuleFactory& factory(smtk::attribute::Resource& resource)
    {
      return resource.associationRules().associationRuleFactory();
    }
    static std::unordered_map<std::size_t, std::function<void(smtk::attribute::Resource&)>>&
    registerFunctions(AssociationRuleManager& ruleManager)
    {
      return ruleManager.m_associationRegisterFunctions;
    }
    static std::unordered_map<std::size_t, smtk::resource::Observers::Key>& observers(
      AssociationRuleManager& ruleManager)
    {
      return ruleManager.m_associationObservers;
    }
  };

  template<bool dummy>
  struct Trait<DissociationRule, dummy>
  {
    static DissociationRuleFactory& factory(smtk::attribute::Resource& resource)
    {
      return resource.associationRules().dissociationRuleFactory();
    }
    static std::unordered_map<std::size_t, std::function<void(smtk::attribute::Resource&)>>&
    registerFunctions(AssociationRuleManager& ruleManager)
    {
      return ruleManager.m_dissociationRegisterFunctions;
    }
    static std::unordered_map<std::size_t, smtk::resource::Observers::Key>& observers(
      AssociationRuleManager& ruleManager)
    {
      return ruleManager.m_dissociationObservers;
    }
  };

public:
  smtkTypedefs(smtk::attribute::AssociationRuleManager);

  static std::shared_ptr<AssociationRuleManager> create()
  {
    return smtk::shared_ptr<AssociationRuleManager>(new AssociationRuleManager());
  }

  // Our map of observer keys is move-only, so this class needs to be at least
  // move-only as well. MSVC 2019 does not correctly intuit this fact when
  // generating copy constructors and assignment operators, so we explicitly
  // remove them. We remove the move constructor and move assignment operator
  // for good measure, since they are not needed anyway.
  AssociationRuleManager(const AssociationRuleManager&) = delete;
  AssociationRuleManager(AssociationRuleManager&&) = delete;

  AssociationRuleManager& operator=(const AssociationRuleManager&) = delete;
  AssociationRuleManager& operator=(AssociationRuleManager&&) = delete;

  virtual ~AssociationRuleManager();

  /// Register to a resource manager. All attribute resources managed by the
  /// resource manager (both preexisting and resources subsequently added to the
  /// resource manager) will be able to read, write and create attributes that
  /// contain rules registered to this class.
  bool registerResourceManager(smtk::resource::Manager::Ptr&);

  /// Explicilty add the contained custom rules to an attribute resource.
  bool registerRulesTo(smtk::attribute::Resource::Ptr& resource) const
  {
    for (const auto& registerFunction : m_associationRegisterFunctions)
    {
      registerFunction.second(*resource);
    }

    for (const auto& registerFunction : m_dissociationRegisterFunctions)
    {
      registerFunction.second(*resource);
    }

    return true;
  }

  /// Register a custom association rule type with a given alias.
  template<typename CustomRuleType>
  bool registerAssociationRule(const std::string& alias)
  {
    return registerRule<AssociationRule, CustomRuleType>(alias);
  }

  /// Register a custom dissociation rule type with a given alias.
  template<typename CustomRuleType>
  bool registerDissociationRule(const std::string& alias)
  {
    return registerRule<DissociationRule, CustomRuleType>(alias);
  }

  /// Unregister a custom association rule type.
  template<typename CustomRuleType>
  bool unregisterAssociationRule()
  {
    return unregisterRule<AssociationRule, CustomRuleType>();
  }

  /// Unregister a custom dissociation rule type.
  template<typename CustomRuleType>
  bool unregisterDissociationRule()
  {
    return unregisterRule<DissociationRule, CustomRuleType>();
  }

protected:
  AssociationRuleManager();
  AssociationRuleManager(const std::shared_ptr<smtk::resource::Manager>&);

  /// Register <CustomRuleType> to all attribute resources.
  template<typename BaseRuleType, typename CustomRuleType>
  bool registerRule(const std::string& alias)
  {
    // Construct a functor for adding the new rule type to an attribute
    // resource.
    auto registerCustomRuleType = [alias](smtk::attribute::Resource& resource) {
      Trait<BaseRuleType>::factory(resource).template registerType<CustomRuleType>();
      Trait<BaseRuleType>::factory(resource).template addAlias<CustomRuleType>(alias);
    };

    // Add the functor to our container of register functions.
    Trait<BaseRuleType>::registerFunctions(*this).insert(
      std::make_pair(typeid(CustomRuleType).hash_code(), registerCustomRuleType));

    // If we currently have an associated resource manager...
    if (auto manager = m_manager.lock())
    {
      // ...add an observer that adds the new rule to all current and
      // future attribute resources associated with this manager.
      auto registerCustomRuleTypeObserver =
        [=](const smtk::resource::Resource& resource, smtk::resource::EventType eventType) -> void {
        if (eventType == smtk::resource::EventType::ADDED)
        {
          if (
            const smtk::attribute::Resource* attributeResource =
              dynamic_cast<const smtk::attribute::Resource*>(&resource))
          {
            registerCustomRuleType(const_cast<smtk::attribute::Resource&>(*attributeResource));
          }
        }
      };

      // Associate the observer key with the rule type, so we can remove it
      // later if requested.
      Trait<BaseRuleType>::observers(*this).insert(std::make_pair(
        typeid(CustomRuleType).hash_code(),
        manager->observers().insert(
          registerCustomRuleTypeObserver,
          "Register custom attribute rule <" + smtk::common::typeName<CustomRuleType>() + ">.")));
    }

    return true;
  }

  /// Unregister <CustomRuleType> from all attribute resources.
  template<typename BaseRuleType, typename CustomRuleType>
  bool unregisterRule()
  {
    // Remove the rule from the container of register functions.
    Trait<BaseRuleType>::registerFunctions(*this).erase(typeid(CustomRuleType).hash_code());

    // Also, remove the observer associated with the rule, if it exists.
    Trait<BaseRuleType>::observers(*this).erase(typeid(CustomRuleType).hash_code());

    // If there is an associated resource manager...
    if (auto manager = m_manager.lock())
    {
      // ...remove the rule from all of its attribute resources.
      for (const auto& resource : manager->find<smtk::attribute::Resource>())
      {
        Trait<BaseRuleType>::factory(*resource).template unregisterType<CustomRuleType>();
      }
    }

    return true;
  }

  std::weak_ptr<smtk::resource::Manager> m_manager;

  std::unordered_map<std::size_t, std::function<void(smtk::attribute::Resource&)>>
    m_associationRegisterFunctions;
  std::unordered_map<std::size_t, std::function<void(smtk::attribute::Resource&)>>
    m_dissociationRegisterFunctions;

  std::unordered_map<std::size_t, smtk::resource::Observers::Key> m_associationObservers;
  std::unordered_map<std::size_t, smtk::resource::Observers::Key> m_dissociationObservers;
};
} // namespace attribute
} // namespace smtk

#endif
