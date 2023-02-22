//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_UIState_h
#define smtk_task_UIState_h

#include "smtk/CoreExports.h"

#include "smtk/string/Token.h"
#include "smtk/task/UIStateGenerator.h"

#include "nlohmann/json.hpp"

#include <ostream>
#include <string>
#include <unordered_map>

namespace smtk
{
namespace task
{
class Task;

/**\brief Stores UI state data for task UI classes. */
class SMTKCORE_EXPORT UIState
{
public:
  UIState() = default;
  ~UIState() = default;

  /** \brief Stores "ui" object for specified task. */
  void setData(std::shared_ptr<smtk::task::Task> task, const nlohmann::json& j);

  /** \brief Returns "ui" data for given class and task. */
  nlohmann::json getData(smtk::string::Token classToken, smtk::task::Task* task) const;

  /** \brief Stores generator for given class name. */
  void setGenerator(const std::string& className, std::shared_ptr<UIStateGenerator> generator)
  {
    m_generators[className] = generator;
  }

  /** \brief Updates "ui" object to include data from all generators. */
  void updateJson(std::shared_ptr<smtk::task::Task> task, nlohmann::json& j) const;

  /** \brief Writes contents of "ui" objects stored for each task and class */
  void dump(std::ostream& os);

protected:
  /** \brief Nested map of <<class>, <task_id, json>> for deserialized UI state data. */
  std::unordered_map<smtk::string::Token, std::unordered_map<smtk::string::Token, nlohmann::json>>
    m_data;

  /** \brief Map of <classname, generator> for serializing UI state data. */
  std::unordered_map<std::string, std::shared_ptr<UIStateGenerator>> m_generators;
};

} // namespace task
} // namespace smtk

#endif
