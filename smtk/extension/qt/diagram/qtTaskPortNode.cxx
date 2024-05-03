//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// This MUST COME FIRST so that M_PI is defined by <cmath> on windows.
#define _USE_MATH_DEFINES

#include "smtk/extension/qt/diagram/qtTaskPortNode.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtResourceDiagram.h"
#include "smtk/extension/qt/diagram/qtResourceDiagramSummary.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/Managers.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

#include <cmath>

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

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

constexpr double pi = 3.14159265358979323846;

namespace smtk
{
namespace extension
{

qtTaskPortNode::qtTaskPortNode(
  qtDiagramGenerator* generator,
  smtk::resource::PersistentObject* obj,
  QGraphicsItem* parent)
  : Superclass(generator, obj, parent)
  , m_length(20)
  , m_angle(90)
  , m_port(dynamic_cast<smtk::task::Port*>(obj))
{
  this->updateShape();

  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  this->setAcceptHoverEvents(true);

  if (!parent)
  {
    this->addToScene();
  }

  // === Task-specific constructor ===
  if (m_port)
  {
    if (m_port->direction() == task::Port::Direction::In)
    {
      if (parent)
      {
        this->setX(parent->boundingRect().left() - 15);
      }
      else
      {
        this->setX(-100);
      }
    }
    else
    {
      if (parent)
      {
        this->setX(parent->boundingRect().right() + 15);
      }
      else
      {
        this->setX(100);
      }
    }
    this->setToolTip(m_port->name().c_str());
  }
  this->setZValue(cfg.nodeLayer() + 1);
}

smtk::common::UUID qtTaskPortNode::nodeId() const
{
  return m_port ? m_port->id() : smtk::common::UUID::null();
}

smtk::resource::PersistentObject* qtTaskPortNode::object() const
{
  return m_port;
}

void qtTaskPortNode::setLength(qreal newLength)
{
  if (newLength > 1)
  {
    m_length = newLength;
    this->updateShape();
  }
}
void qtTaskPortNode::setAngle(qreal newAngle)
{
  if ((newAngle > 10) && (newAngle < 170))
  {
    m_angle = newAngle;
    this->updateShape();
  }
}

void qtTaskPortNode::updateShape()
{
  m_path.clear();

  if (!m_port)
  {
    return; // No port has been set
  }

  qreal hl = m_length * 0.5;
  qreal xmax = hl * (1 + (1.0 / tan(0.5 * m_angle * pi / 180.0)));

  if (m_port->direction() == task::Port::Direction::In)
  {
    m_path.moveTo(hl, hl);
    m_path.lineTo(hl, -hl);
    m_path.lineTo(-xmax, -hl);
    m_path.lineTo(-hl, 0.0);
    m_path.lineTo(-xmax, hl);
    m_path.closeSubpath();
  }
  else
  {
    m_path.moveTo(hl, hl);
    m_path.lineTo(xmax, 0.0);
    m_path.lineTo(hl, -hl);
    m_path.lineTo(-hl, -hl);
    m_path.lineTo(-hl, hl);
    m_path.closeSubpath();
  }
}
QRectF qtTaskPortNode::boundingRect() const
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  const auto border = 4 * cfg.nodeBorderThickness();
  return m_path.boundingRect().adjusted(-border, -border, border, border);
}

void qtTaskPortNode::paint(
  QPainter* painter,
  const QStyleOptionGraphicsItem* option,
  QWidget* widget)
{
  (void)option;
  (void)widget;
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());

  // Determine where the port is relative to its parent task and then properly
  // rotate it.
  this->adjustOrientation(this->scenePos());
  const QColor baseColor = QApplication::palette().window().color();
  const QColor contrastColor = QColor::fromHslF(
    baseColor.hueF(),
    baseColor.saturationF(),
    baseColor.lightnessF() > 0.5 ? baseColor.lightnessF() - 0.5 : baseColor.lightnessF() + 0.5);

  QPen pen;
  pen.setWidth(cfg.nodeBorderThickness() * 1.1);
  pen.setBrush(contrastColor);

  painter->setPen(pen);
  painter->fillPath(m_path, baseColor);
  painter->drawPath(m_path);

  if (this->isSelected())
  {
    QPen spen;
    spen.setWidth(cfg.nodeBorderThickness());
    QPainterPath selPath;
    const QColor selectedColor("#ff00ff");
    const QColor contrastColor2 = QColor::fromHslF(
      selectedColor.hueF(),
      selectedColor.saturationF(),
      qBound(
        0.,
        baseColor.lightnessF() > 0.5 ? selectedColor.lightnessF()
                                     : selectedColor.lightnessF() - 0.25,
        1.));
    spen.setBrush(contrastColor2);
    painter->setPen(spen);
    painter->drawPath(m_path);
    painter->fillPath(m_path, contrastColor2);
  }
}

void qtTaskPortNode::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  auto* gen = dynamic_cast<qtResourceDiagram*>(this->generator());
  if (gen)
  {
    gen->summarizer()->setSubject(this);
  }
  this->Superclass::hoverEnterEvent(event);
}

void qtTaskPortNode::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  auto* gen = dynamic_cast<qtResourceDiagram*>(this->generator());
  if (gen)
  {
    gen->summarizer()->setSubject(nullptr);
  }
  this->Superclass::hoverLeaveEvent(event);
}

void qtTaskPortNode::adjustOrientation(QPointF pnt)
{
  // Determine where the pnt is relative to the port's  parent task and then properly
  // rotate the port.
  if (!this->parentItem())
  {
    return;
  }
  double angle;

  auto pBox =
    this->parentItem()->boundingRegion(this->parentItem()->sceneTransform()).boundingRect();

  if (pnt.x() <= pBox.left())
  {
    angle = (m_port->direction() == task::Port::Direction::In) ? 0 : 180;
  }
  else if (pnt.x() >= pBox.right())
  {
    angle = (m_port->direction() == task::Port::Direction::In) ? 180 : 0;
  }
  else if (pnt.y() >= pBox.center().y())
  {
    angle = (m_port->direction() == task::Port::Direction::In) ? 270 : 90;
  }
  else
  {
    angle = (m_port->direction() == task::Port::Direction::In) ? 90 : 270;
  }
  this->setRotation(angle);
}

void qtTaskPortNode::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  // Determine where the port is relative to its parent task and then properly
  // rotate it.
  this->adjustOrientation(event->scenePos());
  this->Superclass::mouseMoveEvent(event);
}

} // namespace extension
} // namespace smtk
