//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTaskEditor_h
#define smtk_extension_qtTaskEditor_h

#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"

#include "smtk/project/Project.h"
#include "smtk/task/Manager.h"
#include "smtk/view/Configuration.h"

#include "smtk/common/Managers.h"
#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include "nlohmann/json.hpp"

#include <QWidget>

namespace smtk
{

namespace task
{
class Manager;
}

namespace extension
{

class qtBaseArc;
class qtBaseNode;
class qtBaseTaskNode;
class qtTaskPath;
class qtTaskPortArc;

/**\brief A widget that displays SMTK tasks available to users in a diagram.
  *
  */
class SMTKQTEXT_EXPORT qtTaskEditor : public qtDiagramGenerator
{
  Q_OBJECT

public:
  smtkTypenameMacro(smtk::extension::qtTaskEditor);
  smtkSuperclassMacro(smtk::extension::qtDiagramGenerator);

  qtTaskEditor(
    const smtk::view::Information& info,
    const smtk::view::Configuration::Component& config,
    qtDiagram* parent);
  ~qtTaskEditor() override;

  void updateSceneNodes(
    std::unordered_set<smtk::resource::PersistentObject*>& created,
    std::unordered_set<smtk::resource::PersistentObject*>& modified,
    std::unordered_set<smtk::resource::PersistentObject*>& expunged,
    const smtk::operation::Operation& operation,
    const smtk::operation::Operation::Result& result) override;

  void updateSceneArcs(
    std::unordered_set<smtk::resource::PersistentObject*>& created,
    std::unordered_set<smtk::resource::PersistentObject*>& modified,
    std::unordered_set<smtk::resource::PersistentObject*>& expunged,
    const smtk::operation::Operation& operation,
    const smtk::operation::Operation::Result& result) override;

  /// Return a qtDiagram configuration that will create a bare task editor.
  static std::shared_ptr<smtk::view::Configuration> defaultConfiguration();

  /// This is called when a worklet is dropped on the qtDiagram.
  bool addWorklet(const std::string& workletName, std::array<double, 2> location);

  /// Returns true if the event proposes to drop a worklet.
  bool acceptDropProposal(QDragEnterEvent* event) override;

  /// Update any drop preview to the location in the provided \a event.
  void moveDropPoint(QDragMoveEvent* event) override;

  /// Clean up any drop preview; the user has aborted the drag-and-drop.
  void abortDrop(QDragLeaveEvent* event) override;

  /// Return true if we can accept the drag-and-drop data at the finalized location.
  bool acceptDrop(QDropEvent* event) override;

  /// Return the qtBaseTaskNode that corresponds to a Task
  qtBaseTaskNode* findTaskNode(const smtk::task::Task* task) const;

  /// Return the task manager associated with the editor
  smtk::task::Manager* manager() const;

  /// Update the visibility of tasks, ports and related arcs.
  ///
  /// This is called when the active task is changed.
  void updateVisibility(const smtk::task::Task* prevTailTask, const smtk::task::Task* nextTailTask);

  ///@{
  ///\brief Controls if external task port nodes should be snapped
  /// with respected to their task nodes using the snap offset
  bool snapPortsToTask() const { return m_snapPortsToTask; }
  void setSnapPortsToTask(bool val) { m_snapPortsToTask = val; }
  ///@}
  ///@{
  ///\brief Get/set the offset to be used when snapping task ports
  /// nodes with respect to their task nodes.
  ///
  /// Note that the offset should be a positive value.
  double snapPortOffset() const { return m_snapPortOffset; }
  void setSnapPortOffset(double val) { m_snapPortOffset = val; }
  ///@}
  ///@{
  ///\brief Controls if external task port nodes should have a curve
  /// connecting it to their task nodes.
  bool drawPortsToTaskCurves() const { return m_drawPortsToTaskCurves; }
  void setDrawPortsToTaskCurves(bool val) { m_drawPortsToTaskCurves = val; }
  ///@}

  /// Return the task-editor's task breadcrumb.
  const qtTaskPath* taskPath() const { return m_taskPath; }
  qtTaskPath* taskPath() { return m_taskPath; }

protected:
  /// Used to create/destroy arcs incident to the node for \a object.
  ///
  /// As this method creates arcs, it will call updateArcStatus() on them
  /// to ensure they match the current diagram mode/state.
  template<bool RemoveUnusedArcs>
  bool updateArcs(
    smtk::resource::PersistentObject* object,
    QRectF& modBounds,
    ArcLegendEntries& legendInfo);

  /// Determine visibility and pickability based on
  /// the current interaction mode and the current task-path.
  void updateArcStatus(qtBaseArc* arc);

  class Internal;
  Internal* m_p;
  qtTaskPath* m_taskPath{ nullptr };
  bool m_snapPortsToTask{ false };
  double m_snapPortOffset{ 0.0 };
  bool m_drawPortsToTaskCurves{ true };
};

} // namespace extension
} // namespace smtk
#endif // smtk_extension_qtTaskEditor_h
