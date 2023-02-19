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
#include "smtk/project/Registrar.h"

#include "smtk/attribute/Resource.h"

#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

#include "smtk/plugin/Manager.h"

#include "smtk/project/Project.h"
#include "smtk/project/operators/Add.h"
#include "smtk/project/operators/Create.h"
#include "smtk/project/operators/Define.h"
#include "smtk/project/operators/Print.h"
#include "smtk/project/operators/Read.h"
#include "smtk/project/operators/Remove.h"
#include "smtk/project/operators/Write.h"

#include "smtk/project/view/IconConstructor.h"
#include "smtk/project/view/PhraseModel.h"
#include "smtk/project/view/SubphraseGenerator.h"

#include <tuple>

namespace smtk
{
namespace project
{
namespace
{
class ProjectIconConstructor : public smtk::view::SVGIconConstructor
{
  std::string svg(const smtk::resource::PersistentObject&) const override { return project_svg; }
};

typedef std::tuple<Add, Create, Define, Print, Read, Remove, Write> OperationList;
} // namespace

void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  if (
    managers->contains<smtk::operation::Manager::Ptr>() &&
    managers->contains<smtk::resource::Manager::Ptr>())
  {
    auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
    auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
    managers->insert(smtk::project::Manager::create(resourceManager, operationManager));

    smtk::plugin::Manager::instance()->registerPluginsTo(
      managers->get<smtk::project::Manager::Ptr>());
  }
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  managers->erase<smtk::project::Manager::Ptr>();
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  (void)operationManager;
  /// Defer the registration of project operations until there is a project
  /// manager available.
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  (void)operationManager;
}

void Registrar::registerTo(const smtk::project::Manager::Ptr& projectManager)
{
  projectManager->registerOperations<OperationList>();

  smtk::operation::CreatorGroup(projectManager->operationManager())
    .registerOperation<smtk::project::Project, smtk::project::Create>();

  smtk::operation::ReaderGroup(projectManager->operationManager())
    .registerOperation<smtk::project::Project, smtk::project::Read>();

  smtk::operation::WriterGroup(projectManager->operationManager())
    .registerOperation<smtk::project::Project, smtk::project::Write>();
}

void Registrar::unregisterFrom(const smtk::project::Manager::Ptr& projectManager)
{
  projectManager->unregisterOperations<OperationList>();

  smtk::operation::CreatorGroup(projectManager->operationManager())
    .unregisterOperation<smtk::project::Create>();

  smtk::operation::ReaderGroup(projectManager->operationManager())
    .unregisterOperation<smtk::project::Read>();

  smtk::operation::WriterGroup(projectManager->operationManager())
    .unregisterOperation<smtk::project::Write>();
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<Project>(read, write);
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<Project>();
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->phraseModelFactory().registerType<view::PhraseModel>();
  viewManager->subphraseGeneratorFactory().registerType<view::SubphraseGenerator>();

  viewManager->objectIcons().registerIconConstructor<smtk::project::Project>(
    ProjectIconConstructor());
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->phraseModelFactory().unregisterType<view::PhraseModel>();
  viewManager->subphraseGeneratorFactory().unregisterType<view::SubphraseGenerator>();

  viewManager->objectIcons().unregisterIconConstructor<smtk::project::Project>();
}
} // namespace project
} // namespace smtk
