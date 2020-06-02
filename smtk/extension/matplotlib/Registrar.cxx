//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/extension/matplotlib/Registrar.h"

#include "smtk/operation/RegisterPythonOperations.h"

namespace smtk
{
namespace extension
{
namespace matplotlib
{

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::operation::registerPythonOperations(
    operationManager, "smtk.extension.matplotlib.render_mesh");
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperation("smtk.extension.matplotlib.render_mesh.render_mesh");
}
}
}
}
