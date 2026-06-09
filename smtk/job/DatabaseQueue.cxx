//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/DatabaseQueue.h"

#include "smtk/job/Job.h"
#include "smtk/job/Manager.h"
#include "smtk/job/Stage.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/Paths.h"

#include "sqlite3.h"

#include <fstream>

using namespace smtk::string::literals;

namespace smtk
{
namespace job
{
namespace // anonymous
{

std::vector<std::string> sqlInstallSchema{
  // --- Drop existing tables ---
  R"(drop table if exists schema;)",
  R"(drop table if exists queues;)",
  R"(drop table if exists tags;)",
  R"(drop table if exists job_types;)",
  R"(drop table if exists job_stages;)",
  R"(drop table if exists jobs;)",
  R"(drop table if exists job_links;)",

  // --- Create tables ---
  // Add schema table to indicate how the database is structured.
  R"(create table schema (version int unique primary key desc);)",
  R"(insert into schema (version) values (1);)",
  // Add a table for each instance of the DatabaseQueue class (or subclass).
  // name: the "on conflict replace" clause keeps "update" queries from
  //       failing if they change the old name to the same value as before.
  R"(create table queues (
      id integer primary key,
      uid text unique,
      name text unique on conflict replace,
      description text,
      location text,
      max_size integer
    );)",
  // Add a table to track tags on queues (that mark their capabilities).
  R"(create table tags (
      queue integer key,
      tag text key,
      FOREIGN KEY(queue) REFERENCES queues(id) ON DELETE CASCADE
    );)",
  // Add a table of job-type information. As each job is added, this
  // table is expanded to include the job's definition.
  R"(create table job_types (
      id integer primary key,
      name text unique,
      description text,
      script text,
      container_image_url text key,
      case_directory_mount_point text
    );)",
  // Add a table holding the stages for each job_types table entry.
  R"(create table job_stages (
      id integer primary key,
      job_type integer key,
      stage integer key,
      name text key,
      description text,
      log text,
      FOREIGN KEY(job_type) REFERENCES job_types(id) ON DELETE CASCADE
    );)",
  // Add a table to hold jobs (components of a queue) in a queue.
  R"(create table jobs (
      id integer primary key,
      uid text unique on conflict abort,
      queue integer key,
      job_type integer key,
      job_id text key,
      size int key,
      status integer key,
      state integer key,
      stage integer key,
      auto_schedule integer,
      case_directory text key,
      container_image text key,
      case_directory_mount_point text,
      FOREIGN KEY(queue) REFERENCES queues(id) ON DELETE CASCADE,
      FOREIGN KEY(job_type) REFERENCES job_types(id) ON DELETE CASCADE
    );)",
  // Add a table to hold link information listing objects associated to jobs.
  R"(create table job_links (
      job integer key,
      resource text key,
      component text key,
      FOREIGN KEY(job) REFERENCES jobs(id) ON DELETE CASCADE
    );)"
};

std::unordered_set<smtk::string::Token> g_databaseQueueTags{ "database"_token, "job_v1"_token };

class sqlQuery;

class sqlBinding
{
public:
  sqlBinding(std::size_t index, std::size_t maximumSize = 0)
    : m_index(index)
    , m_maximumSize(maximumSize)
  {
  }

  virtual bool captureValue(sqlQuery&) = 0;

  std::size_t index() const { return m_index; }
  std::size_t maximumSize() const { return m_maximumSize; }

protected:
  std::size_t m_index{ std::size_t(~0) };
  std::size_t m_maximumSize{ 0 };
};

template<typename IntegerType, typename Container>
class sqlBindingInt;
template<typename IntegerType, typename Container>
class sqlBindingText;

class sqlQuery
{
public:
  sqlQuery(sqlite3* db, const std::string& statement = std::string())
    : m_db(db)
  {
    if (!statement.empty())
    {
      m_query << statement << '\0';
      const char* tail;
      if (
        sqlite3_prepare(
          m_db, m_query.str().c_str(), static_cast<int>(m_query.str().size()), &m_cursor, &tail) !=
        SQLITE_OK)
      {
        throw std::logic_error("Could not prepare statement.");
      }
    }
  }

  ~sqlQuery()
  {
    if (m_cursor)
    {
      sqlite3_finalize(m_cursor);
      m_cursor = nullptr;
    }
  }

  std::string str() { return m_query.str(); }

  bool execute()
  {
    if (!m_db)
    {
      return false;
    }

    if (!m_cursor)
    {
      if (m_query.str().empty())
      {
        return false;
      }
      const char* tail;
      if (
        sqlite3_prepare(
          m_db, m_query.str().c_str(), static_cast<int>(m_query.str().size()), &m_cursor, &tail) !=
        SQLITE_OK)
      {
        return false;
      }
      m_query.clear();
      m_query.seekp(0);
    }

    int step;
    for (step = sqlite3_step(m_cursor); step == SQLITE_ROW; step = sqlite3_step(m_cursor))
    {
      for (auto& binding : m_bindings)
      {
        if (!binding->captureValue(*this))
        {
          return false;
        }
      }
    }
    if (step != SQLITE_DONE)
    {
      std::cerr << "Failed to execute query. (code: " << step << ", query: " << m_query.str()
                << ")\n";
    }
    return step == SQLITE_DONE;
  }

  sqlite3_stmt* cursor() const { return m_cursor; }

