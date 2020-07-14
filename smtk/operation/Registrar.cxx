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
#include "smtk/operation/Registrar.h"

#ifdef SMTK_PYTHON_ENABLED
#include "smtk/operation/operators/ImportPythonOperation.h"
#endif
#include "smtk/operation/operators/ImportResource.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/RemoveResource.h"
#include "smtk/operation/operators/SetProperty.h"
#include "smtk/operation/operators/WriteResource.h"

#include "smtk/plugin/Manager.h"

#include <tuple>

namespace smtk
{
namespace operation
{
namespace
{
typedef std::tuple<
#ifdef SMTK_PYTHON_ENABLED
  ImportPythonOperation,
#endif
  ImportResource, ReadResource, RemoveResource, SetProperty, WriteResource>
  OperationList;
}

void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  managers->insert(smtk::operation::Manager::create());

  if (managers->contains<smtk::resource::Manager>())
  {
    managers->get<smtk::operation::Manager::Ptr>()->registerResourceManager(
      managers->get<smtk::resource::Manager::Ptr>());
  }
  smtk::plugin::Manager::instance()->registerPluginsTo(
    managers->get<smtk::operation::Manager::Ptr>());
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  managers->erase<smtk::operation::Manager::Ptr>();
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
}
}
}
