//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/job/ProgressMonitor.h"

#include "smtk/job/DatabaseQueue.h"
#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/operators/JobUpdated.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/operation/Manager.h"

#include <fstream>

namespace smtk
{
namespace qt
{
namespace job
{

ProgressMonitor::ProgressMonitor(
  smtk::job::DatabaseQueue* queue,
  QObject* context,
  int interval,
  std::function<void()> poll)
  : m_queue(queue)
{
  m_timer.setInterval(interval);
  m_timer.setSingleShot(false);
  QObject::connect(
    &m_timer, &QTimer::timeout, context, poll ? std::move(poll) : std::function<void()>([this]() {
      this->update();
    }));
  m_timer.start();
}

void ProgressMonitor::start(const smtk::job::Job* job)
{
  if (!job)
  {
    return;
  }
  std::lock_guard<std::mutex> lock(m_mutex);
  m_paths[job->caseDirectory() / "logs" / "progress"] = job->id();
}

void ProgressMonitor::stop(const smtk::job::Job* job)
{
  if (!job)
  {
    return;
  }
  std::lock_guard<std::mutex> lock(m_mutex);
  m_paths.erase(job->caseDirectory() / "logs" / "progress");
}

void ProgressMonitor::update()
{
  auto operationManager = m_queue ? m_queue->operationManager() : nullptr;
  if (!operationManager)
  {
    return;
  }

  decltype(m_paths) paths;
  {
    // Do not hold the mutex while reading files or launching operations. Among
    // other things, an observer of JobUpdated may start or stop another job.
    std::lock_guard<std::mutex> lock(m_mutex);
    paths = m_paths;
  }
  for (const auto& entry : paths)
  {
    std::ifstream progress(entry.first);
    int stage = -3;
    int exitCode = 0;
    progress >> stage >> exitCode;
    if (!progress.good() || stage <= -3)
    {
      // Writers truncate before writing new contents, so an incomplete read is
      // expected occasionally. The repeating timer will retry it.
      continue;
    }

    auto job = m_queue->findJob(entry.second);
    if (!job)
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_paths.erase(entry.first);
      continue;
    }
    if (
      job->stage() == stage ||
      (job->state() != smtk::job::State::Scheduled && job->state() != smtk::job::State::Running))
    {
      continue;
    }

    auto updater = operationManager->create<smtk::job::JobUpdated>();
    updater->parameters()->associate(job);
    updater->parameters()->findInt("stage")->setIsEnabled(true);
    updater->parameters()->findInt("stage")->setValue(stage);
    updater->parameters()->findInt("state")->setIsEnabled(true);
    updater->parameters()->findInt("state")->setValue(
      stage < 0 ? static_cast<int>(smtk::job::State::Scheduled)
        : stage < static_cast<int>(job->jobType()->stages().size())
        ? static_cast<int>(smtk::job::State::Running)
        : static_cast<int>(smtk::job::State::Completed));
    if (stage < 0)
    {
      updater->parameters()->findInt("status")->setIsEnabled(true);
      updater->parameters()->findInt("status")->setValue(
        static_cast<int>(smtk::job::Status::Pending));
    }
    else if (exitCode != 0)
    {
      updater->parameters()->findInt("state")->setValue(
        static_cast<int>(smtk::job::State::Completed));
      updater->parameters()->findInt("status")->setIsEnabled(true);
      updater->parameters()->findInt("status")->setValue(
        static_cast<int>(smtk::job::Status::Failed));
    }
    else if (stage >= static_cast<int>(job->jobType()->stages().size()))
    {
      updater->parameters()->findInt("status")->setIsEnabled(true);
      updater->parameters()->findInt("status")->setValue(
        static_cast<int>(smtk::job::Status::Succeeded));
      std::lock_guard<std::mutex> lock(m_mutex);
      // A terminal stage needs no further progress-file polling. ShellQueue
      // independently retains its QProcess until the process actually exits.
      m_paths.erase(entry.first);
    }
    operationManager->launchers()(updater);
  }
}

} // namespace job
} // namespace qt
} // namespace smtk
