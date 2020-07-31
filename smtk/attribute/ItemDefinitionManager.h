//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_ItemDefinitionManager_h
#define __smtk_attribute_ItemDefinitionManager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Resource.h"

#include <smtk/common/TypeName.h>

#include "smtk/resource/Manager.h"
#include "smtk/resource/Observer.h"

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
/// Upon registration, attribute resources associated with the same resource
/// manager as the ItemDefinitionManager will be able to read, write and create
/// attributes that contain the newly registered custom items.
class SMTKCORE_EXPORT ItemDefinitionManager
  : public std::enable_shared_from_this<ItemDefinitionManager>
{
public:
  smtkTypedefs(smtk::attribute::ItemDefinitionManager);

  static std::shared_ptr<ItemDefinitionManager> create(
    const smtk::resource::ManagerPtr& resourceManager)
  {
    return smtk::shared_ptr<ItemDefinitionManager>(new ItemDefinitionManager(resourceManager));
  }

  // Our map of observer keys is move-only, so this class needs to be at least
  // move-only as well. MSVC 2019 does not correctly intuit this fact when
  // generating default constructors and assignment operators, so we explicitly
  // remove them. We remove the move constructor and move assignment operator
  // for good measure, since they are not needed anyway.
  ItemDefinitionManager() = delete;
  ItemDefinitionManager(const ItemDefinitionManager&) = delete;
  ItemDefinitionManager(ItemDefinitionManager&&) = delete;

  ItemDefinitionManager& operator=(const ItemDefinitionManager&) = delete;
  ItemDefinitionManager& operator=(ItemDefinitionManager&&) = delete;

  virtual ~ItemDefinitionManager();

  /// Register <CustomItemDefinitionType> to all attribute resources.
  template <typename CustomDefinitionType>
  bool registerDefinition()
  {
    auto registerCustomType = [](
      const smtk::resource::Resource& resource, smtk::resource::EventType eventType) -> void {
      if (eventType == smtk::resource::EventType::ADDED)
      {
        if (const smtk::attribute::Resource* attributeResource =
              dynamic_cast<const smtk::attribute::Resource*>(&resource))
        {
          const_cast<smtk::attribute::Resource*>(attributeResource)
            ->customItemDefinitionFactory()
            .registerType<CustomDefinitionType>();
        }
      }
    };

    if (auto manager = m_manager.lock())
    {
      m_observers.insert(std::make_pair(typeid(CustomDefinitionType).hash_code(),
        manager->observers().insert(registerCustomType, "Register custom attribute type <" +
            smtk::common::typeName<CustomDefinitionType>() + ">.")));

      return true;
    }

    return false;
  }

  /// Unregister <CustomItemDefinitionType> from all attribute resources.
  template <typename CustomDefinitionType>
  bool unregisterDefinition()
  {
    if (auto manager = m_manager.lock())
    {
      for (auto resource : manager->find<smtk::attribute::Resource>())
      {
        resource->customItemDefinitionFactory().unregisterType<CustomDefinitionType>();
      }

      m_observers.erase(typeid(CustomDefinitionType).hash_code());

      return true;
    }

    return false;
  }

  /// Register multiple definitions to all attribute resources.
  template <typename Tuple>
  bool registerDefinitions()
  {
    return ItemDefinitionManager::registerDefinitions<0, Tuple>();
  }

  /// Unregister multiple definitions from all attribute resources.
  template <typename Tuple>
  bool unregisterDefinitions()
  {
    return ItemDefinitionManager::unregisterDefinitions<0, Tuple>();
  }

protected:
  ItemDefinitionManager(const std::shared_ptr<smtk::resource::Manager>&);

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  registerDefinitions()
  {
    bool registered = this->registerDefinition<typename std::tuple_element<I, Tuple>::type>();
    return registered && ItemDefinitionManager::registerDefinitions<I + 1, Tuple>();
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  registerDefinitions()
  {
    return true;
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  unregisterDefinitions()
  {
    bool unregistered = this->unregisterDefinition<typename std::tuple_element<I, Tuple>::type>();
    return unregistered && ItemDefinitionManager::unregisterDefinitions<I + 1, Tuple>();
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  unregisterDefinitions()
  {
    return true;
  }

  std::weak_ptr<smtk::resource::Manager> m_manager;

  std::unordered_map<std::size_t, smtk::resource::Observers::Key> m_observers;
};
}
}

#endif
