//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/mesh/Registrar.h"
#include "smtk/extension/paraview/pluginsupport/PluginClient.txx"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#include <memory>

// Our plugin client's type definition is a mouthful. It accepts as its first
// template parameter a Registrar, which has "registerTo" and "unregisterFrom"
// methods for each manager to/from which it registers/unregisters our session.
// The remaining template parameters are the managers that the session uses.
typedef smtk::extension::paraview::PluginClient<smtk::bridge::mesh::Registrar,
  smtk::resource::Manager, smtk::operation::Manager>
  MeshPluginClient;

// We create our plugin as a static shared pointer. Upon construction, it adds
// a weak pointer to itself to the singleton
// smtk::extension::paraview::PluginManager. When the session's underlying
// library goes out of scope, the shared pointer is deleted and the weak pointer
// held by the plugin manager is no longer accessible. Upon deletion, the
// Registry objects held by the PluginClient are destroyed and the Registrar's
// unregister methods are called for each manager to which it was registered.
// The PluginManager's weak pointer becomes invalid, so subsequent calls to
// register/unregister managers through the PluginManager avoid calling methods
// that are no longer in memory.
static std::shared_ptr<MeshPluginClient> mesh_client =
  std::dynamic_pointer_cast<MeshPluginClient>(MeshPluginClient::create());
