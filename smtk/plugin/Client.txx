//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_plugin_Client_txx
#define smtk_plugin_Client_txx

#include "smtk/plugin/Client.h"
#include "smtk/plugin/ClientBase.h"
#include "smtk/plugin/Manager.h"

namespace smtk
{
namespace plugin
{
template<typename Registrar_t, typename Manager_t, typename... T>
std::shared_ptr<ClientBase> Client<Registrar_t, Manager_t, T...>::create()
{
  std::shared_ptr<Client<Registrar_t, Manager_t, T...>> shared(
    new Client<Registrar_t, Manager_t, T...>);
  smtk::plugin::Manager::instance()->addClient(
    std::static_pointer_cast<ClientBase>(shared->ClientBase::shared_from_this()));
  return std::static_pointer_cast<ClientBase>(shared);
}
} // namespace plugin
} // namespace smtk

#endif
