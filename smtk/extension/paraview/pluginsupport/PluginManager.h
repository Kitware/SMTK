//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_paraview_pluginsupport_PluginManager_h
#define __smtk_extension_paraview_pluginsupport_PluginManager_h

#include "smtk/extension/paraview/pluginsupport/Exports.h"
#include "smtk/extension/paraview/pluginsupport/PluginClientBase.h"

#include "smtk/SharedFromThis.h"
#include "smtk/common/Singleton.h"

#include <set>
#include <vector>

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace detail
{

class SMTKPLUGINSUPPORT_EXPORT PluginManager
{
public:
  smtkTypedefs(smtk::extension::paraview::detail::PluginManager);
  smtkCreateMacro(PluginManager);

  virtual ~PluginManager();

  /// Register all current and future plugins to this manager.
  template <typename Manager>
  void registerPluginsTo(const std::shared_ptr<Manager>& manager)
  {
    setRegistryStatus(manager, true);
  }

  /// Unregister all current plugins from this manager, and do not register
  /// future plugins to it either.
  template <typename Manager>
  void unregisterPluginsFrom(const std::shared_ptr<Manager>& manager)
  {
    setRegistryStatus(manager, false);
  }

  /// Add a plugin client to register to existing and future managers.
  void addPluginClient(const std::weak_ptr<PluginClientBase>& pluginClient);

private:
  template <typename Manager>
  void setRegistryStatus(const std::shared_ptr<Manager>&, bool);

  std::vector<std::weak_ptr<PluginClientBase> > m_clients;

  /// The register function accepts a plugin client and registers it to the
  /// manager associated with the function (a weak_ptr is captured by value
  /// within the function). Returns true if the manager has not yet expired,
  /// regardless of the success of the registration process.
  typedef std::function<bool(const std::weak_ptr<PluginClientBase>&)> RegisterFunction;

  struct fn_compare
  {
    bool operator()(const RegisterFunction& lhs, const RegisterFunction& rhs) const
    {
      return lhs.target<bool (*)(const std::weak_ptr<PluginClientBase>&)>() <
        rhs.target<bool (*)(const std::weak_ptr<PluginClientBase>&)>();
    }
  };

  std::set<RegisterFunction, fn_compare> m_registerToExistingManagers;
};
}

/// The PluginManager is a singleton interface for registering available plugins
/// to manager instances. When a plugin is loaded, it creates a PluginClient
/// that adds itself to PluginManager's set of PluginClient weak_ptrs. When a
/// manager instance is passed into the PluginManager's "registerPluginsTo"
/// method, each PluginClient in the set that accepts a manager of this type
/// constructs a Registry that tethers the scope of the manager's ability to use
/// features from the plugin to the lifetime of the Registry object.
/// Additionally, when a manager is passed into this class a register function
/// is created that facilitates the registration of future plugins to the
/// manager.
typedef smtk::common::Singleton<detail::PluginManager> PluginManager;
}
}
}

#ifndef smtkPluginSupport_EXPORTS
extern
#endif
  template class SMTKPLUGINSUPPORT_EXPORT
    smtk::common::Singleton<smtk::extension::paraview::detail::PluginManager>;
#endif
