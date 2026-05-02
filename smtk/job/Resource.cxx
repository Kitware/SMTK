//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkResource.cxx - Abstract base class for CMB resources
// .SECTION Description
// .SECTION See Also

#include "smtk/job/Resource.h"

#include "smtk/common/Paths.h"
#include "smtk/common/UUIDGenerator.h"

#include "smtk/job/Job.h"

#include "smtk/resource/CopyOptions.h"

#include "sqlite3.h"

#include <sstream>

namespace smtk
{
namespace job
{
namespace
{

std::unordered_map<smtk::common::UUID, std::shared_ptr<smtk::job::Job>> g_jobs;

sqlite3* g_db = nullptr;

void installSchema(sqlite3* db)
{
  std::vector<std::string> initializers{
    // Drop existing schema.
    "drop table if exists schema;",
    "drop table if exists jobs;",
    "drop table if exists links;",
    "drop table if exists logs;",
    "drop table if exists container_images;",
    // Create the schema table and insert the schema version.
    "create table schema (version int unique primary key desc);",
    "insert into schema (version) values (1);",
    // Create the tables storing actual job data.
    "create table jobs ("
    "queue_id int64 key,"
    "uid text unique primary key on conflict abort,"
    "size int key,"
    "status int key,"
    "state int key,"
    "case_directory text key unique,"
    "script text,"
    "container_image_id integer key);",
    "create table links ("
    "uid text primary key unique on conflict abort,"
    "job text key,"
    "resource text key,"
    "component text key);",
    "create table logs ("
    "job text key,"
    "log text key);",
    "create table container_images ("
    "url text primary key);"
  };

  sqlite3_stmt* qq;
  const char* tail;
  for (const auto stmt : initializers)
  {
    if (sqlite3_prepare(db, stmt.c_str(), stmt.size(), &qq, &tail) != SQLITE_OK)
    {
      throw std::logic_error("Could not prepare " + stmt);
    }
    if (sqlite3_step(qq) != SQLITE_DONE)
    {
      throw std::logic_error("Could not run " + stmt);
    }
    sqlite3_finalize(qq);
  }
}

void updateSchema(sqlite3* db, int fromVersion)
{
  throw std::logic_error("No upgrades available yet.");
}

bool installOrUpdateSchema(sqlite3* db)
{
  const char* tail;
  sqlite3_stmt* qq;
  std::string check("select version from schema order by version desc limit 1;");
  try
  {
    if (sqlite3_prepare(db, check.c_str(), check.size(), &qq, &tail) != SQLITE_OK)
    {
      // Schema table does not exist; install it.
      installSchema(db);
    }
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
    return true;
  }
  catch (std::logic_error& e)
  {
    std::cerr << "Could not prepare job database: " << e.what() << "\n";
  }
  return false;
}

bool findOrAddContainerImageId(
  sqlite3* db,
  smtk::string::Token url,
  std::uint64_t& containerImageId)
{
  sqlite3_stmt* qq;
  const char* tail;
  std::ostringstream cist;
  cist << "select rowid from container_images where url is '" << url.data() << "';";
  int result = sqlite3_prepare(db, cist.str().c_str(), cist.str().size(), &qq, &tail);
  if (result != SQLITE_OK)
  {
    sqlite3_finalize(qq);
    std::cerr << "ERROR: could not query container_images for '" << url.data() << "'.\n";
    return false;
  }
  try
  {
    result = sqlite3_step(qq);
  }
  catch (std::exception& e)
  {
    std::cerr << "ERROR: Could not search \"" << cist.str() << "\".\n";
    return false;
  }
  if (result == SQLITE_DONE)
  {
    sqlite3_finalize(qq);
    std::ostringstream inserter;
    inserter << "insert into container_images(rowid, url) values (" << url.id() << ", '"
             << url.data() << "');";
    result = sqlite3_prepare(db, inserter.str().c_str(), inserter.str().size(), &qq, &tail);
    if (result != SQLITE_OK)
    {
      std::cerr << "ERROR: Could not prepare insertion into container image table.\n";
    }
    try
    {
      result = sqlite3_step(qq);
    }
    catch (std::exception& e)
    {
      sqlite3_finalize(qq);
      std::cerr << "ERROR: Could not search \"" << cist.str() << "\".\n";
      return false;
    }
    if (result == SQLITE_DONE)
    {
      sqlite3_finalize(qq);
      // This time, we'll find an entry in the table:
      return findOrAddContainerImageId(db, url, containerImageId);
    }
  }
  else if (result == SQLITE_ROW)
  {
    containerImageId = sqlite3_column_int64(qq, 0);
    sqlite3_finalize(qq);
    return true;
  }
  std::cerr << "ERROR: Unknown result " << result << "\n";
  sqlite3_finalize(qq);
  return false;
}

bool sql_add_job(sqlite3* db, const std::shared_ptr<smtk::job::Job>& job)
{
  if (!db || !job)
  {
    return false;
  }
  sqlite3_stmt* qq;
  const char* tail;
  std::uint64_t containerImageId = 0;
  if (!job->containerImage().empty())
  {
    if (!findOrAddContainerImageId(db, job->containerImage(), containerImageId))
    {
      return false;
    }
  }
  std::ostringstream stmt;
  stmt << "insert into jobs(queue_id, uid, size, status, state, case_directory, script, "
          "container_image_id) "
       << "values ('" << (job->queueId().empty() ? "-1" : job->queueId()) << "', '"
       << job->id().toString() << "', " << job->size() << ", " << static_cast<int>(job->status())
       << ", " << static_cast<int>(job->state()) << ", '" << job->caseDirectory().c_str() << "', '"
       << job->script().c_str() << "'," << containerImageId << ");";
  std::cerr << "insertion \"" << stmt.str() << "\"\n";
  int result = sqlite3_prepare(db, stmt.str().c_str(), stmt.str().size(), &qq, &tail);
  std::cerr << "prepared! " << result << " =? " << SQLITE_OK << " tail " << tail << "\n";
  if (result != SQLITE_OK)
  {
    sqlite3_finalize(qq);
    return false;
  }
  try
  {
    result = sqlite3_step(qq);
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception stepping statement: " << e.what() << "\n";
  }
  std::cerr << "stepped! " << result << " =? " << SQLITE_OK << "\n";
  sqlite3_finalize(qq);
  if (result != SQLITE_DONE)
  {
    return false;
  }
  return true;
}

} // anonymous namespace

Resource::Resource()
{
  this->setId(Resource::singletonId());
  this->setName("jobs");
  // This only gets called once: the first time smtk::job::Resource::instance() is called.
  smtk::common::Paths pp;
  std::filesystem::path job_db_directory;
  std::filesystem::path job_db_location;
  job_db_directory = pp.userConfigurationDirectory() / "smtk";
  job_db_location = job_db_directory / "job_database.sqlite3";
  std::filesystem::create_directories(job_db_directory);
  std::string dbp = job_db_location.string();
  if (sqlite3_open(dbp.c_str(), &g_db))
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not open database.");
    return;
  }
  installOrUpdateSchema(g_db);
}

Resource::Resource(const smtk::common::UUID& myID)
  : DirectSuperclass(myID)
{
  this->setName("jobs");
  // This only gets called once: the first time smtk::job::Resource::instance() is called.
}

#if 0
Resource::Resource(const smtk::common::UUID& myID, resource::ManagerPtr manager)
  : DirectSuperclass(myID, manager)
{
  // This should never be called but exists and is private to prevent duplication
  // of the job Resource.
}

Resource::Resource(resource::ManagerPtr manager)
  : DirectSuperclass(manager)
{
  // This should never be called but exists and is private to prevent duplication
  // of the job Resource.
}
#endif // 0

Resource::~Resource()
{
  sqlite3_close_v2(g_db);
  g_db = nullptr;
  std::cerr << "Finalize DB\n";
}

smtk::common::UUID Resource::singletonId()
{
  return smtk::common::UUID("9b5c9884-6c79-4b59-8024-1221c0621826");
}

std::shared_ptr<Resource> Resource::instance()
{
  // We don't need elaborate guards here because this should first be called
  // during plugin registration, rather than in threads during operations.
  static std::shared_ptr<Resource> singleton;
  if (!singleton)
  {
#if 0
    auto* inst = new Resource(Resource::singletonId());
    singleton = inst->shared_from_this();
#endif
    singleton = smtk::job::Resource::create();
  }
  return singleton;
}

smtk::resource::ComponentPtr Resource::find(const smtk::common::UUID& compId) const
{
  return this->findJob(compId);
}

void Resource::visit(std::function<void(const smtk::resource::ComponentPtr&)>& v) const
{
  for (auto [uid, job] : g_jobs)
  {
    v(job);
  }
}

smtk::string::Token Resource::templateType() const
{
  static smtk::string::Token jobTemplateType("jobs");
  return jobTemplateType;
}

std::size_t Resource::templateVersion() const
{
  return 1;
}

bool Resource::addJob(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job)
  {
    return false;
  }

  if (job->id().isNull())
  {
    job->setId(smtk::common::UUID::random());
  }

  auto it = g_jobs.find(job->id());
  if (it != g_jobs.end())
  {
    return false;
  }
  g_jobs[job->id()] = job;
  sql_add_job(g_db, job);
  // TODO: Index by other immutable properties (case directory, logs, script, etc.) or even
  //       mutable properties (queue, state, status, size).
  // TODO: Record permanently immediately.
  return true;
}

std::shared_ptr<smtk::job::Job> Resource::findJob(const smtk::common::UUID& uid) const
{
  auto it = g_jobs.find(uid);
  if (it == g_jobs.end())
  {
    return std::shared_ptr<smtk::job::Job>();
  }
  return it->second;
}

const Resource::GuardedLinks Resource::guardedLinks() const
{
  return GuardedLinks(this->mutex(), this->links());
}

Resource::GuardedLinks Resource::guardedLinks()
{
  return GuardedLinks(this->mutex(), this->links());
}

std::size_t Resource::eraseJob(const std::shared_ptr<Job>& job)
{
  if (!job)
  {
    return 0;
  }
  return g_jobs.erase(job->id());
}

} // namespace job
} // namespace smtk
