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

  Q_PROPERTY(smtk::string::Token mode READ mode);

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

  /// Report the current user interaction mode.
  ///
  /// This will be one of: "pan", "select", "connect" but the list may be extended
  /// in the future.
  smtk::string::Token mode() const;

  /// In "connect" mode, the qtTaskView calls this method with the task node under
  /// the pointer each time it moves.
  void hoverConnectNode(qtBaseTaskNode* node);

  /// In "connect" mode, the qtTaskView calls this method with the task node under
  /// the pointer when its controller is clicked.
  void clickConnectNode(qtBaseTaskNode* node);

  /// In "connect" mode, abandon the connection by resetting the predecessor node.
  ///
  /// If no predecessor node was set, this will exit "connect" mode.
  void abandonConnection();

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
  /// Invoked when, in connection mode, the user selects a different arc type.
  void setConnectionType(int arcTypeItemIndex);

protected:
  /// Invoked by operation::Manager::groupObservers() when operations are added to the
  /// ArcCreator group.
  void updateArcTypes();

  class Internal;
  Internal* m_p;
};

} // namespace extension
} // namespace smtk
#endif // smtk_extension_qtTaskEditor_h
