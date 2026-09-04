//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//=========================================================================
#ifndef smtk_qt_job_ProgressMonitor_h
#define smtk_qt_job_ProgressMonitor_h

#include "smtk/common/UUID.h"

#include <QTimer>

#include <filesystem>
#include <functional>
#include <mutex>
#include <unordered_map>

namespace smtk
{
namespace job
{
class DatabaseQueue;
class Job;
} // namespace job
namespace qt
{
namespace job
{

/// Poll progress files for locally-running jobs and publish stage changes.
///
/// Job scripts communicate progress by atomically replacing or overwriting a
/// ``logs/progress`` file containing two integers: the completed stage and its
/// exit code. Polling is used instead of QFileSystemWatcher because directory
/// change notifications can be lost or denied on Windows and on shared filesystems.
/// Missing, unreadable, and partially-written files are ignored until the next poll.
///
/// The optional a poll callback lets a queue perform bookkeeping before calling
/// update(); without one, the timer calls update() directly. The timer and callback
/// execute in a context's thread. Calls to start() and stop() may come from other
/// threads; access to the collection of monitored paths is synchronized.
class ProgressMonitor
{
public:
  ProgressMonitor(
    smtk::job::DatabaseQueue* queue,
    QObject* context,
    int interval = 100,
    std::function<void()> poll = {});

  /// Begin polling a job's ``logs/progress`` file. Repeated calls are harmless.
  void start(const smtk::job::Job* job);
  /// Stop polling a job's ``logs/progress`` file.
  void stop(const smtk::job::Job* job);
  /// Read each registered progress file and submit any resulting JobUpdated operation.
  void update();

private:
  smtk::job::DatabaseQueue* m_queue{ nullptr };
  QTimer m_timer;
  /// Protect m_paths because operations may register jobs off the Qt thread.
  std::mutex m_mutex;
  std::unordered_map<std::filesystem::path, smtk::common::UUID> m_paths;
};

} // namespace job
} // namespace qt
} // namespace smtk

#endif
