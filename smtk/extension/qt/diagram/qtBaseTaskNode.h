//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtBaseTaskNode_h
#define smtk_extension_qtBaseTaskNode_h

#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsItem>
#include <QGraphicsScene>

class QAbstractItemModel;
class QGraphicsTextItem;
class QItemSelection;
class QTimer;
class QTreeView;

namespace smtk
{
namespace task
{
enum class State;
class Task;
} // namespace task
namespace extension
{

class qtDiagramGenerator;
class qtTaskEditor;

/**\brief A Graphical Item that represents a task as a node in a scene.
  *
  */
class SMTKQTEXT_EXPORT qtBaseTaskNode : public qtBaseObjectNode
{
  Q_OBJECT
  Q_PROPERTY(OutlineStyle outlineStyle READ outlineStyle WRITE setOutlineStyle);

public:
  smtkSuperclassMacro(qtBaseObjectNode);
  smtkTypeMacro(smtk::extension::qtBaseTaskNode);

  /// Determine how the border of the node's visual representation should be rendered.
  enum class OutlineStyle : int
  {
    Normal, //!< Render an unobtrusive, subdued border around the node.
    Active  //!< Render a highlighted border around the node.
  };

  qtBaseTaskNode(
    qtDiagramGenerator* generator,
    smtk::task::Task* task,
    QGraphicsItem* parent = nullptr);
  ~qtBaseTaskNode() override;

  /// Return the task's UUID as the node's UUID.
  smtk::common::UUID nodeId() const override;

  /// Return the task as this node's persistent object.
  smtk::resource::PersistentObject* object() const override;

  /// Return the task this node represents.
  smtk::task::Task* task() const { return m_task; }

  /// Return the Task Editor for the node
  qtTaskEditor* editor() const { return static_cast<qtTaskEditor*>(m_generator); }

  /// Set/get how the node's boundary should be rendered.
  virtual void setOutlineStyle(OutlineStyle cs);
  OutlineStyle outlineStyle() const { return m_outlineStyle; }

  /// Return true if the task is currently active (i.e., being worked on by the user).
  bool isActive() const;

  ///Setup a context menu to be displayed by the node's underlying widget
  ///
  /// Returns true if setting up the menu was successful.
  virtual bool setupContextMenu(QMenu*) { return false; }

  /// Deals with state updates
  virtual void updateTaskState(smtk::task::State prev, smtk::task::State next, bool active);

  /// Deal with task updates (e.g., name or other configuration change).
  ///
  /// This method is invoked when an operation changes a task name or
  /// makes other changes that require a visual update to the GUI.
  SMTK_DEPRECATED_IN_24_01("Override or call dataUpdated() from qtBaseNode instead.")
  virtual void updateToMatchModifiedTask() { this->dataUpdated(); };

  /// Return the rounding radius used at the corners of the task node.
  ///
  /// Note that this does not mean the rounded corners are represented by circular arcs but in
  /// some cases are approximations based on quadratic curves.
  virtual double roundingRadius() { return 0.0; }

  /// Respond to changes in the task node.
  void dataUpdated() override;

protected:
  smtk::task::Task* m_task{ nullptr };
  OutlineStyle m_outlineStyle{ OutlineStyle::Normal };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtBaseTaskNode_h
