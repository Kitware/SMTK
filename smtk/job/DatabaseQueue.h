//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_DatabaseQueue_h
#define smtk_job_DatabaseQueue_h

#include "smtk/job/Queue.h"

struct sqlite3;

namespace smtk
{
namespace job
{
class AddJobToQueue;
class Definition;
class JobUpdated;

/// A "database" queue is a queue with an implementation that stores its component
/// jobs in a (persistent) database.
class SMTKCORE_EXPORT DatabaseQueue : public smtk::resource::DerivedFrom<DatabaseQueue, Queue>
{
public:
  smtkTypeMacro(smtk::job::DatabaseQueue);
  smtkCreateMacro(smtk::job::DatabaseQueue);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  template<typename QueueType>
  static std::shared_ptr<QueueType> createOrRestore(
    const std::string& name,
    const std::string& description,
    const std::string& location,
    int maxJobSize = 0,
    const std::unordered_set<smtk::string::Token>& tags = {},
    bool removeQueueOnDestruction = false,
    const smtk::common::UUID& uid = smtk::common::UUID::null(),
    const std::shared_ptr<smtk::resource::Manager>& resourceManager =
      std::shared_ptr<smtk::resource::Manager>(),
    const std::shared_ptr<smtk::operation::Manager>& operationManager =
      std::shared_ptr<smtk::operation::Manager>(),
    const std::shared_ptr<smtk::job::Manager>& jobManager = std::shared_ptr<smtk::job::Manager>())
  {
    std::shared_ptr<QueueType> queue =
      std::dynamic_pointer_cast<QueueType>(DatabaseQueue::findQueue(resourceManager, uid, name));
    if (!queue)
    {
      // No resource manager was given or no matching queue has been added to the
      // resource manager yet.
      // Search the database for the queue by UUID and then (if no UUID provided)
      // by name. This will not return a queue object but will return either (1) the
      // given UUID or, if it was null and the name matched an existing database
      // entry (2) the UUID from the database or, if no match exists (3) a new, non-null
      // randomly-generated UUID.
      auto actualId = DatabaseQueue::uuidFromDatabase(uid, name);
      if (!actualId.isNull())
      {
        // Construct the queue instance since there is no name+uid collision:
        queue = std::make_shared<QueueType>(actualId, resourceManager);
      }
    }
    if (queue)
    {
      // Update the queue's database entry if the location, description, etc.
      // are different by setting member variables and calling updateQueueData();
      queue->m_maximumSize = maxJobSize;
      queue->m_location = location;
      queue->m_name = name;
      queue->m_description = description;
      queue->m_tags = tags;
      queue->m_removeQueueOnDestruction = removeQueueOnDestruction;
      queue->m_jobManager = jobManager.get();
      queue->setOperationManager(operationManager);
      queue->updateQueueData();
    }
    return queue;
  }

  DatabaseQueue();
  DatabaseQueue(const smtk::common::UUID& uid);
  DatabaseQueue(const smtk::common::UUID& uid, resource::ManagerPtr manager);
  DatabaseQueue(resource::ManagerPtr manager);
  DatabaseQueue(DatabaseQueue&&) = default;
  ~DatabaseQueue() override;

  /// Mark the queue to be destroyed when this instance of DatabaseQueue is destroyed.
  ///
  /// This is normally false, but should be set to true for queues created for testing purposes.
  bool setRemoveQueueOnDestruction(bool shouldRemove);

  /// Provide a method to change the UUID of a queue (used for serialization/deserialization)
  bool setId(const common::UUID& uid) override;

  /// A user-presentable name for the queue.
  bool setName(const std::string& name);

  /// A user-presentable description of the queue.
  bool setDescription(const std::string& description);

  /// Set/get the location of the queue.
  std::string location() const override;

  smtk::resource::ComponentPtr find(const smtk::common::UUID& compId) const override;
  void visit(std::function<void(const smtk::resource::ComponentPtr&)>& v) const override;

  smtk::string::Token templateType() const override;
  std::size_t templateVersion() const override;

  /// Override add/removeTag to keep database up-to-date and prevent
  /// removal of programmatic tags.
  bool addTag(smtk::string::Token tag) override;
  bool removeTag(smtk::string::Token tag) override;

  /// Indicate this queue's maximum job size (concurrent processes/ranks).
  virtual std::uint64_t maximumJobSize() const;

  /// Retrieve a job component given its UUID
  std::shared_ptr<smtk::job::Job> findJob(const smtk::common::UUID& uid) const override;

  /// Add a job to the queue without scheduling it.
  bool add(const std::shared_ptr<Job>& job) override;

  /// Schedule a job to run on the queue's resources.
  bool schedule(const std::shared_ptr<Job>& job) override;

