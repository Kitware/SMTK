//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtTaskNode.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramView.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtTaskEditor.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/operation/Manager.h"
#include "smtk/task/Active.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Task.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QPainter>
#include <QTextDocument>
#include <QTextOption>
#include <QTimer>

#include "task/ui_DefaultTaskNode.h"

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

/// This graphics item displays the status of the qtTaskNode
/// using color
class qtTaskStatusItem : public QGraphicsItem
{
public:
  qtTaskStatusItem(qtTaskNode* node)
    : QGraphicsItem(node)
    , m_node(node)
  {
  }

  QRectF boundingRect() const override
  {
    auto cbounds = m_node->contentBoundingRect();
    return QRectF(0.0, 0.0, m_node->sideTotalWidth(), cbounds.height());
  }

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
  {
    (void)option;
    (void)widget;
    auto& cfg = *m_node->scene()->configuration();
    auto cbounds = m_node->contentBoundingRect();

    QPainterPath
      p1; // Represents the outside edge of the item (excluding the contact with the task node item)
    p1.moveTo(m_node->sideTotalWidth(), cbounds.height());
    p1.lineTo(m_node->controlPointOffset(), cbounds.height());
    p1.quadTo(0.0, cbounds.height(), 0.0, cbounds.height() - m_node->controlPointOffset());
    p1.lineTo(0.0, m_node->controlPointOffset());
    p1.quadTo(0.0, 0.0, m_node->controlPointOffset(), 0.0);
    p1.lineTo(m_node->sideTotalWidth(), 0.0);
    // Paint the overall area first
    QPainterPath path;
    path.connectPath(p1);
    // Add the edge in contact with the task node item
    path.lineTo(m_node->sideTotalWidth(), cbounds.height());

    painter->fillPath(path, cfg.colorForState(m_node->task()->state()));

    // Now see if we need to display active or select status
    bool isActive = m_node->isActive();
    bool isSelected = m_node->isSelected();
    if (isActive || isSelected)
    {
      // We know we need to draw at least the outer indicator
      double borderThickness = cfg.nodeBorderThickness();
      auto outerOffset = borderThickness;
      QPainterPath p2; // An approximate offset curve of p1
      p2.moveTo(m_node->sideTotalWidth(), outerOffset);
      p2.lineTo(m_node->controlPointOffset() + outerOffset, outerOffset);
      p2.quadTo(outerOffset, outerOffset, outerOffset, m_node->controlPointOffset() + outerOffset);
      p2.lineTo(outerOffset, cbounds.height() - (m_node->controlPointOffset() + outerOffset));
      p2.quadTo(
        outerOffset,
        cbounds.height() - outerOffset,
        m_node->controlPointOffset() + outerOffset,
        cbounds.height() - outerOffset);
      p2.lineTo(m_node->sideTotalWidth(), cbounds.height() - outerOffset);

      // Create a region that is defined by the outer edge (p1) and its offset (p2)
      QPainterPath outerPath;
      outerPath.connectPath(p1);
      outerPath.connectPath(p2);
      // Add the path of the curve indicating the where the outer indicator region would meet the indicator region
      outerPath.lineTo(m_node->sideTotalWidth(), cbounds.height());

      // if the item is not selected then show the outer indicator as active
      if (!isSelected)
      {
        painter->fillPath(outerPath, cfg.activeTaskColor());
        return;
      }

      painter->fillPath(outerPath, cfg.selectionColor());

      if (!isActive)
      {
        return;
      }

      auto innerOffset = borderThickness * 2.0;
      // Create a region defined by p2 and an approximate offset of p2
      QPainterPath innerPath;
      // This part is an approximate offset of p2
      innerPath.moveTo(m_node->sideTotalWidth(), cbounds.height() - innerOffset);
      innerPath.lineTo(m_node->controlPointOffset() + innerOffset, cbounds.height() - innerOffset);
      innerPath.quadTo(
        innerOffset,
        cbounds.height() - innerOffset,
        innerOffset,
        cbounds.height() - (m_node->controlPointOffset() + innerOffset));
      innerPath.lineTo(innerOffset, m_node->controlPointOffset() + innerOffset);
      innerPath.quadTo(
        innerOffset, innerOffset, m_node->controlPointOffset() + innerOffset, innerOffset);
      innerPath.lineTo(m_node->sideTotalWidth(), innerOffset);
      innerPath.connectPath(p2);
      innerPath.lineTo(m_node->sideTotalWidth(), cbounds.height() - innerOffset);

      painter->fillPath(innerPath, cfg.activeTaskColor());
    }
  }
  qtTaskNode* m_node;
};

