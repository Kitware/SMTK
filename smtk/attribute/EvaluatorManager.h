//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_EvaluatorManager_h
#define __smtk_attribute_EvaluatorManager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/Resource.h"

#include "smtk/common/TypeName.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Observer.h"

#include <memory>

namespace smtk
{
namespace attribute
{

// A manager for registering Evaluators to attribute resources'
// EvaluatorFactory
//
// EvaluatorManager is an interface for registering Evaluators. SMTK Plugins can
// interact with the EvaluatorManager by adding methods similar to the following
// to their Registrar:
//
// registerTo(const smtk::attribute::EvaluatorManager::Ptr& manager)
// {
//   manager->registerEvaluator<EvaluatorT>("alias");
// }
//
// unregisterFrom(const smtk::attribute::EvaluatorManager::Ptr& manager)
// {
//   manager->unregisterEvaluator<EvaluatorT>();
// }
//
// Additionally, the `smtk_add_plugin()` call for the plugin should be extended
// to include `smtk::attribute::EvaluatorManager` in its list of managers. Upon
// registration, attribute resources (a) associated with the same resource
// manager as the EvaluatorManager or (b) constructed by an operation that is
// managed by an operation manager with access to the EvaluatorManager will be
// able to create Evaluators for Attributes whose Definitions are associated to
// the registered Evalators.
class SMTKCORE_EXPORT EvaluatorManager : public std::enable_shared_from_this<EvaluatorManager>
{
public:
  smtkTypedefs(smtk::attribute::EvaluatorManager);

  static std::shared_ptr<EvaluatorManager> create()
  {
    return smtk::shared_ptr<EvaluatorManager>(new EvaluatorManager());
  }

  EvaluatorManager(const EvaluatorManager&) = delete;
  EvaluatorManager(EvaluatorManager&&) = delete;

  EvaluatorManager& operator=(const EvaluatorManager&) = delete;
  EvaluatorManager& operator=(EvaluatorManager&&) = delete;

  virtual ~EvaluatorManager();

  // Registers to a resource manager. All attrribute resources managed by the
  // resource manager (present and future) will be able to create Evalators for
  // Attributes whose Definitions are associated to the registered Evaluators.
  bool registerResourceManager(smtk::resource::Manager::Ptr&);

  // Explicitly add the contained Evaluator types to an attribute resource.
  bool registerEvaluatorsTo(smtk::attribute::Resource::Ptr& resource) const
  {
    for (const auto& registerFunction : m_registerFunctions)
    {
      registerFunction.second(*resource);
    }

    return true;
  }

  // Registers <EvaluatorType> to all attribute resources.
  template<typename EvaluatorType>
  bool registerEvaluator(const std::string& alias)
  {
    // Construct a functor for adding the Evaluator type to an attribute
    // resource.
    auto registerEvaluatorType = [alias](smtk::attribute::Resource& resource) {
      resource.evaluatorFactory().registerEvaluator<EvaluatorType>(alias);
    };

    // Add the functor to our container of register functions.
    m_registerFunctions.insert(
      std::make_pair(typeid(EvaluatorType).hash_code(), registerEvaluatorType));

    // If we currently have an associated resource manager...
    if (auto manager = m_manager.lock())
    {
      // ...add an observer that adds the Evaluator to all current and future
      // attribute resources associated with this manager.
      auto registerEvaluatorObserver = [registerEvaluatorType](
                                         const smtk::resource::Resource& resource,
                                         smtk::resource::EventType eventType) -> void {
        if (eventType == smtk::resource::EventType::ADDED)
        {
          if (
            const smtk::attribute::Resource* attributeResource =
              dynamic_cast<const smtk::attribute::Resource*>(&resource))
          {
            registerEvaluatorType(const_cast<smtk::attribute::Resource&>(*attributeResource));
          }
        }
      };

      // Associated the observer key with the Evaluator type, so we can remove
      // it later if requested.
      m_observers.insert(std::make_pair(
        typeid(EvaluatorType).hash_code(),
        manager->observers().insert(
          registerEvaluatorObserver,
          "Register Evaluator type <" + smtk::common::typeName<EvaluatorType>() + "> with alias " +
            alias + ".")));
    }

    return true;
  }

  // Unregister <EvaluatorType> from all attribute resources.
  template<typename EvaluatorType>
  bool unregisterEvaluator()
  {
    // Remove the Evaluator from the container of register functions.
    m_registerFunctions.erase(typeid(EvaluatorType).hash_code());

    // Also, remove the observer associated with the Evaluator, if it exists.
    m_observers.erase(typeid(EvaluatorType).hash_code());

    // If there is an associated resource manager...
    if (auto manager = m_manager.lock())
    {
      for (const auto& resource : manager->find<smtk::attribute::Resource>())
      {
        // ...remove the Evaluator from all of its attribute resources.
        resource->evaluatorFactory().unregisterEvaluator<EvaluatorType>();
      }
    }

    return true;
  }

protected:
  EvaluatorManager();
  EvaluatorManager(const smtk::resource::ManagerPtr&);

  std::unordered_map<std::size_t, std::function<void(smtk::attribute::Resource&)>>
    m_registerFunctions;

  std::weak_ptr<smtk::resource::Manager> m_manager;

  std::unordered_map<std::size_t, smtk::resource::Observers::Key> m_observers;
};

} // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_EvaluatorManager_h
