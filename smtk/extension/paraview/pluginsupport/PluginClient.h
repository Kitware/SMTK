//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_paraview_pluginsupport_PluginClient_h
#define __smtk_extension_paraview_pluginsupport_PluginClient_h

#include "smtk/common/Registry.h"
#include "smtk/extension/paraview/pluginsupport/PluginClientBase.h"

#include <functional>
#include <memory>
#include <unordered_set>

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace detail
{
template <typename Registrar, typename Manager>
class PluginClient;
}

/// PluginClients are static objects that live in a plugin. Their lifetime is
/// therefore confined to the lifetime to the plugin's own scope. Upon creation,
/// a PluginClient adds a weak pointer to itself to the singleton PluginManager.
/// The PluginManager has an API for registering/unregistering Managers to/from
/// each of its available PluginClients. When a Manager is registered to a
/// PluginClient, a Registry object is created; it tethers the manager
/// instance's extensions imbued by the plugin to its own lifetime. These
/// registry objects are held by the PluginClient, guaranteeing that they go out
/// of scope when the PluginClient goes out of scope.
///
/// A PluginClient is a composition of detail::PluginClients, one for each
/// Manager the plugin can augment. To facilitate the registration of managers
/// to a PluginClient without explicitly referring to the plugin's Registrar,
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

template <typename Registrar, typename Manager, typename... T>
class PluginClient : public detail::PluginClient<Registrar, Manager>,
                     public detail::PluginClient<Registrar, T>...,
                     public PluginClientBase
{
public:
  static std::shared_ptr<PluginClientBase> create();
  virtual ~PluginClient() {}

private:
  PluginClient()
    : detail::PluginClient<Registrar, Manager>()
    , detail::PluginClient<Registrar, T>()...
    , PluginClientBase()
  {
  }
};

#else

namespace detail
{
struct Sentinel
{
};
}

template <typename Registrar, typename Manager = detail::Sentinel, typename... T>
class PluginClient : public detail::PluginClient<Registrar, Manager>,
                     public PluginClient<Registrar, T...>
{
public:
  static std::shared_ptr<PluginClientBase> create();
  virtual ~PluginClient() {}

protected:
  PluginClient()
    : detail::PluginClient<Registrar, Manager>()
    , PluginClient<Registrar, T...>()
  {
  }
};

template <typename Registrar>
class PluginClient<Registrar, detail::Sentinel> : public PluginClientBase
{
public:
  PluginClient()
    : PluginClientBase()
  {
  }

  virtual ~PluginClient() {}
};

#endif

namespace detail
{
/// PluginClients are a composition of Registrars and Managers. When a developer
/// wishes to register a manager to all available plugins, the Registrar
/// for each plugin is not known but the Manager type is. We therefore present
/// an API that only depends on the Manager type to the user, and we implement
/// it in detail::PluginClient.
template <typename Manager>
class PluginClientFor
{
public:
  virtual ~PluginClientFor();

  virtual bool registerPluginTo(const std::shared_ptr<Manager>&) = 0;
  virtual bool unregisterPluginFrom(const std::shared_ptr<Manager>&) = 0;

private:
  virtual smtk::common::RegistryBase* find(const std::shared_ptr<Manager>&) = 0;

  std::unordered_set<smtk::common::RegistryBase*> m_registries;
};

template <typename Manager>
PluginClientFor<Manager>::~PluginClientFor()
{
  for (auto base_registry : m_registries)
  {
    delete base_registry;
  }
}

/// The "public" PluginClient is simply a composition of detail::PluginClients,
/// one for each Manager type. The detail::PluginClient constructs a Registry
/// object for its Registrar/Manager pair and stores it in its set of
/// Registries. The lifetime of these Registries are therefore tethered to the
/// lifetime of the PluginClient, which lives in the plugin's library.
template <typename Registrar, typename Manager>
class PluginClient : public PluginClientFor<Manager>
{
public:
  bool registerPluginTo(const std::shared_ptr<Manager>&) override;
  bool unregisterPluginFrom(const std::shared_ptr<Manager>&) override;

private:
  smtk::common::RegistryBase* find(const std::shared_ptr<Manager>&) override;

  std::unordered_set<smtk::common::RegistryBase*> m_registries;
};

template <typename Registrar, typename Manager>
smtk::common::RegistryBase* PluginClient<Registrar, Manager>::find(
  const std::shared_ptr<Manager>& manager)
{
  // We are looking for a registry that links the Registrar and Manager. If one
  // exists, it may be the same Registry object that is used to link the
  // Registrar and another Manager. That's ok, because Registries are in turn
  // compositions of Registrar/Manager pairs, so a dynamic_cast to a Registry
  // that only contains the queried Manager will still succeed.
  typedef smtk::common::Registry<Registrar, Manager> QueriedRegistry;
  for (auto base_registry : m_registries)
  {
    QueriedRegistry* registry = dynamic_cast<QueriedRegistry*>(base_registry);

    // To prevent cryptic compilation errors about a Registry object that is
    // linked to a Manager it doesn't support, Registries have the ability to
    // take as a template parameter a Manager that they do not support. Because
    // of this, we need to check if the accessed registry also supports the
    // Manager under query.
    if (registry != nullptr && registry->has(manager))
    {
      return base_registry;
    }
  }
  return nullptr;
}

template <typename Registrar, typename Manager>
bool PluginClient<Registrar, Manager>::registerPluginTo(const std::shared_ptr<Manager>& manager)
{
  // We search to ensure that a Registry object does not already exist for this
  // manager. That way, Registrars only register themselves to a manager once.
  if (this->find(manager) == nullptr)
  {
    auto val = m_registries.insert(new smtk::common::Registry<Registrar, Manager>(manager));
    return val.second;
  }
  return false;
}

template <typename Registrar, typename Manager>
bool PluginClient<Registrar, Manager>::unregisterPluginFrom(const std::shared_ptr<Manager>& manager)
{
  // We search for the Registry object that connects to this manager. If one
  // exists, we break the connection by deleting the registry.
  if (auto registry = this->find(manager))
  {
    m_registries.erase(registry);
    delete registry;
    return true;
  }
  return false;
}
}
}
}
}

#endif