/// This graphics item allows the user to mark the task as completed
/// using a check-box approach - when inside this item the cursor will be a normal arrow
///
/// Note that the requested size should be greater than 2!
class qtTaskCompletionCheckBoxItem : public QGraphicsItem
{
public:
  qtTaskCompletionCheckBoxItem(double size, qtTaskNode* node, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_node(node)
    , m_checked(false)
    , m_size(size)
  {
    this->setCursor(Qt::ArrowCursor);
  }

  QRectF boundingRect() const override
  {
    auto myBounds = QRectF(0, 0, m_size, m_size);
    auto pBounds = this->parentItem()->boundingRect();
    auto offset = pBounds.center() - myBounds.center();

    return QRectF(offset.x(), offset.y(), m_size, m_size);
  }

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
  {
    (void)option;
    (void)widget;
    auto b = this->boundingRect();
    auto& cfg = *m_node->scene()->configuration();
    QPen pen;
    pen.setBrush(cfg.textColor());
    painter->setPen(pen);
    painter->drawRect(b);

    // Fill in the check box if the task is completed
    if (m_node->task()->state() == smtk::task::State::Completed)
    {
      painter->fillRect(b.adjusted(2, 2, -2, -2), cfg.textColor());
    }
  }

  void mousePressEvent(QGraphicsSceneMouseEvent* event) override
  {
    (void)event;
    // Try to changed the task's completeness state and if successful, update the item
    if (m_node->task()->markCompleted(m_node->task()->state() != smtk::task::State::Completed))
    {
      this->update();
    }
  }

private:
  qtTaskNode* m_node;
  bool m_checked;
  double m_size;
};

/// This item deals with completing the task
class qtTaskCompletionItem : public QGraphicsItem
{
public:
  qtTaskCompletionItem(qtTaskNode* node)
    : QGraphicsItem(node)
    , m_node(node)
  {
    auto cbounds = m_node->contentBoundingRect();
    double height = cbounds.height();
    m_checkbox = new qtTaskCompletionCheckBoxItem(height * 0.3, node, this);
  }

