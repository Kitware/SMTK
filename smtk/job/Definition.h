//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_Definition_h
#define smtk_job_Definition_h

#include "smtk/job/LogParser.h"

#include "smtk/string/Token.h"

#include "smtk/SharedFromThis.h"

namespace smtk
{
namespace job
{

class Job;
class Queue;
class Stage;

/// A job definition holds information common to a type of job.
///
/// # Overview
///
/// SMTK models long-running jobs as components owned by queues that have their
/// state backed by and updated by the queue.
/// In a task-based workflow, tasks create computational jobs as users complete
/// tasks that prepare the information needed to run the job.
/// Thus the same workflow may create similar jobs many times (across different
/// projects or within the same project). The similar information (i.e., the
/// stages of the job, the location of logs and input files inside the job's
/// case directory) are modeled as a job **definition** to avoid duplicating
/// this information many times.
///
/// While jobs are owned an managed by a Queue, job definitions are largely
/// independent of which queue a job is submitted to. For this reason, definitions
/// are not owned by a queue or by the job::Resource singleton. Instead, the
/// job::Manager provides a way for plugins to register job definitions as they
/// are loaded.
class SMTKCORE_EXPORT Definition : public std::enable_shared_from_this<Definition>
{
public:
  smtkTypeMacroBase(smtk::job::Definition);
  smtkCreateMacro(smtk::job::Definition);
  smtkSharedFromThisMacro(smtk::job::Definition);

  /// A function that returns a shared pointer to an instance of a log parser.
  using LogParserConstructor = std::function<std::shared_ptr<LogParser>()>;
  /// The type of map used to store parser-constructors for the various log files.
  using LogParserConstructorMap = std::unordered_map<smtk::string::Token, LogParserConstructor>;

  /// Destroy a job (from memory, but not from persistent storage if the job
  /// is owned by the job::Resource).
  virtual ~Definition() = default;

  ///@{
  /// Set/get the job type's user-presentable name.
  std::string name() const;
  bool setName(const std::string& name);
  ///@}

  ///@{
  /// Set/get a description of the job type.
  std::string description() const;
  bool setDescription(const std::string& description);
  ///@}

  ///@{
  /// Set/get the path to the job's run script. This path must be relative to the case directory.
  std::filesystem::path script() const;
  bool setScript(std::filesystem::path dir);
  ///@}

  ///@{
  /// Manage the stages of a job.
  ///
  /// These methods allow operations which construct jobs to add stages to it.
  /// A job is expected to have at least one stage.
  /// Each stage may create its own log file.
  /// Facilities are not provided to remove or alter stages because they are
  /// static properties of jobs.
  ///
  /// The variant which accepts a \a description and \a log path creates a
  /// new stage and appends that instance using the first variant.
  int appendStage(const std::shared_ptr<smtk::job::Stage>& stage);
  int appendStage(const std::string& name, const std::string& description);
  int appendStage(
    const std::string& name,
    const std::string& description,
    const std::filesystem::path& log);
  const std::vector<std::shared_ptr<smtk::job::Stage>>& stages() const;
  ///@}

  ///@{
  /// Set/get the container image to use for this job.
  ///
  /// This string is a URL indicating the host serving containers
  /// as well as the name and version of the container on that host.
  ///
  /// This is unused unless a queue requires it.
  /// You cannot submit a job without a container image URL to a queue
  /// that uses containers; however, you can submit a job with a
  /// container image URL to a queue that does not use containers – this
  /// setting will just be ignored.
  std::string containerImage() const { return m_containerImage; }
  bool setContainerImage(const std::string& imageURL);
  ///@}

  ///@{
  /// Set/get the location within the image of where to mount the case directory.
  std::string caseDirectoryMountPoint() const { return m_caseDirectoryMountPoint; }
  bool setCaseDirectoryMountPoint(const std::string& mountPoint);
  ///@}

  ///@{
  /// Manage log-file parsers.
  ///
  /// The \a logPath parameter is relative path from the case directory to the log file.
  /// Log files must live inside their case directories.
  const LogParserConstructorMap& logParserConstructors() const;
  template<typename LogParserType>
  bool setLogParserConstructor(smtk::string::Token logPath)
  {
    if (m_logParserConstructors.find(logPath) != m_logParserConstructors.end())
    {
      return false;
    }
    m_logParserConstructors[logPath] = []() { return std::make_shared<LogParserType>(); };
    return true;
  }
  bool clearLogParserConstructor(smtk::string::Token logPath);
  bool resetLogParserConstructors();
  ///@}

protected:
  Definition();

  std::string m_name;
  std::filesystem::path m_script;
  int m_index{ -1 };
  std::string m_description;
  std::string m_containerImage;
  std::string m_caseDirectoryMountPoint;
  std::vector<std::shared_ptr<smtk::job::Stage>> m_stages;
  LogParserConstructorMap m_logParserConstructors;
};

} // namespace job
} // namespace smtk

#endif // smtk_job_Definition_h
