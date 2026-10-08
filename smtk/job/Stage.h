//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_Stage_h
#define smtk_job_Stage_h

#include "smtk/string/Token.h"

#include "smtk/SharedFromThis.h"

#include <filesystem>
#include <unordered_set>

namespace smtk
{
namespace job
{

class Definition;
class Queue;

/// A stage is a segment of a job.
///
/// # Overview
///
/// A job::Definition may have 1 or many stages but should always have at least one stage.
/// Each stage has a unique integer index within its parent definition.
/// At this time, definitions are expected to have a fixed number of sequential
/// stage objects (i.e., the number and descriptions of stages should not
/// change over the life of the job). Stages do not hold any information
/// that varies as a job progresses. Instead, they store data common to
/// all jobs of the same type.
///
/// A stage has a (user-presentable) description.
///
/// A stage may produce "artifacts," which are files in some external (non-SMTK)
/// format. Examples include simulation results, geometric models, and summary
/// information like feature statistics. A log file is handled separately (see below).
///
/// A stage may have its own log path (relative to the case directory).
/// A queue (not the stage itself) may report the progress of a stage (in [0,1]
/// as a floating point number) for a job or not.
///
/// A special "logs/progress" file in a job's case directory is expected to be
/// created/overwritten at job start with "-1" and be overwritten as the job
/// proceeds with the active stage's index.
/// If it exists, it is used to provide updates to any observers monitoring the job.
class SMTKCORE_EXPORT Stage : public std::enable_shared_from_this<Stage>
{
public:
  smtkTypeMacroBase(smtk::job::Stage);
  smtkCreateMacro(smtk::job::Stage);

  /// Destroy a job (from memory, but not from persistent storage if the job
  /// is owned by the job::Resource).
  virtual ~Stage() = default;

  /// A pointer to the stage's parent job.
  ///
  /// This is set when the stage is added to the definition
  const smtk::job::Definition* jobDefinition() const;

  /// The integer index of the stage (indicating its place in the job script's sequence of stages).
  ///
  /// This is set when the stage is added to a job definition and never modified.
  int index() const { return m_index; }

  /// A human-presentable name for the stage.
  ///
  /// The name should be a short word or two for display in tables.
  std::string name() const { return m_name; }

  /// A human-presentable description of the stage.
  ///
  /// The description may be a longer markdown narrative of the work the
  /// stage performs.
  std::string description() const { return m_description; }

  ///@{
  /// Set/get this log file output during the course of running this stage.
  ///
  /// The log file is assumed to contain human-readable data that user
  /// interfaces may wish to present.
  ///
  /// The log file is a path relative to the job's case directory.
  ///
  /// The log may be empty (in which case the stage has no log).
  bool setLog(const std::filesystem::path& logFile);
  const std::filesystem::path& log() const;
  ///@}

  ///@{
  /// Set/get/add/remove/clear a list of artifact paths (relative to the case directory
  /// and common to every job).
  ///
  /// These paths are not monitored by the job's queue but once the stage is
  /// complete, they should exist and not have their contents changed for the
  /// remainder of the job.
  const std::set<std::filesystem::path>& artifacts() const { return m_artifacts; }
  bool addArtifact(const std::filesystem::path& path);
  bool removeArtifact(const std::filesystem::path& path);
  bool hasArtifact(const std::filesystem::path& path) const;
  bool clearArtifacts();
  ///@}

protected:
  friend class Definition;
  Stage();
  Stage(
    Definition* jobDef,
    int idx,
    const std::string& name,
    const std::string& description = {},
    const std::filesystem::path& logPath = {});

  static std::shared_ptr<Stage> create(
    Definition* jobDef,
    int idx,
    const std::string& name,
    const std::string& description = {},
    const std::filesystem::path& logPath = {});

  Definition* m_definition{ nullptr };
  int m_index{ -1 };
  std::string m_name;
  std::string m_description;
  std::filesystem::path m_log;
  std::set<std::filesystem::path> m_artifacts;
};

} // namespace job
} // namespace smtk

#endif // smtk_job_Stage_h
