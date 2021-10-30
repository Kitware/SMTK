//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_plugin_Manager_txx
#define smtk_plugin_Manager_txx

#include "smtk/plugin/Client.h"
#include "smtk/plugin/Manager.h"

namespace smtk
{
namespace plugin
{
namespace detail
{

template<typename Manager_t>
void Manager::setRegistryStatus(const std::shared_ptr<Manager_t>& manager, bool status)
{
  for (auto clientIt = m_clients.begin(); clientIt != m_clients.end();)
  {
    if (auto client = clientIt->lock())
    {
      // Cross cast to the Client's Manager-fixed API
      auto clientForManager = dynamic_cast<ClientFor<Manager_t>*>(client.get());
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
      }
      ++clientIt;
    }
    else
    {
      clientIt = m_clients.erase(clientIt);
    }
  }

  // If we are registering a manager to the existing plugins...
  if (status)
  {
    // ...then we also construct a functor for registering this manager to
    // future plugins. It accepts as input the plugin client and returns true is
    // the manager has not yet expired.
    std::weak_ptr<Manager_t> weakMgr = manager;
    auto registerToFuturePlugins = [=](const std::weak_ptr<ClientBase>& pluginClient) {
      if (auto manager = weakMgr.lock())
      {
        if (auto client = pluginClient.lock())
        {
          auto clientForManager = dynamic_cast<ClientFor<Manager_t>*>(client.get());
          if (clientForManager)
          {
            clientForManager->registerPluginTo(manager);
          }
        }
        return true;
      }
      else
      {
        return false;
      }
    };
    m_registerToExistingManagers.insert(registerToFuturePlugins);
  }
}
} // namespace detail
} // namespace plugin
} // namespace smtk

#endif
