//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtOperationTypeView.h"

#include "smtk/extension/qt/qtOperationAction.h"
#include "smtk/extension/qt/qtOperationTypeModel.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"

#include "smtk/io/Logger.h"

#include <QAbstractItemModel>
#include <QEvent>
#include <QScrollBar>
#include <QVBoxLayout>

qtOperationTypeView::qtOperationTypeView(QWidget* parent)
  : Superclass(parent)
{
  m_palette = new QWidget(this);
  m_palette->setObjectName("OperationPalette");
  m_flow = new qtToolPaletteLayout(m_palette, 5, 5, 5);
  m_palette->setLayout(m_flow);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  this->setWidgetResizable(true);
  this->setWidget(m_palette);
}

qtOperationTypeView::~qtOperationTypeView() = default;

QAbstractItemModel* qtOperationTypeView::model() const
{
  return m_model;
}

void qtOperationTypeView::setModel(QAbstractItemModel* model)
{
  if (m_model == model)
  {
    return;
  }

  this->disconnect(m_model);
  this->operationsResetting(); // Changing models is the same as resetting in many ways.

  m_model = model;

  if (m_model)
  {
    // clang-format off
    // Observe model for changes.
    // The following comments are all the signals we might observe, interleaved with
    // connections for things we do observe.
    //
    // QObject::connect(model, &QAbstractItemModel::columnsAboutToBeInserted(const QModelIndex &parent, int first, int last)
    // QObject::connect(model, &QAbstractItemModel::columnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationColumn)
    // QObject::connect(model, &QAbstractItemModel::columnsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
    // QObject::connect(model, &QAbstractItemModel::columnsInserted(const QModelIndex &parent, int first, int last)
    // QObject::connect(model, &QAbstractItemModel::columnsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int column)
    // QObject::connect(model, &QAbstractItemModel::columnsRemoved(const QModelIndex &parent, int first, int last)
    // QObject::connect(model, &QAbstractItemModel::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>())
    QObject::connect(model, &QAbstractItemModel::dataChanged, this, &qtOperationTypeView::operationsUpdated);
    // QObject::connect(model, &QAbstractItemModel::headerDataChanged(Qt::Orientation orientation, int first, int last)
    // QObject::connect(model, &QAbstractItemModel::layoutAboutToBeChanged(const QList<QPersistentModelIndex> &parents = QList<QPersistentModelIndex>(), QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint)
    QObject::connect(model, &QAbstractItemModel::layoutAboutToBeChanged, this, &qtOperationTypeView::operationsLayoutChanging);
    // QObject::connect(model, &QAbstractItemModel::layoutChanged(const QList<QPersistentModelIndex> &parents = QList<QPersistentModelIndex>(), QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint)
    QObject::connect(model, &QAbstractItemModel::layoutChanged, this, &qtOperationTypeView::operationsLayoutChanged);
    // QObject::connect(model, &QAbstractItemModel::modelAboutToBeReset()
    QObject::connect(model, &QAbstractItemModel::modelAboutToBeReset, this, &qtOperationTypeView::operationsResetting);
    // QObject::connect(model, &QAbstractItemModel::modelReset()
    // QObject::connect(model, &QAbstractItemModel::rowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
    // QObject::connect(model, &QAbstractItemModel::rowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
    QObject::connect(model, &QAbstractItemModel::rowsAboutToBeMoved, this, &qtOperationTypeView::operationsReordered);
    // QObject::connect(model, &QAbstractItemModel::rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
    QObject::connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &qtOperationTypeView::operationsRemoved);
    // QObject::connect(model, &QAbstractItemModel::rowsInserted(const QModelIndex &parent, int first, int last)
    QObject::connect(model, &QAbstractItemModel::rowsInserted, this, &qtOperationTypeView::operationsAdded);
    // QObject::connect(model, &QAbstractItemModel::rowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row)
    // QObject::connect(model, &QAbstractItemModel::rowsRemoved(const QModelIndex &parent, int first, int last)

    // Now populate ourselves with the current model contents.
    this->operationsAdded(QModelIndex(), 0, model->rowCount(QModelIndex()));
    // clang-format on
  }
}

QModelIndex qtOperationTypeView::indexAt(const QPoint& point) const
{
  int row = m_flow->indexAt(point);
  QModelIndex thing = row >= 0 ? m_model->index(row, 0) : QModelIndex();
  return thing;
}