  template<typename ValueType>
  sqlQuery& operator<<(const ValueType& value)
  {
    // Modifying the query causes finalization of any prior prepared statement.
    if (m_cursor)
    {
      m_bindings.clear();
      sqlite3_finalize(m_cursor);
      m_cursor = nullptr;
      m_query.clear();
      m_query.str("");
      m_query.seekp(0);
    }
    m_query << value;
    return *this;
  }

  template<typename BinderType>
  sqlQuery& bind(std::size_t index, const typename BinderType::container_type& container)
  {
    m_bindings.push_back(std::make_shared<BinderType>(index, container));
    return *this;
  }

  template<typename ContainerType>
  sqlQuery& bindText(std::size_t index, const ContainerType& container)
  {
    m_bindings.push_back(
      std::make_shared<sqlBindingText<typename ContainerType::value_type, ContainerType>>(
        index, container));
    return *this;
  }

  template<typename ContainerType>
  sqlQuery& bindInt(std::size_t index, const ContainerType& container)
  {
    m_bindings.push_back(
      std::make_shared<sqlBindingInt<typename ContainerType::value_type, ContainerType>>(
        index, container));
    return *this;
  }

protected:
  std::ostringstream m_query;
  sqlite3_stmt* m_cursor{ nullptr };
  sqlite3* m_db{ nullptr };
  std::vector<std::shared_ptr<sqlBinding>> m_bindings;
};

template<typename ValueType>
class sqlSingleValueContainer
{
public:
  using value_type = ValueType;

  sqlSingleValueContainer(ValueType& value)
    : m_value(&value)
  {
  }

  std::size_t end() const { return 0; }

  std::size_t size() const { return m_set ? 1 : 0; }
  template<typename Iter>
  Iter insert(Iter location, const ValueType& value)
  {
    if (!m_set)
    {
      *m_value = value;
      m_set = true;
    }
    return location;
  }

protected:
  bool m_set{ false };
  ValueType* m_value{ nullptr };
};

template<typename IntegerType, typename Container = sqlSingleValueContainer<IntegerType>>
class sqlBindingInt : public sqlBinding
{
public:
  using container_type = Container;
  sqlBindingInt(std::size_t index, const Container& values, std::size_t maxSize = 0)
    : sqlBinding(index, maxSize)
    , m_values(const_cast<Container*>(&values))
  {
  }

  bool captureValue(sqlQuery& query) override
  {
    if (this->maximumSize() && m_values->size() >= this->maximumSize())
    {
      return false;
    }
    m_values->insert(
      m_values->end(), static_cast<IntegerType>(sqlite3_column_int(query.cursor(), m_index)));
    return true;
  }

protected:
  Container* m_values{ nullptr };
};

template<typename TextType, typename Container = sqlSingleValueContainer<TextType>>
class sqlBindingText : public sqlBinding
{
public:
  using container_type = Container;
  sqlBindingText(std::size_t index, const Container& values, std::size_t maxSize = 0)
    : sqlBinding(index, maxSize)
    , m_values(const_cast<Container*>(&values))
  {
  }

  bool captureValue(sqlQuery& query) override
  {
    if (this->maximumSize() && m_values->size() >= this->maximumSize())
    {
      return false;
    }
    m_values->insert(
      m_values->end(),
      TextType(reinterpret_cast<const char*>(sqlite3_column_text(query.cursor(), m_index))));
    return true;
  }

protected:
  Container* m_values{ nullptr };
};

bool enablePragmas(sqlite3* db);
bool installSchema(sqlite3* db);
void updateSchema(sqlite3* db, int fromVersion);
bool installOrUpdateSchema(sqlite3* db);

sqlite3* ensureConnection()
{
  sqlite3* conn{ nullptr };
  smtk::common::Paths pp;
  std::filesystem::path job_db_directory;
  std::filesystem::path job_db_location;
  job_db_directory = pp.userConfigurationDirectory();
  job_db_location = job_db_directory / "job_database.sqlite3";
  std::filesystem::create_directories(job_db_directory);
  std::string dbp = job_db_location.string();
  if (sqlite3_open(dbp.c_str(), &conn))
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not open database.");
    return nullptr;
  }
  enablePragmas(conn);
  installOrUpdateSchema(conn);
  return conn;
}

bool installSchema(sqlite3* db)
{
  bool ok = true;
  sqlQuery qq(db);
  for (const auto statement : sqlInstallSchema)
  {
    // std::cerr << "Prepare statement \"" << statement << "\".\n";
    qq << statement;
    ok &= qq.execute();
    if (!ok)
    {
      std::cerr << "Could not prepare or execute statement \"" << statement << "\".\n";
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Could not prepare or execute statement \"" << statement << "\".");
      break;
    }
  }
  return ok;
}

void updateSchema(sqlite3* db, int fromVersion)
{
  throw std::logic_error("No upgrades available yet.");
}

bool enablePragmas(sqlite3* db)
{
  sqlQuery query(db);
  bool ok = true;
  ok &= (query << "pragma foreign_keys = on;").execute();
  ok &= (query << "pragma busy_timeout = 2000;").execute();
  return ok;
}

