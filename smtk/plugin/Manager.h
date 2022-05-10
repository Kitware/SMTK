//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_plugin_Manager_h
#define smtk_plugin_Manager_h

#include "smtk/CoreExports.h"

#include "smtk/plugin/ClientBase.h"

#include "smtk/SharedFromThis.h"
#include "smtk/common/Singleton.h"

#include <set>
#include <vector>

namespace smtk
{
/// A subsystem for extending the functionality of SMTK at run time.
namespace plugin
{
namespace detail
{

class SMTKCORE_EXPORT Manager
{
public:
  smtkTypedefs(smtk::plugin::detail::Manager);
  smtkCreateMacro(Manager);

  virtual ~Manager();

  /// Register all current and future plugins to this manager.
  template<typename Manager_t>
  void registerPluginsTo(const std::shared_ptr<Manager_t>& manager)
  {
    setRegistryStatus(manager, true);
  }

  /// Unregister all current plugins from this manager, and do not register
  /// future plugins to it either.
  template<typename Manager_t>
  void unregisterPluginsFrom(const std::shared_ptr<Manager_t>& manager)
  {
    setRegistryStatus(manager, false);
  }

  /// Add a plugin client to register to existing and future managers.
  void addClient(const std::weak_ptr<ClientBase>& pluginClient);

private:
  template<typename Manager_t>
  void setRegistryStatus(const std::shared_ptr<Manager_t>&, bool);

  std::vector<std::weak_ptr<ClientBase>> m_clients;

  /// The register function accepts a plugin client and registers it to the
  /// manager associated with the function (a weak_ptr is captured by value
  /// within the function). Returns true if the manager has not yet expired,
  /// regardless of the success of the registration process.
  typedef std::function<bool(const std::weak_ptr<ClientBase>&)> RegisterFunction;

  struct fn_compare
  {
    bool operator()(const RegisterFunction& lhs, const RegisterFunction& rhs) const
    {
      return &lhs < &rhs;
    }
  };

  std::set<RegisterFunction, fn_compare> m_registerToExistingManagers;
};
} // namespace detail

/// The Manager is a singleton interface for registering available plugins
/// to manager instances. When a plugin is loaded, it creates a Client
/// that adds itself to Manager's set of Client weak_ptrs. When a
/// manager instance is passed into the Manager's "registerPluginsTo"
/// method, each Client in the set that accepts a manager of this type
/// constructs a Registry that tethers the scope of the manager's ability to use
/// features from the plugin to the lifetime of the Registry object.
/// Additionally, when a manager is passed into this class a register function
/// is created that facilitates the registration of future plugins to the
/// manager.
typedef smtk::common::Singleton<detail::Manager> Manager;
} // namespace plugin
} // namespace smtk

#ifndef smtkCore_EXPORTS
extern
#endif
  template class SMTKCORE_EXPORT smtk::common::Singleton<smtk::plugin::detail::Manager>;
#endif

#include "smtk/plugin/Manager.txx"
