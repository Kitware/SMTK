//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_plugin_Client_h
#define __smtk_plugin_Client_h

#include "smtk/plugin/ClientBase.h"
#include "smtk/plugin/Registry.h"

#include <functional>
#include <memory>
#include <unordered_set>

#ifdef SMTK_MSVC
#include "smtk/plugin/Sentinel.h"
#endif

namespace smtk
{
namespace plugin
{
namespace detail
{
template<typename Registrar, typename Manager>
class Client;
}

/// plugin::Clients are static objects that live in a plugin. Their lifetime is
/// therefore confined to the lifetime to the plugin's own scope. Upon creation,
/// a Client adds a weak pointer to itself to the singleton PluginManager.
/// The PluginManager has an API for registering/unregistering Managers to/from
/// each of its available Clients. When a Manager is registered to a
/// Client, a Registry object is created; it tethers the manager
/// instance's extensions imbued by the plugin to its own lifetime. These
/// registry objects are held by the Client, guaranteeing that they go out
/// of scope when the Client goes out of scope.
///
/// A Client is a composition of detail::Clients, one for each
/// Manager the plugin can augment. To facilitate the registration of managers
/// to a Client without explicitly referring to the plugin's Registrar,
/// the Manager-specific API is separated from the implementation that uses
/// the Registrar to perform the actual registration.
///
/// Both gcc and clang are savvy to template class descriptions using parameter
/// packs. MSVC isn't quite there yet. The following code is bifurcated
/// according to compiler type because, even though the two code paths produce
/// functionally equivalent code, the first implementation does so with much
/// less compile-time (and library size) overhead. Eventually, only the first
/// implementation should be used.

#ifndef SMTK_MSVC

template<typename Registrar, typename Manager, typename... T>
class SMTK_ALWAYS_EXPORT Client
  : public detail::Client<Registrar, Manager>
  , public detail::Client<Registrar, T>...
  , public ClientBase
{
public:
  static std::shared_ptr<ClientBase> create();
  ~Client() override {}

private:
  Client()
    : detail::Client<Registrar, Manager>()
    , detail::Client<Registrar, T>()...
    , ClientBase()
  {
  }
};

#else

template<typename Registrar, typename Manager = detail::Sentinel, typename... T>
class SMTK_ALWAYS_EXPORT Client
  : public detail::Client<Registrar, Manager>
  , public Client<Registrar, T...>
{
public:
  static std::shared_ptr<ClientBase> create();
  ~Client() override {}

protected:
  Client()
    : detail::Client<Registrar, Manager>()
    , Client<Registrar, T...>()
  {
  }
};

template<typename Registrar>
class SMTK_ALWAYS_EXPORT Client<Registrar, detail::Sentinel> : public ClientBase
{
public:
  Client()
    : ClientBase()
  {
  }

  ~Client() override {}
};

#endif

namespace detail
{
/// Clients are a composition of Registrars and Managers. When a developer
/// wishes to register a manager to all available plugins, the Registrar
/// for each plugin is not known but the Manager type is. We therefore present
/// an API that only depends on the Manager type to the user, and we implement
/// it in detail::Client.
template<typename Manager>
class SMTK_ALWAYS_EXPORT ClientFor
{
public:
  virtual ~ClientFor();

  virtual bool registerPluginTo(const std::shared_ptr<Manager>&) = 0;
  virtual bool unregisterPluginFrom(const std::shared_ptr<Manager>&) = 0;

protected:
  virtual smtk::plugin::RegistryBase* find(const std::shared_ptr<Manager>&) = 0;

  std::unordered_set<smtk::plugin::RegistryBase*> m_registries;
};

template<typename Manager>
ClientFor<Manager>::~ClientFor()
{
  for (auto base_registry : m_registries)
  {
    delete base_registry;
  }
}

/// The "public" Client is simply a composition of detail::Clients,
/// one for each Manager type. The detail::Client constructs a Registry
/// object for its Registrar/Manager pair and stores it in its set of
/// Registries. The lifetime of these Registries are therefore tethered to the
/// lifetime of the Client, which lives in the plugin's library.
template<typename Registrar, typename Manager>
class SMTK_ALWAYS_EXPORT Client : public ClientFor<Manager>
{
public:
  bool registerPluginTo(const std::shared_ptr<Manager>&) override;
  bool unregisterPluginFrom(const std::shared_ptr<Manager>&) override;

private:
  smtk::plugin::RegistryBase* find(const std::shared_ptr<Manager>&) override;
};

template<typename Registrar, typename Manager>
smtk::plugin::RegistryBase* Client<Registrar, Manager>::find(
  const std::shared_ptr<Manager>& manager)
{
  // We are looking for a registry that links the Registrar and Manager. If one
  // exists, it may be the same Registry object that is used to link the
  // Registrar and another Manager. That's ok, because Registries are in turn
  // compositions of Registrar/Manager pairs, so a dynamic_cast to a Registry
  // that only contains the queried Manager will still succeed.
  typedef smtk::plugin::Registry<Registrar, Manager> QueriedRegistry;
  for (auto base_registry : ClientFor<Manager>::m_registries)
  {
    QueriedRegistry* registry = dynamic_cast<QueriedRegistry*>(base_registry);

    // To prevent cryptic compilation errors about a Registry object that is
    // linked to a Manager it doesn't support, Registries have the ability to
    // take as a template parameter a Manager that they do not support. Because
    // of this, we need to check if the accessed registry also supports the
    // Manager under query.
    if (registry != nullptr && registry->contains(manager))
    {
      return base_registry;
    }
  }
  return nullptr;
}

template<typename Registrar, typename Manager>
bool Client<Registrar, Manager>::registerPluginTo(const std::shared_ptr<Manager>& manager)
{
  // We search to ensure that a Registry object does not already exist for this
  // manager. That way, Registrars only register themselves to a manager once.
  if (this->find(manager) == nullptr)
  {
    auto val = ClientFor<Manager>::m_registries.insert(
      new smtk::plugin::Registry<Registrar, Manager>(manager));
    return val.second;
  }
  return false;
}

template<typename Registrar, typename Manager>
bool Client<Registrar, Manager>::unregisterPluginFrom(const std::shared_ptr<Manager>& manager)
{
  // We search for the Registry object that connects to this manager. If one
  // exists, we break the connection by deleting the registry.
  if (auto registry = this->find(manager))
  {
    ClientFor<Manager>::m_registries.erase(registry);
    delete registry;
    return true;
  }
  return false;
}
} // namespace detail
} // namespace plugin
} // namespace smtk

#endif
