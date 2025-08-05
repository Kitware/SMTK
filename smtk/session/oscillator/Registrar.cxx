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
#include "smtk/session/oscillator/Registrar.h"

#include "smtk/session/oscillator/operators/CreateModel.h"
#include "smtk/session/oscillator/operators/EditDomain.h"
#include "smtk/session/oscillator/operators/EditSource.h"
#include "smtk/session/oscillator/operators/Export.h"
#include "smtk/session/oscillator/operators/Read.h"
#include "smtk/session/oscillator/operators/Write.h"

#include "smtk/session/oscillator/Resource.h"

#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/operation/groups/ExporterGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace session
{
namespace oscillator
{

namespace
{
typedef std::tuple<CreateModel, EditDomain, EditSource, Export, Read, Write> OperationList;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  smtk::string::Token dummy("smtk::session::oscillator::Resource");
  // Providing a write method to the resource manager is what allows
  // modelbuilder's pqSMTKSaveResourceBehavior to determine how to write
  // the resource when users click "Save Resource" or ⌘ S/⌃ S.
  resourceManager->registerResource<smtk::session::oscillator::Resource>(read, write);
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  smtk::operation::CreatorGroup(operationManager)
    .registerOperation<
      smtk::session::oscillator::Resource,
      smtk::session::oscillator::EditDomain>();

  // smtk::operation::ImporterGroup(operationManager)
  //   .registerOperation<smtk::session::oscillator::Resource, smtk::session::oscillator::ReadRXFFile>();

  // TODO: Export really expects a simulation attribute, not a geometric model.
  //       This is because the simulation attribute is linked to the geometric model
  //       and not vice-versa (so there is only a slow path for obtaining the
  //       simulation attribute from the geometry).
  // smtk::operation::ExporterGroup(operationManager)
  //   .registerOperation<smtk::session::oscillator::Resource, smtk::session::oscillator::Export>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::session::oscillator::Resource, smtk::session::oscillator::Read>();

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::session::oscillator::Resource, smtk::session::oscillator::Write>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::session::oscillator::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::operation::CreatorGroup(operationManager)
    .unregisterOperation<smtk::session::oscillator::EditDomain>();

  smtk::operation::ReaderGroup(operationManager)
    .unregisterOperation<smtk::session::oscillator::Read>();

  smtk::operation::WriterGroup(operationManager)
    .unregisterOperation<smtk::session::oscillator::Write>();

  operationManager->unregisterOperations<OperationList>();
}
} // namespace oscillator
} // namespace session
} // namespace smtk