  QRectF boundingRect() const override
  {
    auto cbounds = m_node->contentBoundingRect();
    double height = cbounds.height();
    return QRectF(
      cbounds.width() + m_node->sideTotalWidth(), 0.0, m_node->sideTotalWidth(), height);
  }

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
  {
    (void)option;
    (void)widget;
    auto cbounds = m_node->contentBoundingRect();
    double h = cbounds.height();
    double w = cbounds.width() + m_node->sideTotalWidth();
    double delta = m_node->controlPointOffset();
    auto& cfg = *m_node->scene()->configuration();

    QPainterPath
      p1; // Represents the outside edge of the item (excluding the contact with the task node item)
    p1.moveTo(w, h);
    p1.lineTo(w + m_node->sideTotalWidth() - delta, h);
    p1.quadTo(w + m_node->sideTotalWidth(), h, w + m_node->sideTotalWidth(), h - delta);
    p1.lineTo(w + m_node->sideTotalWidth(), delta);
    p1.quadTo(w + m_node->sideTotalWidth(), 0.0, w + m_node->sideTotalWidth() - delta, 0.0);
    p1.lineTo(w, 0.0);

    // Paint the overall area first
    QPainterPath path;
    path.connectPath(p1);
    // Add the edge in contact with the task node item
    path.lineTo(w, h);
    painter->fillPath(path, cfg.baseNodeColor());

    // Now see if we need to display active or select status
    bool isActive = m_node->isActive();
    bool isSelected = m_node->isSelected();
    if (isActive || isSelected)
    {
      // We know we need to draw at least the outer indicator
      double borderThickness = cfg.nodeBorderThickness();
      auto outerOffset = borderThickness;

      QPainterPath p2; // An approximate offset curve of p1
      p2.moveTo(w, outerOffset);
      p2.lineTo(w + m_node->sideTotalWidth() - (delta + outerOffset), outerOffset);
      p2.quadTo(
        w + m_node->sideTotalWidth() - outerOffset,
        outerOffset,
        w + m_node->sideTotalWidth() - outerOffset,
        delta + outerOffset);
      p2.lineTo(w + m_node->sideTotalWidth() - outerOffset, h - (delta + outerOffset));
      p2.quadTo(
        w + m_node->sideTotalWidth() - outerOffset,
        h - outerOffset,
        w + m_node->sideTotalWidth() - (delta + outerOffset),
        h - outerOffset);
      p2.lineTo(w, h - outerOffset);

      // Create a region that is defined by the outer edge (p1) and its offset (p2)
      QPainterPath outerPath;
      outerPath.connectPath(p1);
      outerPath.connectPath(p2);
      // Close the path
      outerPath.lineTo(w, h);

      // if the item is not selected then show the outer indicator as active
      if (!isSelected)
      {
        painter->fillPath(outerPath, cfg.activeTaskColor());
        return;
      }

      painter->fillPath(outerPath, cfg.selectionColor());

      if (!isActive)
      {
        return;
      }

      auto innerOffset = borderThickness * 2.0;
      // Create a region defined by p2 and an approximate offset of p2
      QPainterPath innerPath;
      // This part is an approximate offset of p2
      innerPath.moveTo(w, h - innerOffset);
      innerPath.lineTo(w + m_node->sideTotalWidth() - (delta + innerOffset), h - innerOffset);
      innerPath.quadTo(
        w + m_node->sideTotalWidth() - innerOffset,
        h - innerOffset,
        w + m_node->sideTotalWidth() - innerOffset,
        h - (delta + innerOffset));
      innerPath.lineTo(w + m_node->sideTotalWidth() - innerOffset, delta + innerOffset);
      innerPath.quadTo(
        w + m_node->sideTotalWidth() - innerOffset,
        innerOffset,
        w + m_node->sideTotalWidth() - (delta + innerOffset),
        innerOffset);
      innerPath.lineTo(w, innerOffset);
      innerPath.connectPath(p2);
      innerPath.lineTo(w, h - innerOffset);
      painter->fillPath(innerPath, cfg.activeTaskColor());
    }
  }
  qtTaskNode* m_node;
  qtTaskCompletionCheckBoxItem* m_checkbox;
};

/// This item is used to display and change the name of a Task as well as
/// activating the Task.  When editing the task name, the i-beam curse is
/// displayed.  When toggling activation, the pointing hand cursor is used.
class qtTaskNameItem : public QGraphicsTextItem
{
public:
  qtTaskNameItem(qtTaskNode* node)
    : QGraphicsTextItem(node)
    , m_node(node)
  {
    this->setFlags(ItemIsSelectable | ItemIsFocusable);
    this->setTextInteractionFlags(Qt::NoTextInteraction);
    auto f = this->font();
    std::cerr << "Default Font Size:" << f.pointSize();
    f.setPointSize(18);
    this->setFont(f);
    qtDiagramViewConfiguration& cfg(*m_node->scene()->configuration());
    auto* task = m_node->task();
    if (task)
    {
      this->setHtml(task->name().c_str());
    }
    // See if we need to adjust the text width based on the configuration
    double textWidth = this->boundingRect().width();
    double minTextWidth = cfg.nodeWidth() - (2 * m_node->sideTotalWidth());
    this->document()->setDefaultTextOption(QTextOption(Qt::AlignHCenter));
    this->setTextWidth((minTextWidth > textWidth) ? minTextWidth : textWidth);
    // Since all double click events are preceded by a single click event, the item uses a timer
    // to remove all single clicks that were part of a double click.  When a single click event is
    // received, we start a timer and if no double click event occurs before the timeout, we process
    // the single click event as a task activation request.
    m_timer = new QTimer(this);
    m_timer->setInterval(200);
    connect(m_timer, &QTimer::timeout, this, &qtTaskNameItem::activateTask);
    this->setCursor(Qt::PointingHandCursor);
  }

