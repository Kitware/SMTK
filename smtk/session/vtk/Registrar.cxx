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
#include "smtk/session/vtk/Registrar.h"

#include "smtk/session/vtk/operators/Export.h"
#include "smtk/session/vtk/operators/Import.h"
#include "smtk/session/vtk/operators/LegacyRead.h"
#include "smtk/session/vtk/operators/Read.h"
#include "smtk/session/vtk/operators/Write.h"

#include "smtk/session/vtk/Resource.h"

#include "smtk/operation/groups/ExporterGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace session
{
namespace vtk
{

namespace
{
typedef std::tuple<Export, Import, LegacyRead, Read, Write> OperationList;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::session::vtk::Resource>(read, write);
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::session::vtk::Resource, smtk::session::vtk::Read>();
  smtk::operation::ReaderGroup(operationManager)
    .registerOperation(smtk::common::typeName<smtk::session::vtk::LegacyRead>(), "exodus");

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::session::vtk::Resource, smtk::session::vtk::Write>();

  smtk::operation::ExporterGroup(operationManager)
    .registerOperation<smtk::session::vtk::Resource, smtk::session::vtk::Export>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::session::vtk::Resource, smtk::session::vtk::Import>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::session::vtk::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
}
}
}
}
