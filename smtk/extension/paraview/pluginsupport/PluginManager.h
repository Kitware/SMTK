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

  template <typename Manager>
  void registerPluginsTo(const std::shared_ptr<Manager>& manager)
  {
    setRegistryStatus(manager, true);
  }

  template <typename Manager>
  void unregisterPluginsFrom(const std::shared_ptr<Manager>& manager)
  {
    setRegistryStatus(manager, false);
  }

  void addPluginClient(const std::weak_ptr<PluginClientBase>& pluginClient);

private:
  template <typename Manager>
  void setRegistryStatus(const std::shared_ptr<Manager>&, bool);

  std::vector<std::weak_ptr<PluginClientBase> > m_clients;
};
}

/// The PluginManager is a singleton interface for registering available plugins
/// to manager instances. When a plugin is loaded, it creates a PluginClient
/// that adds itself to PluginManager's set of PluginClient weak_ptrs. When a
/// manager instance is subsequently passed into the PluginManager's
/// "registerPluginsTo" method, each PluginClient in the set that accepts a
/// manager of this type constructs a Registry that tethers the scope of the
/// manager's ability to use features from the plugin to the lifetime of the
/// Registry object.
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
