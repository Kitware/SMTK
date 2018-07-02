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
#include "smtk/bridge/vtk/Registrar.h"

#include "smtk/bridge/vtk/operators/Export.h"
#include "smtk/bridge/vtk/operators/Import.h"
#include "smtk/bridge/vtk/operators/LegacyRead.h"
#include "smtk/bridge/vtk/operators/Read.h"
#include "smtk/bridge/vtk/operators/Write.h"

#include "smtk/bridge/vtk/Resource.h"

#include "smtk/operation/groups/ExporterGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace bridge
{
namespace vtk
{

namespace
{
typedef std::tuple<Export, Import, LegacyRead, Read, Write> OperationList;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::bridge::vtk::Resource>(read, write);
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::bridge::vtk::Resource, smtk::bridge::vtk::Read>();
  smtk::operation::ReaderGroup(operationManager)
    .registerOperation(smtk::common::typeName<smtk::bridge::vtk::LegacyRead>(), "exodus");

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::bridge::vtk::Resource, smtk::bridge::vtk::Write>();

  smtk::operation::ExporterGroup(operationManager)
    .registerOperation<smtk::bridge::vtk::Resource, smtk::bridge::vtk::Export>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::bridge::vtk::Resource, smtk::bridge::vtk::Import>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::bridge::vtk::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
}
}
}
}
