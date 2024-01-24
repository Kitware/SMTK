//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtGroupingNode.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtResourceDiagram.h"
#include "smtk/extension/qt/diagram/qtResourceDiagramSummary.h"
#include "smtk/extension/qt/qtBaseView.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsTextItem>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QPainter>

class QAbstractItemModel;
class QItemSelection;

namespace smtk
{
namespace extension
{

qtGroupingNode::qtGroupingNode(
  qtDiagramGenerator* generator,
  smtk::string::Token group,
  QGraphicsItem* parent)
  : Superclass(generator, parent)
  , m_group(group)
  , m_id(smtk::common::UUID::random())
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());

  this->setAcceptHoverEvents(true);
  if (!parent)
  {
    this->addToScene();
  }

  // === Node-specific constructor ===
  this->setZValue(cfg.nodeLayer());
}

qtGroupingNode::qtGroupingNode(
  const smtk::common::UUID& uid,
  const std::array<double, 2>& location,
  qtDiagramGenerator* generator,
  smtk::string::Token group,
  QGraphicsItem* parent)
  : Superclass(generator, parent)
  , m_group(group)
  , m_id(uid)
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());

  this->setAcceptHoverEvents(true);
  if (!parent)
  {
    this->addToScene();
  }

  // === Node-specific constructor ===
  this->setZValue(cfg.nodeLayer());
  this->setPos(location[0], location[1]);
}

smtk::common::UUID qtGroupingNode::nodeId() const
{
  return m_id;
}

QRectF qtGroupingNode::boundingRect() const
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  const auto border = 4 * cfg.nodeBorderThickness();
  const double height = 10;
  return QRectF(0, 0, 10, height).adjusted(-border, -border, border, border);
}

void qtGroupingNode::paint(
  QPainter* painter,
  const QStyleOptionGraphicsItem* option,
  QWidget* widget)
{
  (void)option;
  (void)widget;
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  QPainterPath path;
  // Make sure the whole node is redrawn to avoid artifacts:
  const double borderOffset = cfg.nodeBorderThickness();
  const QRectF br =
    this->boundingRect().adjusted(borderOffset, borderOffset, -borderOffset, -borderOffset);
  path.addRoundedRect(br, 2 * cfg.nodeBorderThickness(), 2 * cfg.nodeBorderThickness());

  QColor baseColor = QApplication::palette().window().color();
  baseColor.setAlphaF(0.5);
  const QColor contrastColor = QColor::fromHslF(
    baseColor.hueF(),
    baseColor.saturationF(),
    baseColor.lightnessF() > 0.5 ? baseColor.lightnessF() - 0.5 : baseColor.lightnessF() + 0.5,
    /* alpha */ 0.5);

  QPen pen;
  pen.setWidth(cfg.nodeBorderThickness() * 1.1);
  pen.setBrush(contrastColor);

  painter->setPen(pen);
  painter->fillPath(path, baseColor);
  painter->drawPath(path);

  if (this->isSelected())
  {
    QPen spen;
    spen.setWidth(cfg.nodeBorderThickness());
    QPainterPath selPath;
    const QColor contrastColor2 = QColor::fromHslF(
      pen.brush().color().hueF(),
      pen.brush().color().saturationF(),
      qBound(
        0.,
        baseColor.lightnessF() > 0.5 ? pen.brush().color().lightnessF() + 0.25
                                     : pen.brush().color().lightnessF() - 0.25,
        1.));
    spen.setBrush(contrastColor2);
    painter->setPen(spen);
    const QRectF selRect = br.adjusted(borderOffset, borderOffset, -borderOffset, -borderOffset);
    selPath.addRoundedRect(selRect, cfg.nodeBorderThickness(), cfg.nodeBorderThickness());
    painter->drawPath(selPath);
  }
}

void qtGroupingNode::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  (void)event;
  auto* gen = dynamic_cast<qtResourceDiagram*>(this->generator());
  if (gen)
  {
    gen->summarizer()->setSubject(this);
  }
  this->Superclass::hoverEnterEvent(event);
}

void qtGroupingNode::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  (void)event;
  auto* gen = dynamic_cast<qtResourceDiagram*>(this->generator());
  if (gen)
  {
    gen->summarizer()->setSubject(nullptr);
  }
  this->Superclass::hoverLeaveEvent(event);
}

int qtGroupingNode::updateSize()
{
  return 1;
#if 0
  this->prepareGeometryChange();

  m_container->resize(m_container->layout()->sizeHint());
  Q_EMIT this->nodeResized();

  return 1;
#endif
}

void qtGroupingNode::dataUpdated()
{
  // Do nothing.
}

bool qtGroupingNode::setLabel(smtk::string::Token label)
{
  if (!label.valid() || m_label == label)
  {
    return false;
  }
  m_label = label;
  return true;
}

smtk::string::Token qtGroupingNode::label() const
{
  return m_label.valid() ? m_label : m_group;
}

std::string qtGroupingNode::name() const
{
  return (m_label.valid() ? m_label : m_group).data();
}

} // namespace extension
} // namespace smtk
