//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_qt_job_ContainerQueue_h
#define smtk_qt_job_ContainerQueue_h

#include "smtk/common/Managers.h"
#include "smtk/extension/qt/Exports.h" // For export macro.
#include "smtk/job/DatabaseQueue.h"

#include <QObject>

#include <memory>

namespace smtk
{
namespace qt
{
namespace job
{

///\brief ContainerQueue schedules jobs locally by immediately running them.
///
/// This class depends on Qt for process and filesystem monitoring.
class SMTKQTEXT_EXPORT ContainerQueue
  : public QObject
  , public smtk::job::DatabaseQueue
{
public:
  smtkTypeMacro(smtk::qt::job::ContainerQueue);
  smtkSuperclassMacro(smtk::job::DatabaseQueue);
  smtkCreateMacro(smtk::job::Queue);
  smtkSharedFromThisMacro(smtk::job::Queue);

  template<typename QueueType>
  static std::shared_ptr<QueueType> createOrRestore(
    const std::string& name,
    const std::string& description,
    const std::string& location,
    int maxJobSize = 0,
    const std::unordered_set<smtk::string::Token>& tags = {},
    const std::filesystem::path& containerEngineExecutable = "podman",
    bool removeQueueOnDestruction = false,
    const smtk::common::UUID& uid = smtk::common::UUID::null(),
    const std::shared_ptr<smtk::resource::Manager>& resourceManager =
      std::shared_ptr<smtk::resource::Manager>(),
    const std::shared_ptr<smtk::operation::Manager>& operationManager =
      std::shared_ptr<smtk::operation::Manager>(),
    const std::shared_ptr<smtk::job::Manager>& jobManager = std::shared_ptr<smtk::job::Manager>())
  {
    std::shared_ptr<QueueType> queue = Superclass::createOrRestore<QueueType>(
      name,
      description,
      location,
      maxJobSize,
      tags,
      removeQueueOnDestruction,
      uid,
      resourceManager,
      operationManager,
      jobManager);
    if (queue)
    {
      queue->setEngineExecutable(containerEngineExecutable);
    }
    return queue;
  }

  ContainerQueue();
  ContainerQueue(
    const smtk::common::UUID& uid,
    const std::shared_ptr<smtk::resource::Manager>& resourceManager);
  ~ContainerQueue() override;

  std::string location() const override { return "localhost"; }

  /// This type of queue **may** allow some jobs to be canceled.
  bool allowsCancellation() const override { return true; }

  /// Run the given job as a separate process.
  bool schedule(const std::shared_ptr<smtk::job::Job>& job) override;
  /// Cancel a scheduled job (whether it is running or not).
  bool cancel(const std::shared_ptr<smtk::job::Job>& job) override;
  /// Return the state of a job.
  smtk::job::State jobState(const std::shared_ptr<smtk::job::Job>& job) override;
  /// Return the completion-status of a job.
  smtk::job::Status jobStatus(const std::shared_ptr<smtk::job::Job>& job) override;

  /// Return the set of all jobs in this queue.
  std::set<std::shared_ptr<smtk::job::Job>> allJobs() const override;

  /// Pull the named container image.
  bool pullContainerImage(const std::string& imageUrl);

protected Q_SLOTS:

  ///@{
  /// Get the container engine to use.
  ///
  /// Valid values include: "podman" and "docker".
  /// All path information to the executable is stripped from the returned value.
  /// In the future, singularity/apptainer may also be supported.
  smtk::string::Token engine() const;
  ///@}

  ///@{
  /// Set/get the path to the container engine executable.
  ///
  /// The default is "podman" (assumed to be in your path).
  /// The supported engines include podman and docker.
  /// In the future, singularity/apptainer may also be supported.
  std::filesystem::path engineExecutable() const;
  bool setEngineExecutable(std::filesystem::path engineExecutable);
  ///@}

  ///@{
  /// Set/get the user or grooup ID to use when running via docker.
  ///
  /// Note that podman ignores these settings in favor of "--userns=keep-id".
  int dockerUID() const;
  bool setDockerUID(int uid) const;
  int dockerGID() const;
  bool setDockerGID(int gid) const;
  ///@}

  ///@{
  /// Set/get the directory *inside the container* where the case directory
  /// should be mounted.
  ///
  /// This is used to compute the full path to the job script when running the container.
  ///
  /// This must be non-empty (and valid) before jobs are run.
  std::filesystem::path caseDirectoryMountPoint() const;
  bool setCaseDirectoryMountPoint(const std::filesystem::path& mountPoint);
  ///@}

  ///@{
  /// Called when the internal filesystem-watcher notices a watched log
  /// directory or progress file has been modified.
  void fileUpdated(const QString& path);
  void directoryUpdated(const QString& path);
  ///@}

private:
  class Internal;
  std::unique_ptr<Internal> m_p;
};

} // namespace job
} // namespace qt
} // namespace smtk

#endif // smtk_qt_job_ContainerQueue_h
