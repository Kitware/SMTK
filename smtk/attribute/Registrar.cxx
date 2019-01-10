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
#include "smtk/attribute/operators/Read.h"
#include "smtk/attribute/operators/Write.h"

#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace attribute
{
namespace
{
typedef std::tuple<Associate, Dissociate, Read, Write> OperationList;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::attribute::Resource, smtk::attribute::Read>();

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::attribute::Resource, smtk::attribute::Write>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
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
