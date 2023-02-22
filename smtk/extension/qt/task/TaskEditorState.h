//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_TaskEditorState_h
#define smtk_extension_TaskEditorState_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/task/UIStateGenerator.h"

namespace smtk
{
namespace extension
{

class qtTaskEditor;

/** \brief A widget that holds a Qt scene graph. */
class SMTKQTEXT_EXPORT TaskEditorState : public smtk::task::UIStateGenerator
{
public:
  TaskEditorState(qtTaskEditor* taskEditor);
  ~TaskEditorState() = default;

  /// Provide any global state for the task-editor that should be saved across sessions.
  nlohmann::json globalState() const override;
  /// Provide UI state for the given task so it can be saved across sessions.
  nlohmann::json taskState(const std::shared_ptr<smtk::task::Task>& task) const override;

protected:
  qtTaskEditor* m_editor{ nullptr };
};

} // namespace extension
} // namespace smtk

#endif
