//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtResourceNode.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtResourceDiagram.h"
#include "smtk/extension/qt/diagram/qtResourceDiagramSummary.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/Managers.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

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

#include "task/ui_ResourceNode.h"

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

qtResourceNode::qtResourceNode(
  qtDiagramGenerator* generator,
  smtk::resource::PersistentObject* obj,
  QGraphicsItem* parent)
  : Superclass(generator, obj, parent)
  , m_resource(dynamic_cast<smtk::resource::Resource*>(obj))
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  this->setAcceptHoverEvents(true);
  // Create a container to hold node contents
  {
    // install resize event filter
  }

  if (!parent)
  {
    this->addToScene();
  }

  // === Task-specific constructor ===
  this->setZValue(cfg.nodeLayer());
}

smtk::common::UUID qtResourceNode::nodeId() const
{
  return m_resource ? m_resource->id() : smtk::common::UUID::null();
}

smtk::resource::PersistentObject* qtResourceNode::object() const
{
  return m_resource;
}

QRectF qtResourceNode::boundingRect() const
{
  qtDiagramViewConfiguration& cfg(*this->scene()->configuration());
  const auto border = 4 * cfg.nodeBorderThickness();
  const double height = 15;
  const double width = 15;
  return QRectF(0, 0, width, height).adjusted(-border, -border, border, border);
}

void qtResourceNode::paint(
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
  path.addEllipse(br);

  const QColor baseColor = QApplication::palette().window().color();
  const QColor contrastColor = QColor::fromHslF(
    baseColor.hueF(),
    baseColor.saturationF(),
    baseColor.lightnessF() > 0.5 ? baseColor.lightnessF() - 0.5 : baseColor.lightnessF() + 0.5);

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
    const QRectF selRect = br.adjusted(borderOffset, borderOffset, -borderOffset, -borderOffset);
    selPath.addEllipse(selRect);
    painter->drawPath(selPath);
    painter->fillPath(selPath, contrastColor2);
  }
}

void qtResourceNode::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  auto* gen = dynamic_cast<qtResourceDiagram*>(this->generator());
  if (gen)
  {
    gen->summarizer()->setSubject(this);
  }
  this->Superclass::hoverEnterEvent(event);
}

void qtResourceNode::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  auto* gen = dynamic_cast<qtResourceDiagram*>(this->generator());
  if (gen)
  {
    gen->summarizer()->setSubject(nullptr);
  }
  this->Superclass::hoverLeaveEvent(event);
}

void qtResourceNode::dataUpdated()
{
  // NB: If resource nodes have color or text that should change
  //     based on an operation, do so here. There is nothing currently
  //     that will change the appearance of a resource node.
}

} // namespace extension
} // namespace smtk
