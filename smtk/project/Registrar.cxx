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

#include "smtk/project/PhraseModel.h"
#include "smtk/project/Project.h"
#include "smtk/project/icons/project_svg.h"
#include "smtk/project/operators/AddToProject.h"
#include "smtk/project/operators/CreateProject.h"
#include "smtk/project/operators/DefineProject.h"

#include "smtk/view/SVGIconConstructor.h"

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

typedef std::tuple<AddToProject, CreateProject, DefineProject> OperationList;
} // namespace

void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  if (
    managers->contains<smtk::operation::Manager::Ptr>() &&
    managers->contains<smtk::resource::Manager::Ptr>())
  {
    auto operationManager = managers->get<smtk::operation::Manager::Ptr>();
    auto resourceManager = managers->get<smtk::resource::Manager::Ptr>();
    this->OldProjectManager =
      smtk::project::old::Manager::create(resourceManager, operationManager);
  }
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  managers->erase<smtk::project::Manager::Ptr>();
}

void Registrar::registerTo(const smtk::project::Manager::Ptr& projectManager)
{
  projectManager->registerOperations<OperationList>();
}

void Registrar::unregisterFrom(const smtk::project::Manager::Ptr& projectManager)
{
  projectManager->unregisterOperations<OperationList>();
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->phraseModelFactory().registerType<PhraseModel>();

  viewManager->objectIcons().registerIconConstructor<smtk::project::Project>(
    ProjectIconConstructor());
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->phraseModelFactory().unregisterType<PhraseModel>();

  viewManager->objectIcons().unregisterIconConstructor<smtk::project::Project>();
}
} // namespace project
} // namespace smtk