  void setTextInteraction(bool on, bool selectAll = false)
  {
    if (on && this->textInteractionFlags() == Qt::NoTextInteraction)
    {
      // switch on editor mode:
      this->setCursor(Qt::IBeamCursor);
      this->setTextInteractionFlags(Qt::TextEditorInteraction);
      // manually do what a mouse click would do else:
      this->setFocus(Qt::MouseFocusReason); // this gives the item keyboard focus
      this->setSelected(
        true);       // this ensures that itemChange() gets called when we click out of the item
      if (selectAll) // option to select the whole text (e.g. after creation of the TextItem)
      {
        QTextCursor c = this->textCursor();
        c.select(QTextCursor::Document);
        this->setTextCursor(c);
      }
    }
    else if (!on && this->textInteractionFlags() == Qt::TextEditorInteraction)
    {
      // turn off editor mode:
      this->setTextInteractionFlags(Qt::NoTextInteraction);
      // deselect text (else it keeps gray shade):
      QTextCursor c = this->textCursor();
      c.clearSelection();
      this->setTextCursor(c);
      this->clearFocus();
      this->setCursor(Qt::PointingHandCursor);
    }
  }

  void activateTask()
  {
    m_timer->stop();
    auto* task = m_node->task();
    auto* taskManager = task->manager();
    if (taskManager)
    {
      auto* previouslyActive = taskManager->active().task();
      if (previouslyActive != task)
      {
        taskManager->active().switchTo(task);
        m_node->update();
      }
      else
      {
        // Deactivate this task without making any other active.
        taskManager->active().switchTo(nullptr);
        m_node->update();
      }
    }
  }

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
  {
    auto& cfg = *m_node->scene()->configuration();
    this->setDefaultTextColor(cfg.textColor());
    QGraphicsTextItem::paint(painter, option, widget);
  }

  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* evt) override
  {
    // Stop the timer if it was started by a single clink
    m_timer->stop();
    if (this->textInteractionFlags() == Qt::TextEditorInteraction)
    {
      // if editor mode is already on: pass double click events on to the editor:
      QGraphicsTextItem::mouseDoubleClickEvent(evt);
      return;
    }

    // if editor mode is off:
    // 1. turn editor mode on and set selected and focused:
    this->setTextInteraction(true);

    // 2. send a single click to this QGraphicsTextItem (this will set the cursor to the mouse position):
    // create a new mouse event with the same parameters as evt
    QGraphicsSceneMouseEvent* click = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
    click->setButton(evt->button());
    click->setPos(evt->pos());
    QGraphicsTextItem::mousePressEvent(click);
    delete click; // don't forget to delete the event
  }

  void mousePressEvent(QGraphicsSceneMouseEvent* evt) override
  {
    if (this->textInteractionFlags() == Qt::TextEditorInteraction)
    {
      // if editor mode is already on: pass click events on to the editor:
      QGraphicsTextItem::mouseDoubleClickEvent(evt);
      return;
    }
    m_timer->start();
  }

  QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override
  {
    if (
      change == QGraphicsItem::ItemSelectedChange &&
      textInteractionFlags() != Qt::NoTextInteraction && !value.toBool())
    {
      // item received SelectedChange event AND is in editor mode AND is about to be deselected:
      this->setTextInteraction(false); // leave editor mode
      // Set the task's name if it has been changed
      auto* task = m_node->task();
      std::string newName = this->toPlainText().toStdString();
      if (task->name() != newName)
      {
        // Prep and run the operation (if possible)
        auto opMgr = task->manager()->managers()->get<smtk::operation::Manager::Ptr>();
        auto op = opMgr->create("smtk::task::RenameTask");
        if (!op)
        {
          return false;
        }
        if (!op->parameters()->associate(task->shared_from_this()))
        {
          return false;
        }
        if (!op->parameters()->findAs<smtk::attribute::StringItem>("name")->setValue(newName))
        {
          return false;
        }
        if (!op->ableToOperate())
        {
          return false;
        }
        opMgr->launchers()(op);
      }
    }
    return QGraphicsTextItem::itemChange(change, value);
  }
  qtTaskNode* m_node;
  QTimer* m_timer;
};