bool installOrUpdateSchema(sqlite3* db)
{
  const char* tail;
  sqlite3_stmt* qq;
  std::string check("select version from schema order by version desc limit 1;");
  try
  {
    if (sqlite3_prepare(db, check.c_str(), static_cast<int>(check.size()), &qq, &tail) != SQLITE_OK)
    {
      // Schema table does not exist; install it.
      installSchema(db);
    }
    else
    {
      // Schema table exists, find out which version it is.
      if (sqlite3_step(qq) != SQLITE_ROW)
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "No job-table schema version!");
        // No entry in schema table. reset the database.
        installSchema(db);
      }
      else
      {
        int version = sqlite3_column_int(qq, 0);
        if (version < 1)
        {
          updateSchema(db, version);
        }
      }
    }
    sqlite3_finalize(qq);
    return true;
  }
  catch (std::logic_error& e)
  {
    sqlite3_finalize(qq);
    std::cerr << "Could not prepare job database: " << e.what() << "\n";
  }
  return false;
}

} // anonymous namespace

DatabaseQueue::DatabaseQueue()
{
  m_db = ensureConnection();
  this->createQueueData();
}

DatabaseQueue::DatabaseQueue(const smtk::common::UUID& uid)
  : DirectSuperclass(uid)
{
  m_db = ensureConnection();
  if (!this->fetchQueueData())
  {
    this->createQueueData();
  }
}

DatabaseQueue::DatabaseQueue(const smtk::common::UUID& uid, resource::ManagerPtr manager)
  : DirectSuperclass(uid, manager)
{
  m_db = ensureConnection();
  if (!this->fetchQueueData())
  {
    this->createQueueData();
  }
}

DatabaseQueue::DatabaseQueue(resource::ManagerPtr manager)
  : DirectSuperclass(manager)
{
  m_db = ensureConnection();
  if (!this->fetchQueueData())
  {
    this->createQueueData();
  }
}

DatabaseQueue::~DatabaseQueue()
{
  if (m_removeQueueOnDestruction)
  {
    this->destroyQueueData();
  }
  sqlite3_close_v2(m_db);
}

#if 0
std::shared_ptr<DatabaseQueue> DatabaseQueue::createOrRestore(
  const std::string& name,
  const std::string& description,
  const std::string& location,
  int maxJobSize,
  const std::unordered_set<smtk::string::Token>& tags,
  bool removeQueueOnDestruction,
  const smtk::common::UUID& uid,
  const smtk::resource::ManagerPtr& manager,
  const std::shared_ptr<smtk::job::Manager>& jobManager)
{
  smtk::common::UUID qid(uid);
  std::shared_ptr<DatabaseQueue> queue;
  if (name.empty() && uid.isNull())
  {
    return queue;
  }
  if (uid.isNull())
  {
    auto* conn = ensureConnection();
    if (conn)
    {
      bool ok = false;
      std::string uidStr;
      {
        sqlQuery qq(conn);
        qq << "select uid from queues where name='" << name << "';";
        qq.bind<sqlBindingText<std::string>>(0, uidStr);
        ok = qq.execute();
      }
      if (ok)
      {
        qid = smtk::common::UUID(uidStr);
      }
      sqlite3_close(conn);
    }
    if (qid.isNull())
    {
      qid = smtk::common::UUID::random();
    }
  }
  queue = std::make_shared<DatabaseQueue>(qid, manager);
  queue->m_maximumSize = maxJobSize;
  queue->m_location = location;
  queue->m_name = name;
  queue->m_description = description;
  queue->m_tags = tags;
  queue->m_removeQueueOnDestruction = removeQueueOnDestruction;
  queue->updateQueueData();
  return queue;
}
#endif // 0

bool DatabaseQueue::setRemoveQueueOnDestruction(bool shouldRemove)
{
  if (m_removeQueueOnDestruction == shouldRemove)
  {
    return false;
  }
  m_removeQueueOnDestruction = shouldRemove;
  return true;
}

bool DatabaseQueue::setId(const common::UUID& uid)
{
  smtk::common::UUID prevId = this->id();
  bool nextIdHasData = this->hasQueueData(uid);
  if (this->Superclass::setId(uid))
  {
    // We were allowed to update the UUID
    if (nextIdHasData)
    {
      // Reset this instance's ivars to match what is already
      // present in the database at the new UUID.
      this->fetchQueueData();
    }
    else
    {
      // Update queue UUID in database
      this->updateQueueId(prevId, uid);
    }
    return true;
  }
  return false;
}

bool DatabaseQueue::setName(const std::string& name)
{
  if (this->Superclass::setName(name))
  {
    return this->updateQueueData();
  }
  return false;
}

bool DatabaseQueue::setDescription(const std::string& description)
{
  if (this->Superclass::setDescription(description))
  {
    return this->updateQueueData();
  }
  return false;
}

smtk::resource::ComponentPtr DatabaseQueue::find(const smtk::common::UUID& compId) const
{
  return this->findJob(compId);
}

void DatabaseQueue::visit(std::function<void(const smtk::resource::ComponentPtr&)>& v) const
{
  // First, ensure all jobs are live jobs.
  this->loadAllJobs();
  // Then, iterate live jobs.
  for (auto [uid, job] : m_liveJobs)
  {
    v(job);
  }
}

smtk::string::Token DatabaseQueue::templateType() const
{
  static smtk::string::Token jobTemplateType("job_db");
  return jobTemplateType;
}

std::size_t DatabaseQueue::templateVersion() const
{
  return 1;
}

bool DatabaseQueue::addTag(smtk::string::Token tag)
{
  if (this->Superclass::addTag(tag))
  {
    return this->addTagToDatabase(tag);
  }
  return false;
}