void qtOperationTypeView::scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint)
{
  if (!index.isValid())
  {
    return;
  }

  QRect rect = visualRect(index);
  if (!rect.isValid())
  {
    return;
  }
  auto* scrollBar = this->verticalScrollBar();
  if (!scrollBar)
  {
    return;
  }

  // Note: The switch statements below use the following information:
  //
  // + m_palette->geometry().height() is the total height of the m_flow layout
  //       (if the screen was large enough to display all the buttons without scrolling).
  // + scrollBar->maximum() - scrollbar->minimum() is the portion of the above that is
  //       not shown at any time because the scroll area is not as large as the layout needs.
  // + this->geometry().height() is the portion of m_palette is that *is* shown at any time.
  // + rect.y() is the top of the operation's button widget
  // + rect.y() + rect.height() is the bottom of the operation's button widget.
  //
  switch (hint)
  {
    case QAbstractItemView::EnsureVisible:
      // Check if already visible. If so, do not move.
      if (rect.height() > this->geometry().height())
      {
        // First, if the item is taller than the entire scroll area, center the item.
        scrollBar->setValue(rect.y() + rect.height() / 2 - this->geometry().height() / 2);
      }
      else if (rect.y() < scrollBar->value())
      {
        // If the item is above the top of the scroll area, scroll up just enough.
        scrollBar->setValue(rect.y());
      }
      else if (rect.y() + rect.height() > this->geometry().height() + scrollBar->value())
      {
        // If the item is below the bottom of the scroll area, scroll down just enough.
        scrollBar->setValue(rect.y() + rect.height() - this->geometry().height());
      }
      break;
    case QAbstractItemView::PositionAtTop:
      scrollBar->setValue(rect.y());
      break;
    case QAbstractItemView::PositionAtBottom:
      scrollBar->setValue(rect.y() + rect.height() - this->geometry().height());
      break;
    case QAbstractItemView::PositionAtCenter:
      scrollBar->setValue(rect.y() + rect.height() / 2 - this->geometry().height() / 2);
      break;
  }
}

QRect qtOperationTypeView::visualRect(const QModelIndex& index) const
{
  QRect placement = m_flow->itemAt(index.row())->widget()->geometry();
  return placement;
}

void qtOperationTypeView::operationsAdded(const QModelIndex& parent, int first, int last)
{
  (void)parent;
  assert(!parent.isValid()); // Parent must always be the root node.

  auto opModel = m_model;
  if (!opModel)
  {
    return;
  }

  // Add toolbar button for each new model-row.
  for (int ii = first; ii <= last; ++ii)
  {
    QModelIndex operationActionIndex =
      opModel->index(ii, static_cast<int>(qtOperationTypeModel::Column::WidgetAction));
    auto* opAction = qvariant_cast<qtOperationAction*>(operationActionIndex.data());
    if (opAction)
    {
      auto* button = opAction->requestWidget(m_palette);
      // NB: For some reason, the \a button is not visible by default.
      // However, this is what we desire, since the layout will show()
      // it **if** it is placed properly. Blindly calling
      // button->setVisible(true) will cause unused buttons to appear
      // beneath active buttons in the top-left corner of the widget.
      m_flow->insertItem(ii, new QWidgetItem(button));
    }
  }
  m_flow->invalidate();
}

void qtOperationTypeView::operationsReordered(
  const QModelIndex& sourceParent,
  int sourceStart,
  int sourceEnd,
  const QModelIndex& destinationParent,
  int destinationRow)
{
  (void)sourceParent;
  (void)destinationParent;
  assert(!sourceParent.isValid());
  assert(!destinationParent.isValid());
  smtkWarningMacro(
    smtk::io::Logger::instance(),
    "Unhandled reorder " << sourceStart << " " << sourceEnd << " → " << destinationRow);
}

void qtOperationTypeView::operationsRemoved(const QModelIndex& parent, int first, int last)
{
  (void)parent;
  assert(!parent.isValid()); // Parent must always be the root node.

  auto opModel = m_model;
  if (!opModel)
  {
    return;
  }

  // Remove toolbar button for each disappearing row.
  for (int ii = first; ii <= last; ++ii)
  {
    QLayoutItem* layoutItem = m_flow->takeAt(first);
    if (layoutItem)
    {
      QModelIndex operationActionIndex =
        opModel->index(ii, static_cast<int>(qtOperationTypeModel::Column::WidgetAction));
      auto* widgetAction = dynamic_cast<qtOperationAction*>(
        qvariant_cast<QObject*>(opModel->data(operationActionIndex)));
      if (widgetAction)
      {
        widgetAction->releaseWidget(layoutItem->widget());
      }
    }
  }
  m_flow->invalidate();
}

