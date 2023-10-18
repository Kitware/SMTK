//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtDefaultTaskNode_h
#define smtk_extension_qtDefaultTaskNode_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/task/qtBaseTaskNode.h"

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

class qtTaskScene;
class DefaultTaskNodeWidget;

/**\brief A implementation of a qtBaseTaskNode.
  *
  */
class SMTKQTEXT_EXPORT qtDefaultTaskNode : public qtBaseTaskNode
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)

public:
  using Superclass = qtBaseTaskNode;

  qtDefaultTaskNode(qtTaskScene* scene, smtk::task::Task* task, QGraphicsItem* parent = nullptr);
  ~qtDefaultTaskNode() override = default;

  /// Set how much data the node should render inside its boundary.
  void setContentStyle(ContentStyle cs) override;

  /// Set how the node's boundary should be rendered.
  void setOutlineStyle(OutlineStyle cs) override;

  /// Get the bounding box of the node, which includes the border width and the label.
  QRectF boundingRect() const override;

  /// Deals with state updates
  void updateTaskState(smtk::task::State prev, smtk::task::State next, bool active) override;

protected:
  friend class DefaultTaskNodeWidget;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  /// Update the node bounds to fit its content.
  int updateSize() override;

  DefaultTaskNodeWidget* m_container{ nullptr };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtDefaultTaskNode_h