bool DatabaseQueue::removeTag(smtk::string::Token tag)
{
  auto it = g_databaseQueueTags.find(tag);
  if (it != g_databaseQueueTags.end())
  {
    return false;
  }
  if (this->Superclass::removeTag(tag))
  {
    return this->removeTagFromDatabase(tag);
  }
  return false;
}

std::string DatabaseQueue::location() const
{
  // TODO: Fetch location from database.
  return std::string();
}

std::uint64_t DatabaseQueue::maximumJobSize() const
{
  // TODO: Fetch maximum size from database.
  return 0;
}

std::shared_ptr<smtk::job::Job> DatabaseQueue::findJob(const smtk::common::UUID& uid) const
{
  auto it = m_liveJobs.find(uid);
  if (it != m_liveJobs.end())
  {
    return it->second;
  }

  return this->fetchJobData(uid);
}

std::shared_ptr<smtk::job::Definition> DatabaseQueue::fetchJobTypeData(
  const std::string& jobTypeName) const
{
  std::shared_ptr<smtk::job::Definition> jobType;
  auto* jobManager = this->jobManager();
  if (!jobManager)
  {
    return jobType;
  }
  jobType = jobManager->jobTypes().findByName(jobTypeName);
  if (!jobType)
  {
    // No such job type has been registered with the job manager.
    // Fetch it and register.
    jobType = this->fetchJobTypeDataSql(jobTypeName);
  }
  return jobType;
}

std::shared_ptr<smtk::job::Definition> DatabaseQueue::fetchJobTypeDataSql(
  const std::string& jobTypeName) const
{
  std::shared_ptr<smtk::job::Definition> jobType;
  std::int64_t definitionId;
  std::string description;
  std::string script;
  std::string containerImage;
  std::string caseDirectoryMountPoint;
  sqlQuery query(m_db);
  query << "select id, description, script, container_image_url, case_directory_mount_point "
        << "from job_types where name='" << jobTypeName << "';";
  query.bind<sqlBindingInt<std::int64_t>>(0, definitionId)
    .bind<sqlBindingText<std::string>>(1, description)
    .bind<sqlBindingText<std::string>>(2, script)
    .bind<sqlBindingText<std::string>>(3, containerImage)
    .bind<sqlBindingText<std::string>>(4, caseDirectoryMountPoint);
  if (!query.execute())
  {
    return jobType;
  }

  std::vector<int> stageIndices;
  std::vector<std::string> stageNames;
  std::vector<std::string> stageDescriptions;
  std::vector<std::string> stageLogPaths;
  query << "select stage, name, description, log from job_stages where id=" << definitionId
        << " order by stage ascending;";
  query.bindInt(0, stageIndices)
    .bindText(1, stageNames)
    .bindText(2, stageDescriptions)
    .bindText(3, stageLogPaths);
  if (!query.execute())
  {
    // Job definitions must have at least one stage.
    return jobType;
  }

  jobType = smtk::job::Definition::create();
  jobType->setName(jobTypeName);
  jobType->setDescription(description);
  jobType->setScript(script);
  jobType->setContainerImage(containerImage);
  jobType->setCaseDirectoryMountPoint(caseDirectoryMountPoint);

  for (std::size_t ii = 0; ii < stageIndices.size(); ++ii)
  {
    // TODO: We could check that the returned index matches stageIndices[ii].
    jobType->appendStage(stageNames[ii], stageDescriptions[ii], stageLogPaths[ii]);
  }
  m_jobManager->jobTypes().manage(jobType);
  return jobType;
}

std::shared_ptr<smtk::job::Job> DatabaseQueue::fetchJobData(const smtk::common::UUID& uid) const
{
  std::shared_ptr<smtk::job::Job> job;
  sqlQuery query(m_db);
  query << "select "
        << "id, job_type, size, job_id, status, state, stage, case_directory, auto_schedule, "
           "container_image, case_directory_mount_point "
        << "from jobs where uid='" << uid.toString() << "' and queue=" << m_queueId << ";";
  std::int64_t jobRowId = -1;
  std::int64_t jobTypeRow = -1;
  std::uint64_t jobSize = -1;
  int jobStatus = -1;
  int jobState = -1;
  int jobStage = -1;
  int jobAutoSchedule = -1;
  std::string jobQueueId;
  std::string jobCaseDir;
  std::string jobImageUrl;
  std::string jobMountPoint;
  query.bind<sqlBindingInt<std::int64_t>>(0, jobRowId)
    .bind<sqlBindingInt<std::int64_t>>(1, jobTypeRow)
    .bind<sqlBindingInt<std::uint64_t>>(2, jobSize)
    .bind<sqlBindingText<std::string>>(3, jobQueueId)
    .bind<sqlBindingInt<int>>(4, jobStatus)
    .bind<sqlBindingInt<int>>(5, jobState)
    .bind<sqlBindingInt<int>>(6, jobStage)
    .bind<sqlBindingText<std::string>>(7, jobCaseDir)
    .bind<sqlBindingInt<int>>(8, jobAutoSchedule)
    .bind<sqlBindingText<std::string>>(9, jobImageUrl)
    .bind<sqlBindingText<std::string>>(10, jobMountPoint);
  if (!query.execute() || jobRowId == -1 || jobTypeRow == -1)
  {
    return job;
  }
  std::string jobTypeName;
  query << "select name from job_types where id=" << jobTypeRow << ";";
  query.bind<sqlBindingText<std::string>>(0, jobTypeName);
  if (!query.execute() || jobTypeName.empty())
  {
    return job;
  }
  job = smtk::job::Job::create();
  auto jobType = this->fetchJobTypeData(jobTypeName);
  job->restore(
    const_cast<DatabaseQueue*>(this),
    uid,
    jobType.get(),
    jobSize,
    static_cast<smtk::job::State>(jobState),
    static_cast<smtk::job::Status>(jobStatus),
    jobStage,
    jobQueueId,
    jobCaseDir,
    jobImageUrl,
    jobMountPoint,
    jobAutoSchedule != 0);
  // TODO: Restore job links
  m_liveJobs[uid] = job;
  return job;
}

