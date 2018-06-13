//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/pluginsupport/PluginClient.txx"
#include "smtk/mesh/resource/Registrar.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#include <memory>

typedef smtk::extension::paraview::PluginClient<smtk::mesh::Registrar, smtk::resource::Manager,
  smtk::operation::Manager>
  MeshPluginClient;

static std::shared_ptr<MeshPluginClient> mesh_client =
  std::dynamic_pointer_cast<MeshPluginClient>(MeshPluginClient::create());
