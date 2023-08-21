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

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

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

class qtTaskScene;

/**\brief A Graphical Item that represents a task as a node in a scene.
  *
  */
class SMTKQTEXT_EXPORT qtBaseTaskNode
  : public QObject
  , public QGraphicsItem
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)

public:
  using Superclass = QGraphicsItem;

  /// Determine how the node is presented to users.
  enum class ContentStyle : int
  {
    Minimal, //!< Only the node's title-bar is shown.
    Summary, //!< The node's title bar and a "mini" viewer should be shown.
    Details  //!< The node's full state and any contained view should be shown.
  };

  /// Determine how the border of the node's visual representation should be rendered.
  enum class OutlineStyle : int
  {
    Normal, //!< Render an unobtrusive, subdued border around the node.
    Active  //!< Render a highlighted border around the node.
  };

  qtBaseTaskNode(qtTaskScene* scene, smtk::task::Task* task, QGraphicsItem* parent = nullptr);
  ~qtBaseTaskNode() override;

  /// Return the task this node represents.
  smtk::task::Task* task() const { return m_task; }

  /// Set/get how much data the node should render inside its boundary.
  virtual void setContentStyle(ContentStyle cs);
  ContentStyle contentStyle() const { return m_contentStyle; }

  /// Set/get how the node's boundary should be rendered.
  virtual void setOutlineStyle(OutlineStyle cs);
  OutlineStyle outlineStyle() const { return m_outlineStyle; }

  /// Return true if the task is currently active (i.e., being worked on by the user).
  bool isActive() const;

  /// Deals with state updates
  virtual void updateTaskState(smtk::task::State prev, smtk::task::State next) = 0;

Q_SIGNALS:
  void nodeResized();
  void nodeMovedImmediate();
  void nodeMoved(); // a rate-limited version of nodeMovedImmediate

protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

  /// Update the node bounds to fit its content.
  virtual int updateSize();

  ///\brief Adds the node to the scene
  ///
  /// NOTE: since qtBaseTaskNode class does not have any UI geometry associated
  /// with it, it will not automatically add itself to the Task Scene.  Derived
  /// classes should call this method after its UI geometry has been setup
  void addToScene();

  qtTaskScene* m_scene{ nullptr };
  smtk::task::Task* m_task{ nullptr };
  ContentStyle m_contentStyle{ ContentStyle::Minimal };
  OutlineStyle m_outlineStyle{ OutlineStyle::Normal };

  // Use timer to limit frequency of nodeMoved() signals to 10 hz.
  QTimer* m_moveSignalTimer{ nullptr };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtBaseTaskNode_h