qtTaskNode::qtTaskNode(qtDiagramGenerator* generator, smtk::task::Task* task, QGraphicsItem* parent)
  : Superclass(generator, task, parent)
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  m_statusItem = new qtTaskStatusItem(this);
  m_textItem = new qtTaskNameItem(this);
  m_textItem->setPos(m_sideTotalWidth, 0.5 * m_contentHeightOffset);

  m_completionItem = new qtTaskCompletionItem(this);
  this->addToScene();

  // === Task-specific constructor ===
  this->setZValue(cfg.nodeLayer());
  this->setCursor(Qt::SizeAllCursor);
}

QRectF qtTaskNode::boundingRect() const
{
  auto cbounds = this->contentBoundingRect();
  QRectF b(0.0, 0.0, cbounds.width() + 2.0 * m_sideTotalWidth, cbounds.height());
  return b;
}

QRectF qtTaskNode::contentBoundingRect() const
{
  auto b = m_textItem->boundingRect();
  return QRectF(m_sideTotalWidth, 0.0, b.width(), b.height() + m_contentHeightOffset);
}

void qtTaskNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  (void)option;
  (void)widget;
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  QPainterPath path;
  // Make sure the whole node is redrawn to avoid artifacts:
  auto cbounds = this->contentBoundingRect();
  //Adjust the widget so that it can overlap its sides slightly to deal
  // with rendering issues.
  path.addRect(cbounds.adjusted(-1.0, 0.0, 1.0, 0.0));

  QPen pen;

  painter->fillPath(path, cfg.baseNodeColor());
  painter->setPen(pen);

  if (this->isActive())
  {
    QPen apen;
    double activeThickness = cfg.nodeBorderThickness();
    auto activeOffset = activeThickness * (this->isSelected() ? 1.5 : 0.5);
    apen.setWidth(activeThickness);
    QPainterPath activePath;
    // Note that we are making the lines a little longer to deal with a rendering issue
    // where sometimes there is a small gap with the sides
    activePath.moveTo(m_sideTotalWidth - 2, activeOffset);
    activePath.lineTo(m_sideTotalWidth + cbounds.width() + 2, activeOffset);
    activePath.moveTo(m_sideTotalWidth - 2, cbounds.height() - activeOffset);
    activePath.lineTo(m_sideTotalWidth + cbounds.width() + 2, cbounds.height() - activeOffset);

    apen.setBrush(cfg.activeTaskColor());
    painter->setPen(apen);
    painter->drawPath(activePath);
  }

  if (this->isSelected())
  {
    QPen spen;
    double selThickness = cfg.nodeBorderThickness();
    auto selOffset = selThickness * 0.5;
    spen.setWidth(selThickness);
    QPainterPath selPath;
    selPath.moveTo(m_sideTotalWidth - 2, selOffset);
    selPath.lineTo(m_sideTotalWidth + cbounds.width() + 2, selOffset);
    selPath.moveTo(m_sideTotalWidth - 2, cbounds.height() - selOffset);
    selPath.lineTo(m_sideTotalWidth + cbounds.width() + 2, cbounds.height() - selOffset);

    spen.setBrush(cfg.selectionColor());
    painter->setPen(spen);
    painter->drawPath(selPath);
  }
}

int qtTaskNode::updateSize()
{
  this->prepareGeometryChange();

  Q_EMIT this->nodeResized();

  return 1;
}

void qtTaskNode::updateTaskState(smtk::task::State prev, smtk::task::State next, bool active)
{
  // Nothing needs to be done since the item's
  // children take care of this when painting themselves
  (void)prev;
  (void)next;
  (void)active;
  this->update();
}

} // namespace extension
} // namespace smtk
