//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/plugin/Manager.txx"

namespace
{
bool instantiateManager()
{
  return smtk::plugin::Manager::instance() != nullptr;
}
} // namespace

namespace smtk
{
namespace plugin
{
namespace detail
{
Manager::~Manager() = default;

void Manager::addClient(const std::weak_ptr<ClientBase>& pluginClient)
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
} // namespace detail
} // namespace plugin

bool instantiated = instantiateManager();
} // namespace smtk
