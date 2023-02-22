//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/UIState.h"

#include "smtk/io/Logger.h"
#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{

void UIState::setData(std::shared_ptr<smtk::task::Task> task, const nlohmann::json& j)
{
  for (auto& entry : j.items())
  {
    smtk::string::Token classToken = entry.key();
    m_data[classToken][task->id()] = entry.value();
  }
}

nlohmann::json UIState::getData(smtk::string::Token classToken, smtk::task::Task* task) const
{
  auto classIter = m_data.find(classToken);
  if (classIter == m_data.end())
  {
    return nlohmann::json();
  }

  auto uiIter = classIter->second.find(task->id());
  if (uiIter == classIter->second.end())
  {
    return nlohmann::json::object();
  }

  return uiIter->second;
}

void UIState::updateJson(std::shared_ptr<smtk::task::Task> task, nlohmann::json& j) const
{
  auto jUI = nlohmann::json::object();
  if (j.contains("ui"))
  {
    jUI = j["ui"];
  }

  for (const auto& entry : m_generators)
  {
    jUI[entry.first] = entry.second->taskState(task);
  }

  j["ui"] = jUI;
}

void UIState::dump(std::ostream& os)
{
  os << '\n';
  auto classIter = m_data.begin();
  for (; classIter != m_data.end(); ++classIter)
  {
    os << classIter->first.data() << '\n';
    auto configIter = classIter->second.begin();
    for (; configIter != classIter->second.end(); ++configIter)
    {
      os << "  " << configIter->first.data() << ", " << configIter->second << '\n';
    }
  }
  os << std::endl;
}

} // namespace task
} // namespace smtk
