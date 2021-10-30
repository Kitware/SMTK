//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_query_Manager_h
#define smtk_resource_query_Manager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include <smtk/common/TypeName.h>

#include "smtk/resource/Manager.h"
#include "smtk/resource/Observer.h"

#include "smtk/resource/query/Queries.h"

#include <string>

namespace smtk
{
namespace resource
{
namespace query
{

/// A manager for registering Query types.
///
/// smtk::resource::query::Manager is an interface for registering Query types.
/// SMTK Plugins can interact with smtk::resource::query::Manager by adding
/// methods similar to the following to their Registrar:
///
/// registerTo(const smtk::resource::query::Manager::Ptr& manager)
/// {
///   manager->registerQuery<ApplicableResourceType, MyQuery>();
/// }
///
/// unregisterFrom(const smtk::resource::query::Manager::Ptr& manager)
/// {
///   manager->unregisterQuery<MyQuery>();
/// }
///
/// Additionally, the `smtk_add_plugin()` call for the plugin should be extended
/// to include `smtk::resource::query::Manager` in its list of managers.
/// Upon registration, all appropriate resources associated with the same
/// resource manager as the smtk::resource::query::Manager will be able to
/// construct instances of the newly registered Query.
class SMTKCORE_EXPORT Manager : public std::enable_shared_from_this<Manager>
{
public:
  smtkTypedefs(smtk::resource::query::Manager);

  static std::shared_ptr<Manager> create(const smtk::resource::ManagerPtr& resourceManager)
  {
    return smtk::shared_ptr<Manager>(new Manager(resourceManager));
  }

  // Our map of observer keys is move-only, so this class needs to be at least
  // move-only as well. MSVC 2019 does not correctly intuit this fact when
  // generating default constructors and assignment operators, so we explicitly
  // remove them. We remove the move constructor and move assignment operator
  // for good measure, since they are not needed anyway.
  Manager() = delete;
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;

  Manager& operator=(const Manager&) = delete;
  Manager& operator=(Manager&&) = delete;

  virtual ~Manager();

  /// Register <QueryType> to all resources for which the input functor <fn>
  /// returns true.
  template<typename QueryType>
  bool registerQueryIf(std::function<bool(smtk::resource::Resource&)> fn)
  {
    auto registerQueryType =
      [fn](const smtk::resource::Resource& rsrc, smtk::resource::EventType eventType) -> void {
      if (eventType == smtk::resource::EventType::ADDED)
      {
        smtk::resource::Resource& resource = const_cast<smtk::resource::Resource&>(rsrc);
        if (fn(resource))
        {
          resource.queries().registerQuery<QueryType>();
        }
      }
    };

    if (auto manager = m_manager.lock())
    {
      m_observers.insert(std::make_pair(
        typeid(QueryType).hash_code(),
        manager->observers().insert(
          registerQueryType,
          "Register query type <" + smtk::common::typeName<QueryType>() + ">.")));

      return true;
    }

    return false;
  }

  /// Register <QueryType> for all resources derived from <ResourceType>.
  template<typename ResourceType, typename QueryType>
  bool registerQuery()
  {
    return registerQueryIf<QueryType>(
      [](smtk::resource::Resource& resource) -> bool { return resource.isOfType<ResourceType>(); });
  }

  /// Unregister <QueryType> from all appropriate resources.
  template<typename QueryType>
  bool unregisterQuery()
  {
    if (auto manager = m_manager.lock())
    {
      manager->visit([&](Resource& resource) -> smtk::common::Processing {
        resource.queries().template unregisterQuery<QueryType>();
        return smtk::common::Processing::CONTINUE;
      });

      m_observers.erase(typeid(QueryType).hash_code());

      return true;
    }

    return false;
  }

  /// Register <QueryTypes> to all resources for which the input functor <fn>
  /// returns true.
  template<typename QueryTypes>
  bool registerQueriesIf(std::function<bool(smtk::resource::Resource&)> fn)
  {
    auto registerQueryTypes =
      [fn](const smtk::resource::Resource& rsrc, smtk::resource::EventType eventType) -> void {
      if (eventType == smtk::resource::EventType::ADDED)
      {
        smtk::resource::Resource& resource = const_cast<smtk::resource::Resource&>(rsrc);
        if (fn(resource))
        {
          resource.queries().registerQueries<QueryTypes>();
        }
      }
    };

    if (auto manager = m_manager.lock())
    {
      m_observers.insert(std::make_pair(
        typeid(QueryTypes).hash_code(),
        manager->observers().insert(
          registerQueryTypes,
          "Register query types <" + smtk::common::typeName<QueryTypes>() + ">.")));

      return true;
    }

    return false;
  }

  /// Register <QueryTypes> for all resources derived from <ResourceType>.
  template<typename ResourceType, typename QueryTypes>
  bool registerQueries()
  {
    return registerQueriesIf<QueryTypes>(
      [](smtk::resource::Resource& resource) -> bool { return resource.isOfType<ResourceType>(); });
  }

  /// Unregister <QueryTypes> from all appropriate resources.
  template<typename QueryTypes>
  bool unregisterQueries()
  {
    if (auto manager = m_manager.lock())
    {
      manager->visit([&](Resource& resource) -> smtk::common::Processing {
        resource.queries().template unregisterQueries<QueryTypes>();
        return smtk::common::Processing::CONTINUE;
      });

      m_observers.erase(typeid(QueryTypes).hash_code());

      return true;
    }

    return false;
  }

protected:
  Manager(const std::shared_ptr<smtk::resource::Manager>&);

  std::weak_ptr<smtk::resource::Manager> m_manager;

  std::unordered_map<std::size_t, smtk::resource::Observers::Key> m_observers;
};
} // namespace query
} // namespace resource
} // namespace smtk

#endif
