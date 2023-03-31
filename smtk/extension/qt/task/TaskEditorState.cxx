//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/TaskEditorState.h"

#include "smtk/extension/qt/task/qtTaskEditor.h"
#include "smtk/task/Task.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace extension
{

TaskEditorState::TaskEditorState(qtTaskEditor* taskEditor)
  : m_editor(taskEditor)
{
}

nlohmann::json TaskEditorState::globalState() const
{
  return nlohmann::json();
}

nlohmann::json TaskEditorState::taskState(const std::shared_ptr<smtk::task::Task>& task) const
{
  (void)task;
  return m_editor->uiStateForTask(task.get());
}

} // namespace extension
} // namespace smtk