bool DatabaseQueue::add(const std::shared_ptr<Job>& job)
{
  if (!job || job->queue() != this || m_liveJobs.find(job->id()) != m_liveJobs.end())
  {
    return false;
  }
  sqlQuery query(m_db);
  query << "select count(*) from jobs where uid='" << job->id().toString() << "';";
  int matching = 0;
  if (!query.bind<sqlBindingInt<int>>(0, matching).execute() || matching > 0)
  {
    // Job may not be "live" but previously existed somehow.
    return false;
  }
  // Now we know the job is not already owned by this queue but reports this queue
  // as its parent. Add it.
  m_liveJobs[job->id()] = job;
  return this->storeJob(job);
}

bool DatabaseQueue::schedule(const std::shared_ptr<Job>& job)
{
  std::ostringstream commandLine;
  commandLine << (job->caseDirectory() / job->script()).string();
  job->setStage(-1);
  job->setState(State::Running);
  job->setStatus(Status::Pending);

  // Temporarily set the current working directory to the case directory
  // before spawning the job script.
  auto prevPath = std::filesystem::current_path();
  std::filesystem::current_path(job->caseDirectory());
  int jobId = std::system(commandLine.str().c_str());
  std::filesystem::current_path(prevPath);

  job->setQueueId(std::to_string(jobId));
  if (jobId == 0)
  {
    job->setState(State::Completed);
    job->setStatus(Status::Failed);
  }
  return jobId != 0;
}

bool DatabaseQueue::cancel(const std::shared_ptr<Job>& job)
{
  return false;
}

State DatabaseQueue::jobState(const std::shared_ptr<Job>& job)
{
  if (!job)
  {
    return State::Unscheduled;
  }
  if (job->state() == State::Running)
  {
    // Check whether the log file exists.
    auto sf = std::ifstream(job->caseDirectory() / "logs" / "progress");
    if (!sf)
    {
      // File may not be created by the script yet.
      return State::Running;
    }
    int stage;
    sf >> stage;
    if (stage >= static_cast<int>(job->jobType()->stages().size()))
    {
      job->setState(State::Completed);
      job->setStatus(Status::Succeeded);
    }
  }

  return job->state();
}

Status DatabaseQueue::jobStatus(const std::shared_ptr<Job>& job)
{
  if (!job)
  {
    return Status::Pending;
  }
  return job->status();
}

std::set<std::shared_ptr<Job>> DatabaseQueue::allJobs() const
{
  this->loadAllJobs();
  std::set<std::shared_ptr<Job>> result;
  for (auto [uid, job] : m_liveJobs)
  {
    result.insert(job);
  }
  return result;
}

std::shared_ptr<DatabaseQueue> DatabaseQueue::findQueue(
  const std::shared_ptr<smtk::resource::Manager>& resourceManager,
  const smtk::common::UUID& uid,
  const std::string& name)
{
  std::shared_ptr<DatabaseQueue> queue;
  if (!resourceManager)
  {
    return queue;
  }
  if (!uid.isNull())
  {
    auto rsrc = resourceManager->get(uid);
    if (!rsrc)
    {
      return queue;
    }
    queue = std::dynamic_pointer_cast<DatabaseQueue>(rsrc);
    if (!queue)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "A resource with the given UUID already exists but is not a queue.");
      return queue;
    }
    if (queue->name() != name)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "A queue with the given UUID already exists but has a different name (" << queue->name()
                                                                                << ").");
      queue.reset();
      return queue;
    }
  }
  if (!name.empty())
  {
    auto rsrcs = resourceManager->findByName(name);
    for (const auto& rsrc : rsrcs)
    {
      queue = std::dynamic_pointer_cast<DatabaseQueue>(rsrc);
      if (queue)
      {
        break;
      }
    }
  }
  return queue;
}

smtk::common::UUID DatabaseQueue::uuidFromDatabase(
  const smtk::common::UUID& uid,
  const std::string& name)
{
  smtk::common::UUID qid(uid);
  std::shared_ptr<DatabaseQueue> queue;
  if (name.empty() && uid.isNull())
  {
    return uid;
  }
  auto* conn = ensureConnection();
  if (conn)
  {
    if (uid.isNull())
    {
      bool ok = false;
      std::string uidStr;
      {
        sqlQuery qq(conn);
        qq << "select uid from queues where name='" << name << "';";
        qq.bind<sqlBindingText<std::string>>(0, uidStr);
        ok = qq.execute();
      }
      if (ok)
      {
        qid = smtk::common::UUID(uidStr);
      }
    }
    else
    {
      // We were given both a name and UUID or just a UUID.
      // Fetch the name matching the UUID (or fail) and check against
      // the given name (if provided).
      std::string dbName;
      bool ok = false;
      {
        sqlQuery qq(conn);
        qq << "select name from queues where uid='" << uid.toString() << "';";
        qq.bind<sqlBindingText<std::string>>(0, dbName);
        ok = qq.execute();
      }
      if (ok)
      {
        if (!name.empty() && !dbName.empty())
        {
          if (name != dbName)
          {
            // Name mismatch. Set the "actual" UUID to null to indicate a problem.
            qid = smtk::common::UUID::null();
          }
          else
          {
            qid = uid;
          }
        }
      }
    }
    sqlite3_close(conn);
    if (qid.isNull())
    {
      qid = smtk::common::UUID::random();
    }
  }
  if (!uid.isNull() && uid != qid)
  {
    // We were given a UID but it does not match the database entry.
    qid = smtk::common::UUID::null();
  }
  return qid;
}

