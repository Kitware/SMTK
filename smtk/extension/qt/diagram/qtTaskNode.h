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
#include "smtk/extension/qt/diagram/qtBaseTaskNode.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/task/Task.h"

#include <QGraphicsItem>
#include <QGraphicsScene>

class QAbstractItemModel;
class QGraphicsTextItem;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace task
{
class Task;
}
namespace extension
{

class qtDiagramGenerator;
class qtTaskStatusItem;
class qtTaskCompletionItem;
class qtTaskNameItem;

///\brief A implementation of a qtBaseTaskNode.
///
/// This implementation only depends qtGraphicItems and does not use Qt Widgets.
/// The result is both a cleaner looking design and also does not suffer from the
/// transformation problems that was noticed on Macs.
///
/// The new qtTaskNode is composed of 3 main graphics items:
///
/// * An item to display the Task's state as a color
/// * An item to display and manage the Task's name as well as the Task's activation status
/// * An item to control the Task's completed state
class SMTKQTEXT_EXPORT qtTaskNode : public qtBaseTaskNode
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)

public:
  smtkSuperclassMacro(qtBaseTaskNode);
  smtkTypeMacro(smtk::extension::qtTaskNode);

  qtTaskNode(
    qtDiagramGenerator* generator,
    smtk::task::Task* task,
    QGraphicsItem* parent = nullptr);
  ~qtTaskNode() override = default;

  /// Get the bounding box of the node, which includes the border width and the label.
  QRectF boundingRect() const override;

  /// Get the bounding box of the center content alone.
  virtual QRectF contentBoundingRect() const;

  /// Get the scale factor for the text height
  double contentHeightOffset() const { return m_contentHeightOffset; }

  /// Width of the side items
  double sideTotalWidth() const { return m_sideTotalWidth; }

  /// Deals with state updates
  void updateTaskState(smtk::task::State prev, smtk::task::State next, bool active) override;

  /// Return the rounding radius used at the corners of the task node.
  ///
  /// Note that this does not mean the rounded corners are represented by circular arcs but in
  /// some cases are approximations based on quadratic curves.
  double roundingRadius() override { return m_roundingRadius; }

  /// Handle renames, etc.
  void dataUpdated() override;

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  /// Update the node bounds to fit its content.
  int updateSize() override;

  qtTaskNameItem* m_textItem;
  qtTaskStatusItem* m_statusItem;
  qtTaskCompletionItem* m_completionItem;
  double m_contentHeightOffset = 30.0;
  double m_roundingRadius = 8.0;
  double m_sideTotalWidth = 50.0;
  double m_width;
  smtk::task::Task::Observers::Key m_taskObserver;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtTaskNode_h
