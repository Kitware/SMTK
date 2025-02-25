//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTaskPath_h
#define smtk_extension_qtTaskPath_h

#include "smtk/extension/qt/diagram/qtDiagram.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/task/State.h"

#include <QPointer>
#include <QScrollArea>

class QFrame;
class QHBoxLayout;
class QToolButton;

namespace smtk
{
namespace task
{
class Task;
}
namespace extension
{

class qtDiagramViewConfiguration;
class qtTaskEditor;

/**\brief An interactive widget that display the current path Task descendants.
  *
  * The leaf of the path corresponds to the task whose children should be
  * displayed in the diagram.
  */
class SMTKQTEXT_EXPORT qtTaskPath : public QScrollArea
{
  Q_OBJECT

public:
  smtkSuperclassMacro(QScrollArea);
  qtTaskPath(qtTaskEditor* editor, QWidget* parent = nullptr);
  // Append a task to the path
  QToolButton* addTask(smtk::task::Task* task);
  /// Return the last task in the path
  smtk::task::Task* lastTask() const;
  /// Update the path based on a task being made active
  ///
  /// The path will reflect the path of the newly active task.
  /// If the task has children, then the task will be included in the path
  /// else the path will end with its parent.
  void setActiveTask(smtk::task::Task* task);
  /// Clear the path
  void clearPath();
  /// Set the Path to be the root (aka top level)
  void gotoRoot();
  /// Signal handler for a task being made active via the path's tool buttons
  void updateActiveTask(bool makeActive);
  /// Return the task editor
  qtTaskEditor* editor() const { return m_editor; }
  /// Return true if the task can accept any worklets
  bool canAcceptWorklets(const smtk::task::Task* task) const;
  /// Returns true if the task should be included in the path
  ///
  /// This is based on whether the task has/can have children or if it has
  /// internal ports
  bool includeInPath(const smtk::task::Task* task) const;

protected:
  QFrame* m_main;
  QHBoxLayout* m_mainLayout;
  qtTaskEditor* m_editor;
  qtDiagramViewConfiguration* m_config; // needed to map Task state to color
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtDiagramLegend_h