void DatabaseQueue::createQueueData()
{
  bool ok = true;
  {
    sqlQuery qq(m_db);
    qq << "insert into queues (uid, name, description, location, max_size) values ("
       << "'" << this->id().toString() << "', '" << m_name << "', '" << m_description << "',"
       << "'" << this->location() << "', " << this->maximumJobSize() << ");";
    ok = qq.execute();
  }
  m_queueId = static_cast<std::int64_t>(sqlite3_last_insert_rowid(m_db));
  // No need to push tags here as only constructors call this method and none of
  // them accept tags. DatabaseQueue::createOrRestore() calls updateQueueData()
  // rather than this method—and it handles tags.
}

bool DatabaseQueue::hasQueueData(const smtk::common::UUID& uid) const
{
  std::ostringstream statement;
  statement << "select count(*) from queues where uid is '" << uid.toString() << "';";

  sqlite3_stmt* qq;
  const char* tail;
  if (
    sqlite3_prepare(
      m_db, statement.str().c_str(), static_cast<int>(statement.str().size()), &qq, &tail) !=
    SQLITE_OK)
  {
    throw std::logic_error("Could not prepare " + statement.str());
  }
  // Table exists; we should have a count.
  if (sqlite3_step(qq) != SQLITE_ROW)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No queues table!");
    // No entry in schema table. reset the database.
    installSchema(m_db);
  }
  else
  {
    int count = sqlite3_column_int(qq, 0);
    sqlite3_finalize(qq);
    return count >= 1;
  }
  sqlite3_finalize(qq);
  return false;
}

bool DatabaseQueue::fetchQueueData()
{
  std::string name;
  std::string description;
  std::string location;
  int maximumSize{ -1 };
  sqlQuery qq(m_db);
  qq << "select id, name, description, location, max_size from queues where uid='"
     << this->id().toString() << "';";
  qq.bind<sqlBindingInt<std::int64_t>>(0, m_queueId)
    .bind<sqlBindingText<std::string>>(1, name)
    .bind<sqlBindingText<std::string>>(2, description)
    .bind<sqlBindingText<std::string>>(3, location)
    .bind<sqlBindingInt<int>>(4, maximumSize);
  bool ok = qq.execute();

  if (!ok)
  {
    std::cerr << "Could not fetch queue data\n";
    // throw std::logic_error("Could not prepare <<" + statement.str() + ">>");
    return false;
  }
  if (m_queueId < 0)
  {
    // Query executed, but returned no row.
    return false;
  }
  bool didModify = false;
  if (name != m_name)
  {
    didModify = true;
    m_name = name;
  }
  if (description != m_description)
  {
    didModify = true;
    m_description = description;
  }
  if (location != m_location)
  {
    didModify = true;
    m_location = location;
  }
  if (m_maximumSize != maximumSize)
  {
    didModify = true;
    m_maximumSize = maximumSize;
  }

  std::unordered_set<smtk::string::Token> tags;
  qq << "select tag from tags where queue=" << m_queueId << ";";
  qq.bindText(0, tags);
  ok = qq.execute();
  if (!ok)
  {
    std::cerr << "Could not fetch queue tags\n";
    return false;
  }
  if (tags != m_tags)
  {
    didModify = true;
    m_tags = tags;
  }
  return didModify;
}

