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

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/project/Project.h"
#include "smtk/task/Manager.h"
#include "smtk/view/Configuration.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include "nlohmann/json.hpp"

#include <QWidget>

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

class qtBaseTaskNode;
class qtTaskScene;
class qtTaskView;

/**\brief A widget that displays SMTK tasks available to users as a graph.
  *
  */
class SMTKQTEXT_EXPORT qtTaskEditor : public qtBaseView
{
  Q_OBJECT
  typedef smtk::extension::qtBaseView Superclass;

  Q_PROPERTY(smtk::string::Token mode READ mode WRITE requestModeChange);

public:
  smtkTypenameMacro(qtTaskEditor);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtTaskEditor(const smtk::view::Information& info);
  ~qtTaskEditor() override;

  qtTaskScene* taskScene() const;
  qtTaskView* taskWidget() const;

  /// Provide the scene (and nodes within it) a way to look up nodes from a task.
  ///
  /// This will never create a node for the given task and may return a null pointer.
  qtBaseTaskNode* findNode(smtk::task::Task* task) const;

  static std::shared_ptr<smtk::view::Configuration> defaultConfiguration();
  nlohmann::json uiStateForTask(const smtk::task::Task* task) const;

  bool addWorklet(const std::string& workletName, std::array<double, 2> location);

  ///@{
  /// Set and get the current configuration of the Task Nodes' positions in JSON format.
  bool configure(const nlohmann::json& data);
  nlohmann::json configuration();
  ///@}

  /// Enable or disable task nodes.
  ///
  /// Modes such as qtConnectMode use this to modify user interactions.
  void enableTasks(bool shouldEnable);

  /// Enable or disable arc selection.
  ///
  /// Modes such as qtDisconnectMode use this to allow users to select arcs.
  void enableArcSelection(bool shouldEnable);

  /// Report the current user interaction mode.
  ///
  /// This will be one of: "pan", "select", "connect", or "disconnect"; but the list
  /// may be extended in the future.
  smtk::string::Token mode() const;

  /// Return the default mode for the editor.
  ///
  /// If a non-default mode wishes to allow users to "escape" from the mode
  /// (usually via the Escape key), they can request a change to this mode.
  /// The "connect" and "disconnect" modes currently use this method.
  smtk::string::Token defaultMode() const;

Q_SIGNALS:
  /// Emitted by modeChangeRequested when the mode is actually changed
  /// (and not when unchanged).
  void modeChanged(smtk::string::Token nextMode);

public Q_SLOTS:
  /// Display the \a project's tasks in this widget.
  virtual void displayProject(const std::shared_ptr<smtk::project::Project>& project);
  /// Display a \a taskManager's tasks in this widget (which need not belong to a project).
  virtual void displayTaskManager(smtk::task::Manager* taskManager);
  /// Request a change in the user-interaction mode.
  virtual void requestModeChange(smtk::string::Token mode);

protected Q_SLOTS:
  /// Invoked when a user clicks on a title-bar button to change modes.
  void modeChangeRequested(QAction* modeAction);
  /// Invoked when a user moves a node.
  void onNodeGeometryChanged();

protected:
  class Internal;
  Internal* m_p;
};

} // namespace extension
} // namespace smtk
#endif // smtk_extension_qtTaskEditor_h
