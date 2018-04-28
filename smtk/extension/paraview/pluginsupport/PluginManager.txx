//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_paraview_pluginsupport_PluginManager_txx
#define __smtk_extension_paraview_pluginsupport_PluginManager_txx

#include "smtk/extension/paraview/pluginsupport/PluginClient.h"
#include "smtk/extension/paraview/pluginsupport/PluginManager.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace detail
{

template <typename Manager>
void PluginManager::setRegistryStatus(const std::shared_ptr<Manager>& manager, bool status)
{
  for (auto clientIt = m_clients.begin(); clientIt != m_clients.end();)
  {
    if (auto client = clientIt->lock())
    {
      // Cross cast to the PluginClient's Manager-fixed API
      auto clientForManager = dynamic_cast<PluginClientFor<Manager>*>(client.get());
      if (clientForManager)
      {
        if (status)
        {
          clientForManager->registerPluginTo(manager);
        }
        else
        {
          clientForManager->unregisterPluginFrom(manager);
        }

        ++clientIt;
      }
    }
    else
    {
      clientIt = m_clients.erase(clientIt);
    }
  }
}
}
}
}
}

#endif