bool DatabaseQueue::fetchJobLinks(const std::shared_ptr<Job>& job)
{
  if (!job)
  {
    return false;
  }
  auto resourceManager = this->manager();
  if (!resourceManager)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "No resource manager available; "
      "cannot restore links for job "
        << job->id().toString() << ".");
    return false;
  }

  sqlQuery query(m_db);
  query << "select id from jobs where uid='" << job->id().toString() << "';";
  std::int64_t jobId = -1;
  query.bind<sqlBindingInt<std::int64_t>>(0, jobId);
  if (!query.execute() || jobId < 0)
  {
    return false;
  }
  query << "select resource,component from job_links where job=" << jobId
        << " order by resource, component;";
  std::vector<smtk::common::UUID> rsrcs;
  std::vector<smtk::common::UUID> comps;
  query.bindText(0, rsrcs).bindText(1, comps);
  query.execute();

  smtk::resource::PersistentObjectSet linkSet =
    job->links().linkedTo(smtk::job::Queue::JobOriginRole);
  std::size_t nn = rsrcs.size();
  if (nn == 0)
  {
    for (const auto& obj : linkSet)
    {
      if (auto rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(obj))
      {
        job->links().removeLinksTo(rsrc, smtk::job::Queue::JobOriginRole);
      }
      else if (auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj))
      {
        job->links().removeLinksTo(comp, smtk::job::Queue::JobOriginRole);
      }
    }
    return !linkSet.empty(); // Return true if edits were needed.
  }
  smtk::resource::PersistentObjectSet toErase = linkSet;
  bool didModify = false;
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    if (comps[ii].isNull())
    {
      // Linked object is a resource.
      auto rsrc = resourceManager->get(rsrcs[ii]);
      if (!rsrc)
      {
        smtkWarningMacro(
          smtk::io::Logger::instance(), "No such resource \"" << rsrcs[ii].toString() << "\".");
        continue;
      }
      if (linkSet.find(rsrc) == linkSet.end())
      {
        // The resource was not linked. Link it.
        didModify = true;
        job->links().addLinkTo(rsrc, smtk::job::Queue::JobOriginRole);
      }
      else
      {
        // Do not erase the resource.
        toErase.erase(rsrc);
      }
    }
    else
    {
      // Linked object is a component.
      auto rsrc = resourceManager->get(rsrcs[ii]);
      auto comp = rsrc ? rsrc->find(comps[ii]) : smtk::resource::Component::Ptr();
      if (!comp)
      {
        smtkWarningMacro(
          smtk::io::Logger::instance(),
          "No such component \"" << comps[ii].toString()
                                 << "\" "
                                    "in  resource \""
                                 << rsrcs[ii].toString() << "\".");
        continue;
      }
      if (linkSet.find(comp) == linkSet.end())
      {
        // The component was not linked. Link it.
        didModify = true;
        job->links().addLinkTo(comp, smtk::job::Queue::JobOriginRole);
      }
      else
      {
        // Do not erase the component.
        toErase.erase(comp);
      }
    }
  }
  // Anything that remains in toErase should be unlinked from the job.
  for (const auto& obj : toErase)
  {
    if (auto rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(obj))
    {
      didModify = true;
      job->links().removeLinksTo(rsrc, smtk::job::Queue::JobOriginRole);
    }
    else if (auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj))
    {
      didModify = true;
      job->links().removeLinksTo(comp, smtk::job::Queue::JobOriginRole);
    }
  }
  return didModify;
}

bool DatabaseQueue::updateQueueId(
  const smtk::common::UUID& prevId,
  const smtk::common::UUID& nextId)
{
  if (prevId == nextId)
  {
    return false;
  }

  sqlQuery query(m_db);
  query << "update queues set uid='" << nextId.toString() << "' where uid='" << prevId.toString()
        << "';";
  return query.execute();
}

bool DatabaseQueue::updateQueueData()
{
  sqlQuery query(m_db);
  query << "update queues set"
        << " id=" << m_queueId << ","
        << " name='" << m_name << "',"
        << " description='" << m_description << "',"
        << " max_size='" << m_maximumSize << "',"
        << " location='" << m_location << "'"
        << " where uid='" << this->id().toString() << "';";
  if (!query.execute())
  {
    return false;
  }

  // Now handle tags.
  query << "delete from tags where queue=" << m_queueId << ";";
  if (!query.execute())
  {
    return false;
  }
  if (!m_tags.empty())
  {
    query << "insert into tags(queue, tag) values";
    bool comma = false;
    for (const auto tag : m_tags)
    {
      if (comma)
      {
        query << ",";
      }
      else
      {
        comma = true;
      }
      query << " (" << m_queueId << ",'" << tag.data() << "')";
    }
    query << ";";
    if (!query.execute())
    {
      return false;
    }
  }
  return true;
}

bool DatabaseQueue::destroyQueueData()
{
  sqlQuery query(m_db);
  query << "delete from queues where id=" << m_queueId << ";";
  bool ok = query.execute();
  return ok;
}

std::shared_ptr<Job> DatabaseQueue::loadJob(const smtk::common::UUID& jobId) const
{
  auto it = m_liveJobs.find(jobId);
  if (it != m_liveJobs.end())
  {
    return it->second;
  }
  /// TODO: Search for job in database and add to m_liveJobs.

  return std::shared_ptr<Job>();
}

void DatabaseQueue::loadAllJobs() const
{
  constexpr int delta = 1024;
  int offset = 0;
  sqlQuery query(m_db);
  std::vector<smtk::common::UUID> jobIds;
  for (jobIds.reserve(delta); true; jobIds.clear(), offset += delta)
  {
    query << "select uid from jobs order by uid limit " << delta << " offset " << offset << ";";
    if (!query.bindText(0, jobIds).execute())
    {
      break;
    }
    for (const auto& jobId : jobIds)
    {
      this->loadJob(jobId);
    }
    if (jobIds.size() < delta)
    {
      break;
    }
  }
}

bool DatabaseQueue::storeJob(const std::shared_ptr<smtk::job::Job>& job)
{
  std::int64_t jobTypeId = this->fetchOrAssignJobTypeId(job->jobType());
  sqlQuery query(m_db);
  query << "insert into jobs("
        << "uid,queue,job_type,job_id,size,"
        << "status,state,stage,"
        << "auto_schedule,case_directory,"
        << "container_image,case_directory_mount_point) values ("
        << "'" << job->id().toString() << "'"
        << "," << m_queueId << "," << jobTypeId << ",'" << job->queueId() << "'"
        << "," << job->size() << "," << static_cast<int>(job->status()) << ","
        << static_cast<int>(job->state()) << "," << job->stage() << ","
        << (job->autoSchedule() ? 1 : 0) << ",'" << job->caseDirectory().string() << "'"
        << ",'" << job->containerImage() << "'"
        << ",'" << job->caseDirectoryMountPoint() << "'"
        << ");";
  if (!query.execute())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Could not insert job " << job->id() << " into table.");
    return false;
  }
  return this->updateJobDatabaseLinks(job);
}

