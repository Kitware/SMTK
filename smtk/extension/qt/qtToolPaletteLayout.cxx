//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtToolPaletteLayout.h"

#include <QtWidgets>

qtToolPaletteLayout::qtToolPaletteLayout(QWidget* parent, int margin, int hSpacing, int vSpacing)
  : QLayout(parent)
  , m_hSpace(hSpacing)
  , m_vSpace(vSpacing)
{
  setContentsMargins(margin, margin, margin, margin);
}

qtToolPaletteLayout::qtToolPaletteLayout(int margin, int hSpacing, int vSpacing)
  : m_hSpace(hSpacing)
  , m_vSpace(vSpacing)
{
  setContentsMargins(margin, margin, margin, margin);
}

qtToolPaletteLayout::~qtToolPaletteLayout()
{
  QLayoutItem* item;
  while ((item = takeAt(0)))
    delete item;
}

void qtToolPaletteLayout::addItem(QLayoutItem* item)
{
  m_itemList.append(item);
}

void qtToolPaletteLayout::insertItem(int insertAfter, QLayoutItem* item)
{
  // Adjust insertion position if needed.
  insertAfter = qMax(0, insertAfter);
  insertAfter = qMin(m_itemList.count(), insertAfter);

  if (item && item->widget())
  {
    item->widget()->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    item->widget()->hide(); // This will get shown only if doLayout() places it.
  }

  m_itemList.insert(insertAfter, item);
}

int qtToolPaletteLayout::horizontalSpacing() const
{
  if (m_hSpace >= 0)
  {
    return m_hSpace;
  }
  return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int qtToolPaletteLayout::verticalSpacing() const
{
  if (m_vSpace >= 0)
  {
    return m_vSpace;
  }
  return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int qtToolPaletteLayout::count() const
{
  return m_itemList.size();
}

QLayoutItem* qtToolPaletteLayout::itemAt(int index) const
{
  return m_itemList.value(index);
}

QLayoutItem* qtToolPaletteLayout::takeAt(int index)
{
  if (index >= 0 && index < m_itemList.size())
  {
    auto* item = m_itemList.takeAt(index);
    return item;
  }
  return nullptr;
}

int qtToolPaletteLayout::indexAt(const QPoint& point) const
{
  int left, top, right, bottom;
  int index = -1;
  getContentsMargins(&left, &top, &right, &bottom);
  QPoint offset(point.x() - left, point.y() - top);
  if (offset.x() < 0 || offset.y() < 0)
  {
    return index;
  }
  int ii = offset.x() / m_effectiveSpacing.width();
  int jj = offset.y() / m_effectiveSpacing.height();
  if (ii > m_effectiveColumns)
  {
    return index;
  }
  index = jj * m_effectiveColumns + ii;
  return index;
}

Qt::Orientations qtToolPaletteLayout::expandingDirections() const
{
  return Qt::Vertical;
}

bool qtToolPaletteLayout::hasHeightForWidth() const
{
  return true;
}

int qtToolPaletteLayout::heightForWidth(int width) const
{
  int height = doLayout(QRect(0, 0, width, 0), true);
  return height;
}

void qtToolPaletteLayout::setGeometry(const QRect& rect)
{
  int height = doLayout(rect, false);
  QLayout::setGeometry(QRect(rect.x(), rect.y(), rect.width(), height));
}

QSize qtToolPaletteLayout::sizeHint() const
{
  return minimumSize();
}

QSize qtToolPaletteLayout::minimumSize() const
{
  QSize size;
  QLayoutItem* item;
  Q_FOREACH (item, m_itemList)
    size = size.expandedTo(item->minimumSize());

  size += QSize(2 * margin(), 2 * margin());
  return size;
}

int qtToolPaletteLayout::doLayout(const QRect& rect, bool testOnly) const
{
  int left, top, right, bottom;
  getContentsMargins(&left, &top, &right, &bottom);
  QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
  int x = effectiveRect.x();
  int y = effectiveRect.y();

  // Determine number of columns and width of each.
  QLayoutItem* item;
  int maxWidth = 1;
  int maxHeight = 1;
  int numItems = 0;
  Q_FOREACH (item, m_itemList)
  {
    // TODO: We could factor in wid->style()->layoutSpacing(...)
    //       instead of relying on horizontal/verticalSpacing()
    //       to be a fixed positive number.
    int itemWidth = item->sizeHint().width();
    if (itemWidth > maxWidth)
    {
      maxWidth = itemWidth;
    }
    int itemHeight = item->sizeHint().height();
    if (itemHeight > maxHeight)
    {
      maxHeight = itemHeight;
    }
    ++numItems;
  }
  int hSpace = this->horizontalSpacing() == -1 ? 5 : this->horizontalSpacing();
  int numCols = std::floor(
    static_cast<double>(effectiveRect.width() + hSpace) / static_cast<double>(maxWidth + hSpace));
  if (numCols == 0)
  {
    ++numCols;
  }
  int vSpace = this->verticalSpacing() == -1 ? 5 : this->verticalSpacing();
  int numRows = numItems / numCols + (numItems % numCols ? 1 : 0);
  int effectiveHeight = maxHeight * numRows + (maxHeight - 1) * vSpace;
  int totalHeight = bottom + effectiveHeight + top;
  if (testOnly)
  {
    return totalHeight;
  }

  maxWidth = qMax(maxWidth, (effectiveRect.width() - (numCols - 1) * hSpace) / numCols);
  m_effectiveSpacing = QSize(maxWidth + hSpace, maxHeight + vSpace);
  m_effectiveColumns = numCols;

  int ii = 0;
  Q_FOREACH (item, m_itemList)
  {
    QWidget* wid = item->widget();
    x = effectiveRect.x() + (maxWidth + hSpace) * (ii % numCols);
    y = effectiveRect.y() + (maxHeight + vSpace) * (ii / numCols);
    item->setGeometry(QRect(QPoint(x, y), QSize(maxWidth, maxHeight)));
    wid->setGeometry(QRect(QPoint(x, y), QSize(maxWidth, maxHeight)));
    wid->show(); // Only show widgets we have laid out.
    ++ii;
  }
  return totalHeight;
}

int qtToolPaletteLayout::smartSpacing(QStyle::PixelMetric pm) const
{
  QObject* parent = this->parent();
  if (!parent)
  {
    return -1;
  }
  else if (parent->isWidgetType())
  {
    QWidget* pw = static_cast<QWidget*>(parent);
    return pw->style()->pixelMetric(pm, nullptr, pw);
  }
  return static_cast<QLayout*>(parent)->spacing();
}
