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

class qtTaskNode;
class qtTaskScene;
class qtTaskView;

/**\brief A widget that displays SMTK tasks available to users as a graph.
  *
  */
class SMTKQTEXT_EXPORT qtTaskEditor : public qtBaseView
{
  Q_OBJECT
  typedef smtk::extension::qtBaseView Superclass;

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
  qtTaskNode* findNode(smtk::task::Task* task) const;

  static std::shared_ptr<smtk::view::Configuration> defaultConfiguration();
  nlohmann::json uiStateForTask(const smtk::task::Task* task) const;

public Q_SLOTS:
  /// Display the \a project's tasks in this widget.
  virtual void displayProject(const std::shared_ptr<smtk::project::Project>& project);
  /// Display a \a taskManager's tasks in this widget (which need not belong to a project).
  virtual void displayTaskManager(smtk::task::Manager* taskManager);

protected Q_SLOTS:
  void onNodeGeometryChanged();

protected:
  class Internal;
  Internal* m_p;
};

} // namespace extension
} // namespace smtk
#endif // smtk_extension_qtTaskEditor_h
