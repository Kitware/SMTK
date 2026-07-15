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

#include "smtk/attribute/Attribute.h"
#include "smtk/common/Managers.h"
#include "smtk/extension/qt/Exports.h"            // For export macro.
#include "smtk/extension/qt/qtTypeDeclarations.h" // So property links work.
#include "smtk/job/DatabaseQueue.h"
#include "smtk/operation/Manager.h"

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
  Q_OBJECT
  Q_PROPERTY(
    QString rootJobDirectory READ rootJobDirectoryAsString WRITE setRootJobDirectoryAsString);

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
    const std::filesystem::path& rootJobDirectory = std::filesystem::path(),
    int dockerUserId = -1,
    int dockerGroupId = -1,
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
      if (dockerUserId >= 0)
      {
        queue->setDockerUID(dockerUserId);
      }
      if (dockerGroupId >= 0)
      {
        queue->setDockerGID(dockerGroupId);
      }
      // If the root job directory causes a change to the queue, this will
      // start a podman machine. If there is no change, the queue must already
      // have existed before since the database will only have non-null metadata
      // if it has been created *and* started before.
      if (!queue->checkQueueRoot(rootJobDirectory))
      {
        queue->setRootJobDirectory(rootJobDirectory);
      }
    }
    return queue;
  }

  ContainerQueue();
  ContainerQueue(
    const smtk::common::UUID& uid,
    const std::shared_ptr<smtk::resource::Manager>& resourceManager);
  ~ContainerQueue() override;

  /// Get the path to the container engine executable.
  ///
  /// The default is "podman" (assumed to be in your path).
  /// The supported engines include podman and docker.
  /// In the future, singularity/apptainer may also be supported.
  std::filesystem::path engineExecutable() const;

  /// Get whether the queue is online or not.
  ///
  /// The queue is online when the virtual machine used to run containers
  /// has been started. It is offline when the virtual machine does not
  /// exist or has been stopped.
  bool queueOnline() const { return m_queueOnline; }

  /// Return the location whose resources are used to run jobs.
  std::string location() const override { return "localhost"; }

  /// This type of queue **may** allow some jobs to be canceled.
  bool allowsCancellation() const override { return true; }

  /// Run the given job as a separate process.
  /// This should only be called from within the smtk::job::ScheduleJob operation.
  bool schedule(const std::shared_ptr<smtk::job::Job>& job) override;

  /// Cancel a scheduled job (whether it is running or not).
  /// This should only be called from within the smtk::job::CancelJob operation.
  bool cancel(const std::shared_ptr<smtk::job::Job>& job) override;

  /// Return the state of a job.
  smtk::job::State jobState(const std::shared_ptr<smtk::job::Job>& job) override;
  /// Return the completion-status of a job.
  smtk::job::Status jobStatus(const std::shared_ptr<smtk::job::Job>& job) override;

  /// Return the set of all jobs in this queue.
  std::set<std::shared_ptr<smtk::job::Job>> allJobs() const override;

  /// Pull the named container image.
  bool pullContainerImage(const std::string& imageUrl);

protected:
  friend class UpdateContainerQueueMachine;
  using DatabaseQueue::setMaximumJobSize;

public Q_SLOTS:
  /// When users change the ProjectsRootFolder setting (in the Edit→Settings dialog),
  /// this slot is called to cycle the virtual machine so that containers can mount
  /// case files in new projects. Note this will halt existing jobs and (if they are
  /// not in a subdirectory of the new ProjectsRootFolder) make those jobs inaccessible.
  void projectRootChanged(const std::filesystem::path& nextProjectRoot);

  ///@{
  /// Get the container engine to use.
  ///
  /// Valid values include: "podman" and "docker".
  /// All path information to the executable is stripped from the returned value.
  /// In the future, singularity/apptainer may also be supported.
  smtk::string::Token engine() const;
  ///@}

Q_SIGNALS:
  /// This signal is emitted when the case directory mount point is being changed.
  void rootJobDirectoryChanging(
    const std::filesystem::path& prev,
    const std::filesystem::path& next);

protected Q_SLOTS:

  ///@{
  /// Set/get the path to the container engine executable.
  ///
  /// The default is "podman" (assumed to be in your path).
  /// The supported engines include podman and docker.
  /// In the future, singularity/apptainer may also be supported.
  bool setEngineExecutable(std::filesystem::path engineExecutable);
  ///@}

  ///@{
  /// Set/get the user or grooup ID to use when running via docker.
  ///
  /// Note that podman ignores these settings in favor of "--userns=keep-id".
  int dockerUID() const;
  bool setDockerUID(int uid);
  int dockerGID() const;
  bool setDockerGID(int gid);
  ///@}

  ///@{
  /// Set/get the directory *inside the container* where the case directory
  /// should be mounted.
  ///
  /// This is used to compute the full path to the job script when running the container.
  ///
  /// This must be non-empty (and valid) before jobs are run.
  std::filesystem::path rootJobDirectory() const { return m_rootJobDirectory; }
  bool setRootJobDirectory(const std::filesystem::path& mountPoint);
  QString rootJobDirectoryAsString();
  bool setRootJobDirectoryAsString(const QString& mountPoint);
  ///@}

  ///@{
  /// Called when the internal filesystem-watcher notices a watched log
  /// directory or progress file has been modified.
  // void fileUpdated(const QString& path);
  // void directoryUpdated(const QString& path);
  ///@}

  ///@{
  /// Set whether the queue is online or not.
  ///
  /// The queue is online when the virtual machine used to run containers
  /// has been started. It is offline when the virtual machine does not
  /// exist or has been stopped.
  bool setQueueOnline(bool online);
  ///@}

  /// Iterate m_p->m_pathsToPoll to see if any job states have changed.
  void updateJobStates();

protected:
  /// Returns true if the queue's (podman) machine has the given \a root.
  ///
  /// If not, then the machine must be recreated from scratch and restarted.
  bool checkQueueRoot(const std::filesystem::path& root);

  /// Set persistent metadata for the queue.
  bool setMetadata(const std::string& key, const std::string& value);

private:
  class Internal;
  std::unique_ptr<Internal> m_p;
  std::filesystem::path m_rootJobDirectory;
  bool m_queueOnline{ false };
};

} // namespace job
} // namespace qt
} // namespace smtk

#endif // smtk_qt_job_ContainerQueue_h
