//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/pluginsupport/PluginManager.txx"

namespace
{
bool instantiatePluginManager()
{
  return smtk::extension::paraview::PluginManager::instance() != nullptr;
}
}

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace detail
{
PluginManager::~PluginManager() = default;

void PluginManager::addPluginClient(const std::weak_ptr<PluginClientBase>& pluginClient)
{
  m_clients.push_back(pluginClient);

  // Loop through the set of functions associated with extant managers and
  // attempt to register this plugin to those managers. If the manager
  // associated with the function has expired, remove it from the set.
  for (auto it = m_registerToExistingManagers.begin(); it != m_registerToExistingManagers.end();)
  {
    if ((*it)(pluginClient))
    {
      ++it;
    }
    else
    {
      m_registerToExistingManagers.erase(it++);
    }
  }
}
}

bool instantiated = instantiatePluginManager();
}
}
}
