//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_ItemDefinitionManager_h
#define smtk_attribute_ItemDefinitionManager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
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

/// A manager for custom (i.e. user-defined) item definitions.
///
/// ItemDefinitionManager is an interface for registering user-defined item
/// definitions. SMTK Plugins can interact with the ItemDefinitionManager by
/// adding methods similar to the following to their Registrar:
///
/// registerTo(const smtk::attribute::ItemDefinitionManager::Ptr& manager)
/// {
///   manager->registerDefinition<MyCustomItemDefinition>();
/// }
///
/// unregisterFrom(const smtk::attribute::ItemDefinitionManager::Ptr& manager)
/// {
///   manager->unregisterDefinition<MyCustomItemDefinition>();
/// }
///
/// Additionally, the `smtk_add_plugin()` call for the plugin should be extended
/// to include `smtk::attribute::ItemDefinitionManager` in its list of managers.
/// Upon registration, attribute resources (a) associated with the same resource
/// manager as the ItemDefinitionManager or (b) constructed by an operation that
/// is managed by an operation manager with access to the ItemDefinitionManager
/// will be able to read, write and create attributes that contain the newly
/// registered custom items.
class SMTKCORE_EXPORT ItemDefinitionManager
  : public std::enable_shared_from_this<ItemDefinitionManager>
{
public:
  smtkTypedefs(smtk::attribute::ItemDefinitionManager);

  static std::shared_ptr<ItemDefinitionManager> create()
  {
    return smtk::shared_ptr<ItemDefinitionManager>(new ItemDefinitionManager());
  }

  // Our map of observer keys is move-only, so this class needs to be at least
  // move-only as well. MSVC 2019 does not correctly intuit this fact when
  // generating copy constructors and assignment operators, so we explicitly
  // remove them. We remove the move constructor and move assignment operator
  // for good measure, since they are not needed anyway.
  ItemDefinitionManager(const ItemDefinitionManager&) = delete;
  ItemDefinitionManager(ItemDefinitionManager&&) = delete;

  ItemDefinitionManager& operator=(const ItemDefinitionManager&) = delete;
  ItemDefinitionManager& operator=(ItemDefinitionManager&&) = delete;

  virtual ~ItemDefinitionManager();

  /// Register to a resource manager. All attribute resources managed by the
  /// resource manager (both preexisting and resources subsequently added to the
  /// resource manager) will be able to read, write and create attributes that
  /// contain definitions registered to this class.
  bool registerResourceManager(smtk::resource::Manager::Ptr&);

  /// Explicilty add the contained custom item definitions to an attribute
  /// resource.
  bool registerDefinitionsTo(smtk::attribute::Resource::Ptr& resource) const
  {
    for (const auto& registerFunction : m_registerFunctions)
    {
      registerFunction.second(*resource);
    }

    return true;
  }

  /// Register <CustomItemDefinitionType> to all attribute resources.
  template<typename CustomDefinitionType>
  bool registerDefinition()
  {
    // Construct a functor for adding the new definition type to an attribute
    // resource.
    auto registerCustomType = [](smtk::attribute::Resource& resource) {
      resource.customItemDefinitionFactory().registerType<CustomDefinitionType>();
    };

    // Add the functor to our container of register functions.
    m_registerFunctions.insert(
      std::make_pair(typeid(CustomDefinitionType).hash_code(), registerCustomType));

    // If we currently have an associated resource manager...
    if (auto manager = m_manager.lock())
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
            registerCustomType(const_cast<smtk::attribute::Resource&>(*attributeResource));
          }
        }
      };

      // Associate the observer key with the definition type, so we can remove it
      // later if requested.
      m_observers.insert(std::make_pair(
        typeid(CustomDefinitionType).hash_code(),
        manager->observers().insert(
          registerCustomTypeObserver,
          "Register custom attribute type <" + smtk::common::typeName<CustomDefinitionType>() +
            ">.")));
    }

    return true;
  }

  /// Unregister <CustomItemDefinitionType> from all attribute resources.
  template<typename CustomDefinitionType>
  bool unregisterDefinition()
  {
    // Remove the definiton from the container of register functions.
    m_registerFunctions.erase(typeid(CustomDefinitionType).hash_code());

    // Also, remove the observer associated with the definition, if it exists.
    m_observers.erase(typeid(CustomDefinitionType).hash_code());

    // If there is an associated resource manager...
    if (auto manager = m_manager.lock())
    {
      // ...remove the definition from all of its attribute resources.
      for (const auto& resource : manager->find<smtk::attribute::Resource>())
      {
        resource->customItemDefinitionFactory().unregisterType<CustomDefinitionType>();
      }
    }

    return true;
  }

  /// Register multiple definitions to all attribute resources.
  template<typename Tuple>
  bool registerDefinitions()
  {
    return ItemDefinitionManager::registerDefinitions<0, Tuple>();
  }

  /// Unregister multiple definitions from all attribute resources.
  template<typename Tuple>
  bool unregisterDefinitions()
  {
    return ItemDefinitionManager::unregisterDefinitions<0, Tuple>();
  }

protected:
  ItemDefinitionManager();
  ItemDefinitionManager(const std::shared_ptr<smtk::resource::Manager>&);

  template<std::size_t I, typename Tuple>
  typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type registerDefinitions()
  {
    bool registered = this->registerDefinition<typename std::tuple_element<I, Tuple>::type>();
    return registered && ItemDefinitionManager::registerDefinitions<I + 1, Tuple>();
  }

  template<std::size_t I, typename Tuple>
  typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type registerDefinitions()
  {
    return true;
  }

  template<std::size_t I, typename Tuple>
  typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type unregisterDefinitions()
  {
    bool unregistered = this->unregisterDefinition<typename std::tuple_element<I, Tuple>::type>();
    return unregistered && ItemDefinitionManager::unregisterDefinitions<I + 1, Tuple>();
  }

  template<std::size_t I, typename Tuple>
  typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type unregisterDefinitions()
  {
    return true;
  }

  std::unordered_map<std::size_t, std::function<void(smtk::attribute::Resource&)>>
    m_registerFunctions;

  std::weak_ptr<smtk::resource::Manager> m_manager;

  std::unordered_map<std::size_t, smtk::resource::Observers::Key> m_observers;
};
} // namespace attribute
} // namespace smtk

#endif
