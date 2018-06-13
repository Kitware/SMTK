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
#include "smtk/model/Registrar.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#include <memory>

typedef smtk::extension::paraview::PluginClient<smtk::model::Registrar, smtk::resource::Manager,
  smtk::operation::Manager>
  ModelPluginClient;

static std::shared_ptr<ModelPluginClient> model_client =
  std::dynamic_pointer_cast<ModelPluginClient>(ModelPluginClient::create());