void qtOperationTypeView::operationsUpdated(
  const QModelIndex& topLeft,
  const QModelIndex& bottomRight,
  const QVector<int>& roles)
{
  (void)roles;
  // NB: This assumes that none of the proxy models reorder columns.
  //     The view does not allow column reordering.
  if (
    topLeft.column() > static_cast<int>(qtOperationTypeModel::Column::WidgetAction) ||
    bottomRight.column() < static_cast<int>(qtOperationTypeModel::Column::WidgetAction))
  {
    // If no widget actions were modified, we don't need to update their state.
    return;
  }

  // Enabling/disabling actions does not cause their
  // associated buttons to update. Fix that here when
  // we are notified that the column was modified.
  for (int ii = topLeft.row(); ii <= bottomRight.row(); ++ii)
  {
    qtOperationAction* op = nullptr;
    for (auto* action : m_flow->itemAt(ii)->widget()->actions())
    {
      if ((op = dynamic_cast<qtOperationAction*>(action)))
      {
        if (op->isEnabled() != m_flow->itemAt(ii)->widget()->isEnabled())
        {
          m_flow->itemAt(ii)->widget()->setEnabled(op->isEnabled());
        }
        break;
      }
    }
  }
}

void qtOperationTypeView::operationsLayoutChanging(
  const QList<QPersistentModelIndex>& parents,
  QAbstractItemModel::LayoutChangeHint hint)
{
  (void)parents;
  (void)hint;
  this->operationsResetting();
}

void qtOperationTypeView::operationsLayoutChanged(
  const QList<QPersistentModelIndex>& parents,
  QAbstractItemModel::LayoutChangeHint hint)
{
  (void)parents;
  (void)hint;
  this->operationsAdded(QModelIndex(), 0, m_model->rowCount(QModelIndex()));
}

void qtOperationTypeView::operationsResetting()
{
  // Remove all buttons
  auto numRows = m_model ? m_model->rowCount(QModelIndex()) : 0;
  if (numRows > 0)
  {
    this->operationsRemoved(QModelIndex(), 0, numRows);
  }
}

#if 0
int qtOperationTypeView::horizontalOffset() const
{
  // TODO
  return 0;
}

int qtOperationTypeView::verticalOffset() const
{
  // TODO
  return 0;
}

bool qtOperationTypeView::isIndexHidden(const QModelIndex& index) const
{
  (void) index;
  // TODO: Hide inapplicable operations...
  return false;
}

QModelIndex qtOperationTypeView::moveCursor(
  QAbstractItemView::CursorAction cursorAction,
  Qt::KeyboardModifiers modifiers)
{
  QModelIndex index = this->currentIndex();
  int row = index.row();
  int col = index.column();
  if (index.isValid())
  {
    switch (cursorAction)
    {
    case QAbstractItemView::MoveUp:
      return index.siblingAtRow(row > 0 ? row - 1 : row);
    case QAbstractItemView::MoveDown:
      return index.siblingAtRow(row < this->model()->rowCount() - 1 ? row + 1 : row);
    case QAbstractItemView::MoveLeft:
      return index.siblingAtRow(row > 0 ? row - 1 : row);
    case QAbstractItemView::MoveRight:
      return index.siblingAtRow(row < this->model()->rowCount() - 1 ? row + 1 : row);
    case QAbstractItemView::MoveHome:
      return index.siblingAtRow(0);
    case QAbstractItemView::MoveEnd:
      return index.siblingAtRow(this->model()->rowCount() - 1);
    case QAbstractItemView::MovePageUp:
      // TODO
      break;
    case QAbstractItemView::MovePageDown:
      // TODO
      break;
    case QAbstractItemView::MoveNext:
      // TODO
      break;
    case QAbstractItemView::MovePrevious:
      // TODO
      break;
    }
  }
  return index;
}

void qtOperationTypeView::setSelection(
  const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
  (void) rect;
  (void) flags;
  // TODO
}

QRegion qtOperationTypeView::visualRegionForSelection(const QItemSelection& selection) const
{
  (void) selection;
  QRegion region;
  return region;
}

bool qtOperationTypeView::eventFilter(QObject* object, QEvent* event)
{
  if (/*object == this->viewport() &&*/ event->type() == QEvent::Resize)
  {
    //this->updateScrollBars();
    std::cout << "Update scrollbars, obj " << object->objectName().toStdString() << "\n";
  }
  return this->Superclass::eventFilter(object, event);
}
#endif // 0