bool DatabaseQueue::updateJobDatabaseInfo(const std::shared_ptr<smtk::job::Job>& job)
{
  sqlQuery query(m_db);
  query << "update jobs set "
        << "job_id='" << job->queueId() << "', size=" << job->size() << ", "
        << "status=" << static_cast<int>(job->status()) << ","
        << "state=" << static_cast<int>(job->state()) << ", "
        << "stage=" << job->stage() << " "
        << "where uid='" << job->id().toString() << "';";
  bool ok = query.execute();
  return ok;
}

bool DatabaseQueue::updateJobDatabaseLinks(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job)
  {
    return true;
  }

  sqlQuery query(m_db);
  query << "select id from jobs where uid='" << job->id().toString() << "';";
  std::int64_t jobId = -1;
  query.bind<sqlBindingInt<std::int64_t>>(0, jobId);
  if (!query.execute() || jobId < 0)
  {
    // No such job in jobs table.
    smtkWarningMacro(
      smtk::io::Logger::instance(), "No such job " << job->id().toString() << " in jobs table.");
    return false;
  }

  std::vector<std::int64_t> rowIds;
  std::vector<smtk::common::UUID> rsrcs;
  std::vector<smtk::common::UUID> comps;
  query << "select rowid,resource,component from job_links where job=" << jobId << ";";
  query.bindInt(0, rowIds).bindText(1, rsrcs).bindText(2, comps);
  if (!query.execute())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not fetch links for job " << jobId << " (" << job->id().toString() << ").");
    std::cerr << "Query \"" << query.str() << "\"\n";
    return false;
  }

  bool didModify = false;
  std::set<smtk::resource::PersistentObject::Ptr> add;
  std::set<std::int64_t> drop;
  std::size_t nn = rowIds.size();
  std::set<std::int64_t> found;
  for (const auto& ll : job->links().linkedTo(smtk::job::Queue::JobOriginRole))
  {
    for (std::size_t ii = 0; ii < nn; ++ii)
    {
      if ((ll->id() == comps[ii]) || (ll->id() == rsrcs[ii] && comps[ii].isNull()))
      {
        found.insert(rowIds[ii]);
      }
      else
      {
        add.insert(ll);
      }
    }
  }
  for (const auto& rowId : rowIds)
  {
    if (found.find(rowId) == found.end())
    {
      (query << "delete from job_links where id=" << rowId << ";").execute();
      didModify = true;
    }
  }
  if (!add.empty())
  {
    didModify = true;
    query << "insert into job_links(job,resource,component) values ";
    bool needComma = false;
    for (const auto& obj : add)
    {
      if (!needComma)
      {
        needComma = true;
      }
      else
      {
        query << ",";
      }
      if (obj->matchesType("smtk::resource::Resource"_token))
      {
        query << "(" << jobId << ","
              << "'" << obj->id().toString() << "','00000000-0000-0000-0000-000000000000')";
      }
      else
      {
        auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj);
        auto rsrc = comp ? comp->resource() : smtk::resource::Resource::Ptr();
        if (!comp || !rsrc)
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Object " << obj->id().toString() << " is not a component or a resource.");
          continue;
        }
        query << "(" << jobId << ","
              << "'" << rsrc->id().toString() << "',"
              << "'" << comp->id().toString() << "')";
      }
    }
    query << ";";
    query.execute();
  }
  return didModify;
}

std::int64_t DatabaseQueue::fetchOrAssignJobTypeId(smtk::job::Definition* jobType)
{
  if (!jobType)
  {
    return -1;
  }
  sqlQuery query(m_db);
  query << "select id from job_types where name='" << jobType->name() << "';";
  std::int64_t jobTypeId = -1;
  query.bind<sqlBindingInt<std::int64_t>>(0, jobTypeId);
  query.execute();
  if (jobTypeId == -1)
  {
    // Need to insert a record for this job type.
    query << "insert into job_types"
          << "(name,description,script,container_image_url,case_directory_mount_point)"
          << " values "
          << "('" << jobType->name() << "'"
          << ",'" << jobType->description() << "'"
          << ",'" << jobType->script().string() << "'"
          << ",'" << jobType->containerImage() << "'"
          << ",'" << jobType->caseDirectoryMountPoint() << "'"
          << ");";
    if (!query.execute())
    {
      return jobTypeId;
    }
    jobTypeId = static_cast<std::int64_t>(sqlite3_last_insert_rowid(m_db));
    for (const auto& stage : jobType->stages())
    {
      query << "insert into job_stages"
            << "(job_type,stage,name,description,log)"
            << " values "
            << "(" << jobTypeId << "," << stage->index() << ",'" << stage->name() << "'"
            << ",'" << stage->description() << "'"
            << ",'" << stage->log().string() << "'"
            << ";";
      query.execute();
    }
  }
  return jobTypeId;
}

bool DatabaseQueue::addTagToDatabase(smtk::string::Token tag)
{
  sqlQuery query(m_db);
  query << "insert into tags (queue, tag) values (" << m_queueId << ",'" << tag.data() << "');";
  bool ok = query.execute();
  return ok;
}

bool DatabaseQueue::removeTagFromDatabase(smtk::string::Token tag)
{
  sqlQuery query(m_db);
  query << "delete from tags where queue=" << m_queueId << " and tag='" << tag.data() << "';";
  bool ok = query.execute();
  return ok;
}

} // namespace job
} // namespace smtk
