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
#include "smtk/session/rgg/Registrar.h"

#include "smtk/session/rgg/operators/AddMaterial.h"
#include "smtk/session/rgg/operators/CreateAssembly.h"
#include "smtk/session/rgg/operators/CreateDuct.h"
#include "smtk/session/rgg/operators/CreateModel.h"
#include "smtk/session/rgg/operators/CreatePin.h"
#include "smtk/session/rgg/operators/Delete.h"
#include "smtk/session/rgg/operators/EditAssembly.h"
#include "smtk/session/rgg/operators/EditCore.h"
#include "smtk/session/rgg/operators/EditDuct.h"
#include "smtk/session/rgg/operators/EditMaterial.h"
#include "smtk/session/rgg/operators/EditPin.h"
#include "smtk/session/rgg/operators/ReadRXFFile.h"
#include "smtk/session/rgg/operators/RemoveMaterial.h"

#include "smtk/session/rgg/Resource.h"

#include "smtk/operation/RegisterPythonOperations.h"

#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/operation/groups/ExporterGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace session
{
namespace rgg
{

namespace
{
typedef std::tuple<AddMaterial, CreateAssembly, CreateDuct, CreateModel, CreatePin, Delete,
  EditAssembly, EditCore, EditDuct, EditMaterial, EditPin, ReadRXFFile, RemoveMaterial>
  OperationList;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  // resourceManager->registerResource<smtk::session::rgg::Resource>(read, write);
  resourceManager->registerResource<smtk::session::rgg::Resource>();
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

#ifdef ENABLE_PYARC_BINDINGS
  smtk::operation::registerPythonOperations(operationManager, "smtk.session.rgg.export_to_pyarc");

  smtk::operation::ExporterGroup(operationManager)
    .registerOperation<smtk::session::rgg::Resource>(
      "smtk.session.rgg.export_to_pyarc.export_to_pyarc");
#endif

  smtk::operation::CreatorGroup(operationManager)
    .registerOperation<smtk::session::rgg::Resource, smtk::session::rgg::CreateModel>();

  // smtk::operation::ImporterGroup(operationManager)
  //   .registerOperation<smtk::session::rgg::Resource, smtk::session::rgg::ReadRXFFile>();

  // smtk::operation::ReaderGroup(operationManager)
  //   .registerOperation<smtk::session::rgg::Resource, smtk::session::rgg::Read>();

  // smtk::operation::WriterGroup(operationManager)
  //   .registerOperation<smtk::session::rgg::Resource, smtk::session::rgg::Write>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::session::rgg::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{

#ifdef ENABLE_PYARC_BINDINGS
  smtk::operation::ExporterGroup(operationManager)
    .unregisterOperation("smtk.session.rgg.export_to_pyarc.export_to_pyarc");
#endif

  smtk::operation::CreatorGroup(operationManager)
    .unregisterOperation<smtk::session::rgg::CreateModel>();

  // smtk::operation::ImporterGroup(operationManager)
  //   .unregisterOperation<smtk::session::rgg::ReadRXFFile>();

  // smtk::operation::ReaderGroup(operationManager)
  //   .unregisterOperation<smtk::session::rgg::Read>();

  // smtk::operation::WriterGroup(operationManager)
  //   .unregisterOperation<smtk::session::rgg::Write>();

  operationManager->unregisterOperations<OperationList>();
}
}
}
}
