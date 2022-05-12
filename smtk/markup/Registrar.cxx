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
#include "smtk/markup/Registrar.h"

#include "smtk/markup/operators/Create.h"
#include "smtk/markup/operators/CreateAnalyticShape.h"
#include "smtk/markup/operators/CreateGroup.h"
#include "smtk/markup/operators/Delete.h"
#include "smtk/markup/operators/Import.h"
#include "smtk/markup/operators/Read.h"
#include "smtk/markup/operators/SetName.h"
#include "smtk/markup/operators/Ungroup.h"
#include "smtk/markup/operators/Write.h"

#include "smtk/markup/Resource.h"

#include "smtk/markup/SubphraseGenerator.h"

#include "smtk/view/NameManager.h"

#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/operation/groups/DeleterGroup.h"
#include "smtk/operation/groups/GroupingGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/NamingGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/UngroupingGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace markup
{

namespace
{
using OperationList = std::
  tuple<Create, CreateAnalyticShape, CreateGroup, Delete, Import, Read, SetName, Ungroup, Write>;

using SubphraseGeneratorList = std::tuple<smtk::markup::SubphraseGenerator>;
} // anonymous namespace

void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  // The markup resource uses a NameManager to ensure names are easy to identify
  // without visualizing the entire hierarchy above a component. If one isn't
  // already provided by the application, create one here.
  if (!managers->contains<smtk::view::NameManager::Ptr>())
  {
    managers->insert(smtk::view::NameManager::create());
  }
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  (void)managers;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  // Providing a write method to the resource manager is what allows
  // modelbuilder's pqSMTKSaveResourceBehavior to determine how to write
  // the resource when users click "Save Resource" or ⌘S/⌃S.
  resourceManager->registerResource<smtk::markup::Resource>(read, write, create);

  // Beyond registering the resource type, we now prepare the resource by
  // registering domain and node types to their respective factories.
  smtk::markup::Resource::domainFactory().registerTypes<smtk::markup::Traits::DomainTypes>();
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  // Add operations to groups
  smtk::operation::CreatorGroup(operationManager)
    .registerOperation<smtk::markup::Resource, smtk::markup::Create>();

  smtk::operation::DeleterGroup(operationManager).registerOperation<smtk::markup::Delete>();

  smtk::operation::GroupingGroup(operationManager).registerOperation<smtk::markup::CreateGroup>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::markup::Resource, smtk::markup::Import>();

  smtk::operation::NamingGroup(operationManager)
    .registerOperation<smtk::markup::Resource, smtk::markup::SetName>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::markup::Resource, smtk::markup::Read>();

  smtk::operation::UngroupingGroup(operationManager).registerOperation<smtk::markup::Ungroup>();

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::markup::Resource, smtk::markup::Write>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::markup::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::operation::CreatorGroup(operationManager).unregisterOperation<smtk::markup::Create>();

  smtk::operation::DeleterGroup(operationManager).unregisterOperation<smtk::markup::Delete>();

  smtk::operation::GroupingGroup(operationManager).unregisterOperation<smtk::markup::CreateGroup>();

  smtk::operation::UngroupingGroup(operationManager).unregisterOperation<smtk::markup::Ungroup>();

  smtk::operation::ImporterGroup(operationManager).unregisterOperation<smtk::markup::Import>();

  smtk::operation::NamingGroup(operationManager).unregisterOperation<smtk::markup::SetName>();

  smtk::operation::ReaderGroup(operationManager).unregisterOperation<smtk::markup::Read>();

  smtk::operation::WriterGroup(operationManager).unregisterOperation<smtk::markup::Write>();

  operationManager->unregisterOperations<OperationList>();
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->subphraseGeneratorFactory().registerTypes<SubphraseGeneratorList>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->subphraseGeneratorFactory().unregisterTypes<SubphraseGeneratorList>();
}

} // namespace markup
} // namespace smtk
