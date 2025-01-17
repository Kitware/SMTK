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

#include "smtk/extension/qt/diagram/qtBaseTaskNode.h"
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
#include "smtk/task/ObjectsInRoles.h"

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
namespace
{

std::string objTypeName(
  smtk::resource::PersistentObject* obj,
  std::unordered_map<smtk::string::Token, smtk::string::Token>* typeLabels)
{
  smtk::string::Token tn = obj->typeName();
  if (typeLabels)
  {
    auto it = typeLabels->find(tn);
    if (it != typeLabels->end())
    {
      tn = it->second;
    }
  }
  return tn.data();
}

void summarizePortData(
  std::ostringstream& ttip,
  const std::shared_ptr<smtk::task::ObjectsInRoles>& pdata,
  std::unordered_map<smtk::string::Token, smtk::string::Token>* typeLabels)
{
  if (!pdata || pdata->data().empty())
  {
    return;
  }

  ttip << "<ul>";
  for (const auto& roleEntry : pdata->data())
  {
    ttip << "<li><i>" << roleEntry.first.data() << "</i>";
    if (!roleEntry.second.empty())
    {
      ttip << "<ul>";
      for (const auto& obj : roleEntry.second)
      {
        ttip << "<li><b>" << obj->name() << "</b> (" << objTypeName(obj, typeLabels) << ")";
        if (auto* port = dynamic_cast<smtk::task::Port*>(obj))
        {
          ttip << " from " << port->parent()->name();
        }
        ttip << "</li>";
      }
      ttip << "</ul>";
    }
    ttip << "</li>";
  }
  ttip << "</ul>";
}

} // anonymous namespace

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
    this->updateToolTip();
  }
  this->setZValue(cfg.nodeLayer() + 1);
}

QVariant qtTaskPortNode::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& val)
{
  if (change == ItemPositionChange && scene())
  {
    // correct the port orientation if needed
    QPointF newPos = val.toPointF();
    this->adjustOrientation(newPos);

    // Next take snapping into consideration
    this->adjustPosition(newPos);
    QVariant newVal(newPos);
    return Superclass::itemChange(change, newVal);
  }
  return Superclass::itemChange(change, val);
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

  qreal hl;
  if (m_port->access() == smtk::task::Port::Access::External)
  {
    hl = m_length * 0.5;
  }
  else
  {
    hl = m_length * 0.75;
  }
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
  if (this->isSelected())
  {
    qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
    const auto border = 0.5 * cfg.nodeBorderThickness();
    return m_path.boundingRect().adjusted(-border, -border, border, border);
  }
  return m_path.boundingRect();
}

void qtTaskPortNode::paint(
  QPainter* painter,
  const QStyleOptionGraphicsItem* option,
  QWidget* widget)
{
  (void)option;
  (void)widget;
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());

  painter->fillPath(m_path, cfg.portNodeColor());

  QPen spen;
  if (this->isSelected())
  {
    spen.setWidth(cfg.nodeBorderThickness());
    spen.setBrush(cfg.selectionColor());
  }
  else
  {
    spen.setWidth(cfg.nodeBorderThickness() * 0.25);
    spen.setBrush(cfg.borderColor());
  }
  painter->setPen(spen);
  painter->drawPath(m_path);
}

void qtTaskPortNode::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  this->updateToolTip();
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

void qtTaskPortNode::updateToolTip()
{
  auto mgrs = this->diagram()->managers();
  auto rsrcMgr = mgrs ? mgrs->get<smtk::resource::Manager::Ptr>() : nullptr;
  auto* typeLabels = rsrcMgr ? &rsrcMgr->objectTypeLabels() : nullptr;

  if (m_port)
  {
    std::ostringstream ttip;
    ttip << "<html><body><h1>" << m_port->name() << "</h1>";
    if (m_port->direction() == smtk::task::Port::Direction::In && !m_port->connections().empty())
    {
      ttip << "<ul>";
      for (const auto& obj : m_port->connections())
      {
        if (obj)
        {
          ttip << "<li><b>" << obj->name() << "</b> (" << objTypeName(obj, typeLabels) << ")";
          if (auto* port = dynamic_cast<smtk::task::Port*>(obj))
          {
            ttip << " from " << port->parent()->name();
            auto pdata =
              std::dynamic_pointer_cast<smtk::task::ObjectsInRoles>(m_port->portData(port));
            summarizePortData(ttip, pdata, typeLabels);
          }
          else
          {
            ttip << " as <i>" << m_port->unassignedRole().data() << "</i>";
          }
          ttip << "</li>";
        }
      }
      ttip << "</ul>";
    }
    else if (m_port->direction() == smtk::task::Port::Direction::Out)
    {
      auto pdata =
        std::dynamic_pointer_cast<smtk::task::ObjectsInRoles>(m_port->parent()->portData(m_port));
      summarizePortData(ttip, pdata, typeLabels);
    }
    ttip << "</body></html>";
    this->setToolTip(QString::fromStdString(ttip.str()));
  }
}

void qtTaskPortNode::adjustOrientation(const QPointF& pnt)
{
  // Determine where the pnt is relative to the port's  parent task and then properly
  // rotate the port.
  if (!this->parentItem())
  {
    return;
  }
  double origAngle = this->rotation();
  double angle;

  auto pBox = this->parentItem()->boundingRect();

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
  if (origAngle != angle)
  {
    this->setRotation(angle);
  }
}

void qtTaskPortNode::adjustPosition(QPointF& pnt)
{
  // See if the task node is requesting its ports to be snapped.
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  if ((m_port->access() == smtk::task::Port::Access::Internal) || !cfg.snapPortsToTask())
  {
    return;
  }

  auto taskBounds = this->parentItem()->boundingRect();
  double hl = (m_length * 0.5) + cfg.snapPortOffset();
  ;

  if (pnt.x() <= taskBounds.left())
  {
    // change X if the new point is too far away from the node
    double xmin = taskBounds.left() - hl;
    if (pnt.x() < xmin)
    {
      pnt.setX(xmin);
    }
    // Move the point away if it is too close and if there is
    // a possibility that it would intersect the task node
    else if (((pnt.y() + hl) < taskBounds.bottom()) && ((pnt.y() - hl) > taskBounds.top()))
    {
      pnt.setX(xmin);
    }
  }
  else if (pnt.x() >= taskBounds.right())
  {
    // change X if the new point is too far away from the node
    double xmax = taskBounds.right() + hl;
    if (pnt.x() > xmax)
    {
      pnt.setX(xmax);
    }
    // Move the point away if it is too close and if there is
    // a possibility that it would intersect the task node
    else if (((pnt.y() + hl) < taskBounds.bottom()) && ((pnt.y() - hl) > taskBounds.top()))
    {
      pnt.setX(xmax);
    }
  }
  else if (pnt.y() >= taskBounds.center().y())
  {
    // Change Y so that the point is near the top of the task node
    double ymin = taskBounds.bottom() + hl;
    pnt.setY(ymin);
  }
  else
  {
    // Change Y so that the point is near the bottom of the task node
    double ymax = taskBounds.top() - hl;
    pnt.setY(ymax);
  }
}

} // namespace extension
} // namespace smtk
