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
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/resource/Manager.h"

#include <memory>

typedef smtk::extension::paraview::PluginClient<smtk::operation::Registrar, smtk::resource::Manager,
  smtk::operation::Manager>
  OperationPluginClient;

static std::shared_ptr<OperationPluginClient> operation_client =
  std::dynamic_pointer_cast<OperationPluginClient>(OperationPluginClient::create());
