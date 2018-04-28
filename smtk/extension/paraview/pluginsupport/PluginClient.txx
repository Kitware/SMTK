//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_paraview_pluginsupport_PluginClient_txx
#define __smtk_extension_paraview_pluginsupport_PluginClient_txx

#include "smtk/extension/paraview/pluginsupport/PluginClient.h"
#include "smtk/extension/paraview/pluginsupport/PluginClientBase.h"
#include "smtk/extension/paraview/pluginsupport/PluginManager.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
template <typename Registrar, typename Manager, typename... T>
std::shared_ptr<PluginClientBase> PluginClient<Registrar, Manager, T...>::create()
{
  std::shared_ptr<PluginClient<Registrar, Manager, T...> > shared(
    new PluginClient<Registrar, Manager, T...>);
  smtk::extension::paraview::PluginManager::instance()->addPluginClient(
    std::static_pointer_cast<PluginClientBase>(shared->PluginClientBase::shared_from_this()));
  return std::static_pointer_cast<PluginClientBase>(shared);
}
}
}
}

#endif
