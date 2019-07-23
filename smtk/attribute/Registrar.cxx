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
#include "smtk/attribute/Registrar.h"

#include "smtk/attribute/Resource.h"

#include "smtk/attribute/operators/Associate.h"
#include "smtk/attribute/operators/Dissociate.h"
#include "smtk/attribute/operators/Export.h"
#include "smtk/attribute/operators/Import.h"
#include "smtk/attribute/operators/Read.h"
#include "smtk/attribute/operators/Signal.h"
#include "smtk/attribute/operators/Write.h"

#include "smtk/operation/groups/ExporterGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/InternalGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace attribute
{
namespace
{
typedef std::tuple<Associate, Dissociate, Export, Import, Read, Signal, Write> OperationList;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::attribute::Resource, smtk::attribute::Read>();

  smtk::operation::ExporterGroup(operationManager)
    .registerOperation<smtk::attribute::Resource, smtk::attribute::Export>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::attribute::Resource, smtk::attribute::Import>();

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::attribute::Resource, smtk::attribute::Write>();

  smtk::operation::InternalGroup(operationManager).registerOperation<smtk::attribute::Signal>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::operation::ReaderGroup(operationManager).unregisterOperation<smtk::attribute::Read>();

  smtk::operation::ExporterGroup(operationManager).unregisterOperation<smtk::attribute::Export>();

  smtk::operation::ImporterGroup(operationManager).unregisterOperation<smtk::attribute::Import>();

  smtk::operation::WriterGroup(operationManager).unregisterOperation<smtk::attribute::Write>();

  smtk::operation::InternalGroup(operationManager).unregisterOperation<smtk::attribute::Signal>();

  operationManager->unregisterOperations<OperationList>();
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::attribute::Resource>(read, write);
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::attribute::Resource>();
}
}
}
