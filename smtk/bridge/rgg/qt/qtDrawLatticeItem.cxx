//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/qt/qtDrawLatticeItem.h"

#include <QDebug>
#include <QFont>
#include <QPainter>

#include <cmath>

#include <iostream>

qtDrawLatticeItem::qtDrawLatticeItem(
  const QPolygonF& poly, int l, int cellIdx, qtCellReference const& ref, QGraphicsItem* parent)
  : QGraphicsPolygonItem(poly, parent)
  , m_refCell(ref)
  , m_layer(l)
  , m_cellIndex(cellIdx)

{
  this->setAcceptDrops(true);
  this->m_localCenter = QPointF(0, 0);
  for (int i = 0; i < poly.count(); ++i)
  {
    this->m_localCenter += poly.value(i);
  }
  this->m_localCenter /= poly.count();
}

QString qtDrawLatticeItem::text() const
{
  return m_refCell.getCell()->getLabel();
}

int qtDrawLatticeItem::layer()
{
  return m_layer;
}

int qtDrawLatticeItem::cellIndex()
{
  return m_cellIndex;
}

void qtDrawLatticeItem::drawText(QPainter* painter)
{
  QRectF trect = this->boundingRect();
  qtCell const* cell = m_refCell.getModeCell();
  QColor c = cell->getColor();
  double gray = c.red() * 0.299 + c.green() * 0.587 + c.blue() * 0.114;

  QColor textColor;

  switch (m_refCell.getDrawMode())
  {
    case qtCellReference::SELECTED:
    case qtCellReference::NORMAL:
    case qtCellReference::FUNCTION_APPLY:
      textColor = (gray < 186) ? Qt::white : Qt::black;
      break;
    case qtCellReference::UNSELECTED:
      textColor = (gray < 186) ? QColor(200, 200, 200, 100) : QColor(50, 50, 50, 100);
      break;
  }

  QFont font;
  font.setPixelSize(12);
  painter->setPen(textColor);
  QString txt = cell->getLabel();
  if (m_refCell.isInConflict())
  {
    txt = "!" + txt + "!";
    font.setBold(true);
    font.setStrikeOut(true);
  }
  painter->setFont(font);
#ifdef DEBUG_CORDINATE
  double mr = m_refCell.getMaxRadius();
  txt = QString::number(mr, 'f', 2);
#endif
  painter->drawText(trect, Qt::AlignCenter | Qt::AlignCenter | Qt::TextWordWrap, txt);
}

void qtDrawLatticeItem::paint(
  QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  qtCell const* cell = m_refCell.getModeCell();
  QColor c = cell->getColor();
  double gray = c.red() * 0.299 + c.green() * 0.587 + c.blue() * 0.114;
  switch (m_refCell.getDrawMode())
  {
    case qtCellReference::SELECTED:
      this->setPen(QPen(Qt::darkCyan, 3));
      this->setAcceptDrops(true);
      break;
    case qtCellReference::NORMAL:
      this->setPen(QPen(Qt::black));
      this->setAcceptDrops(true);
      break;
    case qtCellReference::UNSELECTED:
    {
      int g = qGray(c.rgb());
      if (g >= 230)
        g -= 25;
      c = QColor(g, g, g, 175);
      this->setPen(QPen(Qt::white, 0));
      this->setAcceptDrops(false);
      break;
    }
    case qtCellReference::FUNCTION_APPLY:
      this->setPen(QPen(Qt::darkRed, 2));
      this->setAcceptDrops(true);
      break;
  }
  this->setBrush(QBrush(c));

  this->Superclass::paint(painter, option, widget);

  if (m_refCell.isInConflict())
  {
    if (gray < 186)
    {
      c = c.lighter();
    }
    else
    {
      c = c.darker();
    }
    painter->setBrush(QBrush(c, Qt::DiagCrossPattern));
    painter->drawPolygon(this->polygon());
  }

  this->drawText(painter);
}

smtk::model::EntityRef qtDrawLatticeItem::getPart()
{
  qtCell const* cell = m_refCell.getCell();
  return (cell) ? cell->getPart() : smtk::model::EntityRef();
}

QPointF qtDrawLatticeItem::getCentroid() const
{
  return this->m_localCenter + pos();
}
