//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/rgg/qt/nuclides/Nuclide.h"

#include <QtWidgets>

#include <array>

namespace smtk
{
namespace bridge
{
namespace rgg
{

Nuclide::Nuclide(unsigned int N, unsigned int Z, const QString& symbol, const QString& jn,
  MainDecayMode decayMode, double halfLife)
  : m_N(N)
  , m_Z(Z)
  , m_symbol(symbol)
  , m_jn(jn)
  , m_decayMode(decayMode)
  , m_halfLife(halfLife)
{
}

void Nuclide::setSelectable(bool choice)
{
  setFlags(choice ? ItemIsSelectable : QGraphicsItem::GraphicsItemFlags());
  setAcceptHoverEvents(choice);
}

QRectF Nuclide::boundingRect() const
{
  return QRectF(0, 0, 100, 100);
}

QPainterPath Nuclide::shape() const
{
  QPainterPath path;
  path.addRect(0, 0, 100, 100);
  return path;
}

void Nuclide::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  Q_UNUSED(widget);

  QColor fillColor = (option->state & QStyle::State_Selected) ? color().dark(150) : color();
  if (option->state & QStyle::State_MouseOver)
  {
    fillColor = fillColor.light(125);
  }

  const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
  if (lod < 0.2)
  {
    if (lod < 0.125)
    {
      painter->fillRect(QRectF(0, 0, 100, 100), fillColor);
      return;
    }

    QPen oldPen = painter->pen();
    QPen pen = oldPen;
    int width = 0;
    if (option->state & QStyle::State_Selected)
      width += 2;

    pen.setWidth(width);
    QBrush b = painter->brush();
    painter->setBrush(fillColor);
    painter->drawRect(0, 0, 100, 100);
    painter->setBrush(b);
    return;
  }

  QPen oldPen = painter->pen();
  QPen pen = oldPen;
  int width = 0;
  if (option->state & QStyle::State_Selected)
    width += 2;

  pen.setWidth(width);
  QBrush b = painter->brush();
  painter->setBrush(QBrush(fillColor.dark(option->state & QStyle::State_Sunken ? 120 : 100)));

  painter->drawRect(QRect(0, 0, 100, 100));
  painter->setBrush(b);

  // Draw text
  if (lod >= .25)
  {
    QFont font("Times", 100);
    painter->setFont(font);
    painter->save();

    double brightness =
      fillColor.red() * 0.299 + fillColor.green() * 0.587 + fillColor.blue() * 0.114;
    QColor foreground = (brightness > 160 ? QColor(0, 0, 0) : QColor(255, 255, 255));
    painter->setPen(foreground);
    painter->scale(0.2, 0.2);
    QString zVal = QString("%1").arg(this->A());
    painter->drawText(80 - 25 * (m_symbol.size() - 1) - 10 * (zVal.size() - 1), 110, zVal);
    painter->scale(2, 2);
    painter->drawText(120 - 20 * m_symbol.size(), 135, m_symbol);

    if (lod >= 1.1)
    {
      painter->scale(.25, .25);
      painter->drawText(210, 700, QString("n = %1").arg(m_N));
      painter->drawText(560, 700, QString("z = %1").arg(m_Z));
      painter->scale(.8, .8);
      painter->drawText(310, 950, "n");
      painter->scale(1.25, 1.25);
      painter->drawText(210, 800, QString("J  = %1").arg(m_jn.isEmpty() ? "?" : m_jn));
      if (m_decayMode != MainDecayMode::stable && m_decayMode != MainDecayMode::unknown)
      {
        painter->scale(.8, .8);
        painter->drawText(310, 1150, "1/2");
        painter->scale(1.25, 1.25);
        painter->drawText(210, 900, QString("t     = %1").arg(m_halfLife));
      }
    }

    painter->restore();
  }
}

QString Nuclide::name() const
{
  return m_symbol.toLower() + QString::number(this->A());
}

QString Nuclide::prettyName() const
{
  return "(" + QString::number(this->A()) + ")" + m_symbol;
}

QPixmap Nuclide::toPixmap() const
{
  if (!scene())
  {
    qWarning() << "[Nuclide::toPixmap] scene is null.";
    return QPixmap();
  }

  QRectF r = boundingRect();
  QPixmap pixmap(r.width(), r.height());
  pixmap.fill(QColor(0, 0, 0, 0));
  QPainter painter(&pixmap);
  painter.drawRect(r);
  scene()->render(&painter, QRectF(), sceneBoundingRect());
  painter.end();
  return pixmap.scaled(30, 30);
}

void Nuclide::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mousePressEvent(event);
  update();
  if (event->button() == Qt::LeftButton)
  {
    QDrag* drag = new QDrag(scene());
    QMimeData* mimeData = new QMimeData;

    mimeData->setText(name());
    drag->setMimeData(mimeData);
    drag->setPixmap(this->toPixmap());

    drag->exec();
  }
}

void Nuclide::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mouseReleaseEvent(event);
  update();
}

QColor Nuclide::color() const
{
  static const QColor yellow(220, 220, 112);
  static const QColor teal(0, 205, 205);
  static const QColor mustard(216, 170, 0);
  static const QColor magenta(205, 0, 205);
  static const QColor green(0, 180, 0);
  static const QColor salmon(255, 156, 175);
  static const QColor black(0, 0, 0);
  static const QColor gray(211, 211, 211);

  static const std::array<const QColor*, 8> decayColors = { { &yellow, &teal, &mustard, &magenta,
    &green, &salmon, &black, &gray } };

  return *decayColors[static_cast<int>(m_decayMode)];
}
}
}
}
