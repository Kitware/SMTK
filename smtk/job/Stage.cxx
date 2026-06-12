//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/Stage.h"

#include "smtk/job/Job.h"

namespace smtk
{
namespace job
{

class Job;
class Queue;

Stage::Stage()
  : m_definition{ nullptr }
{
}

Stage::Stage(
  Definition* jobDef,
  int idx,
  const std::string& name,
  const std::string& description,
  const std::filesystem::path& logPath)
  : m_definition(jobDef)
  , m_index(idx)
  , m_name(name)
  , m_description(description)
  , m_log(logPath)
{
}

std::shared_ptr<Stage> Stage::create(
  Definition* jobDef,
  int idx,
  const std::string& name,
  const std::string& description,
  const std::filesystem::path& logPath)
{
  return std::shared_ptr<Stage>(new Stage(jobDef, idx, name, description, logPath));
}

const Definition* Stage::jobDefinition() const
{
  return m_definition;
}

bool Stage::setLog(const std::filesystem::path& logFile)
{
  if (m_log == logFile)
  {
    return false;
  }
  m_log = logFile;
  return true;
}

const std::filesystem::path& Stage::log() const
{
  return m_log;
}

bool Stage::addArtifact(const std::filesystem::path& path)
{
  auto result = m_artifacts.insert(path);
  return result.second;
}

bool Stage::removeArtifact(const std::filesystem::path& path)
{
  auto result = m_artifacts.erase(path);
  return result > 0;
}

bool Stage::hasArtifact(const std::filesystem::path& path) const
{
  return m_artifacts.find(path) != m_artifacts.end();
}

bool Stage::clearArtifacts()
{
  bool empty = m_artifacts.empty();
  m_artifacts.clear();
  return !empty;
}

} // namespace job
} // namespace smtk
