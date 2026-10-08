//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/Definition.h"

#include "smtk/job/Stage.h"

namespace smtk
{
namespace job
{

Definition::Definition() = default;

std::string Definition::name() const
{
  return m_name;
}

bool Definition::setName(const std::string& name)
{
  if (m_name == name || name.empty())
  {
    return false;
  }
  m_name = name;
  return true;
}

std::string Definition::description() const
{
  return m_description;
}

bool Definition::setDescription(const std::string& description)
{
  if (m_description == description)
  {
    return false;
  }
  m_description = description;
  return true;
}

std::filesystem::path Definition::script() const
{
  return m_script;
}

bool Definition::setScript(std::filesystem::path scriptPath)
{
  if (scriptPath.empty() || scriptPath == m_script)
  {
    return false;
  }
  m_script = scriptPath;
  return true;
}

bool Definition::setContainerImage(const std::string& imageURL)
{
  if (imageURL == m_containerImage)
  {
    return false;
  }
  m_containerImage = imageURL;
  return true;
}

bool Definition::setCaseDirectoryMountPoint(const std::string& mountPoint)
{
  if (mountPoint == m_caseDirectoryMountPoint)
  {
    return false;
  }
  m_caseDirectoryMountPoint = mountPoint;
  return true;
}

const Definition::LogParserConstructorMap& Definition::logParserConstructors() const
{
  return m_logParserConstructors;
}

bool Definition::clearLogParserConstructor(smtk::string::Token logPath)
{
  auto it = m_logParserConstructors.find(logPath);
  if (it == m_logParserConstructors.end())
  {
    return false;
  }
  m_logParserConstructors.erase(it);
  return true;
}

bool Definition::resetLogParserConstructors()
{
  if (m_logParserConstructors.empty())
  {
    return false;
  }
  m_logParserConstructors.clear();
  return true;
}

int Definition::appendStage(const std::shared_ptr<smtk::job::Stage>& stage)
{
  if (!stage || (stage->m_definition && stage->m_definition != this) || stage->name().empty())
  {
    return -1;
  }
  stage->m_definition = this;
  stage->m_index = static_cast<int>(m_stages.size());
  m_stages.push_back(stage);
  return true;
}

std::shared_ptr<Stage> Definition::appendStage(
  const std::string& name,
  const std::string& description)
{
  if (name.empty())
  {
    return std::shared_ptr<Stage>();
  }

  auto idx = static_cast<int>(m_stages.size());
  m_stages.push_back(Stage::create(this, idx, name, description));
  return m_stages[idx];
}

std::shared_ptr<Stage> Definition::appendStage(
  const std::string& name,
  const std::string& description,
  const std::filesystem::path& log)
{
  if (name.empty())
  {
    return std::shared_ptr<Stage>();
  }

  auto idx = static_cast<int>(m_stages.size());
  m_stages.push_back(Stage::create(this, idx, name, description, log));
  return m_stages[idx];
}

const std::vector<std::shared_ptr<smtk::job::Stage>>& Definition::stages() const
{
  return m_stages;
}

} // namespace job
} // namespace smtk