  /// Cancel a scheduled job (whether it is running or not).
  ///
  /// This scheduler will refuse to allow users to cancel jobs
  /// because the C++ standard library provides no simple means
  /// to kill a process (especially in a cross-platform manner).
  bool cancel(const std::shared_ptr<Job>& job) override;

  /// Return the state of a job.
  State jobState(const std::shared_ptr<Job>& job) override;

  /// Return the completion-status of a job.
  Status jobStatus(const std::shared_ptr<Job>& job) override;

  /// Return the set of all jobs in this queue.
  std::set<std::shared_ptr<Job>> allJobs() const override;

protected:
  /// Allow operations access to update database info.
  friend class AddJobToQueue;
  friend class JobUpdated;

  /// Return the queue with the given \a uid and \a name in the \a resourceManager.
  static std::shared_ptr<DatabaseQueue> findQueue(
    const std::shared_ptr<smtk::resource::Manager>& resourceManager,
    const smtk::common::UUID& uid,
    const std::string& name);

  /// Return the actual UUID of a queue in the database matching \a uid and \a name.
  ///
  /// It is acceptable for the input \a uid to be null (in which case the UUID of the
  /// queue with the matching name will be returned).
  ///
  /// If both \a uid and \a name are provided, both must match. If not, a null UUID
  /// will be returned, indicating the given \a uid is taken by a different queue.
  static smtk::common::UUID uuidFromDatabase(
    const smtk::common::UUID& uid,
    const std::string& name);

  /// Insert new data into the database for a new instance of a database.
  void createQueueData();

  /// Return true if the given UUID exists in the database as a queue.
  bool hasQueueData(const smtk::common::UUID& uid) const;

  /// Deserialize member data (for the queue, not job data) from a database.
  ///
  /// This returns true if entries exist in the database for the queue's UUID.
  bool fetchQueueData();

  /// Deserialize a job definition from the database.
  std::shared_ptr<smtk::job::Definition> fetchJobTypeData(const std::string& jobTypeName) const;
  std::shared_ptr<smtk::job::Definition> fetchJobTypeDataSql(const std::string& jobTypeName) const;

  /// Deserialize a job from the database and add it to the queue's "live" job map.
  std::shared_ptr<smtk::job::Job> fetchJobData(const smtk::common::UUID& uid) const;

  /// Deserialize job links from the database.
  bool fetchJobLinks(const std::shared_ptr<Job>& job);

  /// Update the database: change all references to the old UUID to the new UUID.
  bool updateQueueId(const smtk::common::UUID& prevId, const smtk::common::UUID& nextId);

  /// Update the database: reset all database entries to match ivars (name, description, etc.).
  bool updateQueueData();

  /// Remove all database entries (ivars, jobs, logs, etc.) for this queue.
  bool destroyQueueData();

  /// Load 1 job from the database (adding it to m_liveJobs and returning the shared job pointer).
  std::shared_ptr<Job> loadJob(const smtk::common::UUID& jobId) const;

  /// Load all jobs from the database into m_liveJobs.
  void loadAllJobs() const;

  /// Store 1 job to the database; this assumes the job is already in m_liveJobs.
  ///
  /// This will update the jobs table but may also update the links, job_types, and
  /// job_stages tables. If you just want to update the job's state/status/stage,
  /// call updateJobDatabaseInfo(). If you just want to update the job's links, call
  /// updateJobDatabaseLinks().
  bool storeJob(const std::shared_ptr<smtk::job::Job>& job);

  /// Update a pre-existing row in the jobs table to indicate a new state/status/stage/queueId.
  bool updateJobDatabaseInfo(const std::shared_ptr<smtk::job::Job>& job);

  /// Update the links table to indicate reflect a job's links.
  bool updateJobDatabaseLinks(const std::shared_ptr<smtk::job::Job>& job);

  /// Given a job definition, return its row-id in the database (or -1)
  std::int64_t fetchOrAssignJobTypeId(smtk::job::Definition* jobType);

  /// Add a queue tag to the database tag table.
  bool addTagToDatabase(smtk::string::Token tag);

  /// Remove a queue tag from the database tag table.
  bool removeTagFromDatabase(smtk::string::Token tag);

  /// Jobs from the database which also reside in memory.
  mutable std::unordered_map<smtk::common::UUID, std::shared_ptr<Job>> m_liveJobs;
  /// True when the class destructor should call destroyQueueData().
  bool m_removeQueueOnDestruction{ false };
  /// maximum job size
  int m_maximumSize{ 0 };
  /// Queue location
  std::string m_location;
  /// Integer row ID of this queue in the database "queues" table.
  std::int64_t m_queueId{ -1 };
  /// Database connection
  sqlite3* m_db{ nullptr };
};

} // namespace job
} // namespace smtk

#endif // smtk_job_DatabaseQueue_h
