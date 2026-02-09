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
#include "smtk/job/Registrar.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/job/Job.h"
#include "smtk/job/Resource.h"
#include "smtk/plugin/Manager.h"

namespace smtk
{
namespace job
{
namespace
{

// An observer attached to the operation manager:
smtk::operation::Observers::Key g_operationObserver;

void possiblyLaunchJob(
  const smtk::operation::Operation& op,
  smtk::operation::Operation::Result result)
{
  // Ignore jobs attached to failed operations.
  if (smtk::operation::outcome(result) != smtk::operation::Operation::Outcome::SUCCEEDED)
  {
    return;
  }
  auto jobResource = smtk::job::Resource::instance();
  auto jobsItem = result->findComponent("jobsToSubmit");
  if (jobsItem && jobsItem->numberOfValues() > 0)
  {
    for (const auto& value : *jobsItem)
    {
      if (auto job = std::dynamic_pointer_cast<smtk::job::Job>(value))
      {
        jobResource->addJob(job);
        std::cerr << "Launch job " << job << "\n";
      }
    }
  }
}

} // anonymous namespace

void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  // Add a job::Manager if none is present:
  if (!managers->contains<smtk::job::Manager::Ptr>())
  {
    if (managers->insert(smtk::job::Manager::create()))
    {
      // if (managers->contains<smtk::resource::Manager::Ptr>())
      // {
      //   managers->get<smtk::job::Manager::Ptr>()->registerResourceManager(
      //     managers->get<smtk::resource::Manager::Ptr>());
      // }
      // smtk::plugin::Manager::instance()->registerPluginsTo(
      //   managers->get<smtk::job::Manager::Ptr>());
    }
  }
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  (void)managers;
  managers->erase<smtk::job::Manager::Ptr>();
}

void Registrar::registerTo(const smtk::job::Manager::Ptr& jobManager) {}

void Registrar::unregisterFrom(const smtk::job::Manager::Ptr& jobManager) {}
#if 0
#endif

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager) {}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager) {}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  g_operationObserver = operationManager->observers().insert(
    [&](
      const smtk::operation::Operation& op,
      smtk::operation::EventType event,
      smtk::operation::Operation::Result result) -> int {
      if (event == smtk::operation::EventType::DID_OPERATE)
      {
        possiblyLaunchJob(op, result);
      }
      return 0; // Never cancel an operation.
    },
    /*priority*/ 0,
    /*initialize*/ false,
    "Observe operations for jobs to queue.");
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  (void)operationManager;
  g_operationObserver.release();
}
} // namespace job
} // namespace smtk
