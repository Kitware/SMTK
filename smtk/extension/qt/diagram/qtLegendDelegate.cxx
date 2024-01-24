//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtLegendDelegate.h"

#include "smtk/extension/qt/diagram/qtBaseNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"
#include "smtk/extension/qt/diagram/qtDiagramLegend.h"
#include "smtk/extension/qt/diagram/qtDiagramLegendEntry.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/qtBaseView.h"

#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsTextItem>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QStyledItemDelegate>

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

qtLegendDelegate::qtLegendDelegate(QObject* parent)
  : Superclass(parent)
{
}

qtLegendDelegate::~qtLegendDelegate() = default;

bool qtLegendDelegate::setHoverIndex(const QModelIndex& index)
{
  if (m_hoverIndex == index)
  {
    return false;
  }

  m_hoverIndex = index;
  return true;
}

QSize qtLegendDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  (void)option;
  (void)index;
  return option.rect.isEmpty() ? QSize(16, 16) : option.rect.size();
}

void qtLegendDelegate::paint(
  QPainter* painter,
  const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  if (index.column() == qtDiagramLegend::Column::Symbol)
  {
    if (m_hoverIndex == index)
    {
      std::cout << "Draw icons for "
                << index.sibling(index.row(), qtDiagramLegend::Column::Description)
                     .data()
                     .toString()
                     .toStdString()
                << "\n";
    }
    if (auto* qo = qvariant_cast<QObject*>(index.data(Qt::UserRole + 1)))
    {
      if (auto* entry = qobject_cast<qtDiagramLegendEntry*>(qo))
      {
        entry->paint(painter, option, nullptr);
      }
    }
  }
  else
  {
    return Superclass::paint(painter, option, index);
  }
}

} // namespace extension
} // namespace smtk
