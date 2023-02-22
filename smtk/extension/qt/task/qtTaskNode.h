//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTaskNode_h
#define smtk_extension_qtTaskNode_h

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
class Task;
}
namespace extension
{

class qtTaskEditor;
class qtTaskScene;
class TaskNodeWidget;

/**\brief A widget that represents a task as a node in a scene.
  *
  */
class SMTKQTEXT_EXPORT qtTaskNode
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

  qtTaskNode(qtTaskScene* scene, smtk::task::Task* task, QGraphicsItem* parent = nullptr);
  ~qtTaskNode() override;

  /// Return the task this node represents.
  smtk::task::Task* task() const { return m_task; }

  /// Set/get how much data the node should render inside its boundary.
  void setContentStyle(ContentStyle cs);
  ContentStyle contentStyle() const { return m_contentStyle; }

  /// Set/get how the node's boundary should be rendered.
  void setOutlineStyle(OutlineStyle cs);
  OutlineStyle outlineStyle() const { return m_outlineStyle; }

  /// Get the bounding box of the node, which includes the border width and the label.
  QRectF boundingRect() const override;

Q_SIGNALS:
  void nodeResized();
  void nodeMovedImmediate();
  void nodeMoved(); // a rate-limited version of nodeMovedImmediate

protected:
  friend class TaskNodeWidget;
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  /// Update the node bounds to fit its content.
  int updateSize();

  qtTaskScene* m_scene{ nullptr };
  smtk::task::Task* m_task{ nullptr };
  TaskNodeWidget* m_container{ nullptr };
  ContentStyle m_contentStyle{ ContentStyle::Minimal };
  OutlineStyle m_outlineStyle{ OutlineStyle::Normal };

  // Use timer to limit frequency of nodeMoved() signals to 10 hz.
  QTimer* m_moveSignalTimer{ nullptr };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtTaskNode_h
