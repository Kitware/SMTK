//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtBaseArc.h"

#include "smtk/extension/qt/diagram/qtBaseNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"
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

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

qtBaseArc::qtBaseArc(
  qtDiagramGenerator* generator,
  qtBaseNode* predecessor,
  qtBaseNode* successor,
  ArcType arcType,
  QGraphicsItem* parent)
  : Superclass(parent)
  , m_generator(generator)
  , m_predecessor(predecessor)
  , m_successor(successor)
  , m_arcType(arcType)
{
  QObject::connect(m_predecessor, &qtBaseNode::nodeMoved, this, &qtBaseArc::updateArcPoints);
  QObject::connect(m_predecessor, &qtBaseNode::nodeResized, this, &qtBaseArc::updateArcPoints);
  QObject::connect(m_successor, &qtBaseNode::nodeMoved, this, &qtBaseArc::updateArcPoints);
  QObject::connect(m_successor, &qtBaseNode::nodeResized, this, &qtBaseArc::updateArcPoints);
  this->setAcceptedMouseButtons(Qt::NoButton);

  // Do not cache arcs as they will typically have large bounding boxes containing very
  // few rendered pixels. Also, they are relatively fast to render.
  this->setCacheMode(QGraphicsItem::NoCache);

  // Make selecting individual arcs possible by requiring pixel-accurate bounds on this item.
  this->setBoundingRegionGranularity(1.00);
}

qtBaseArc::~qtBaseArc() = default;

QRectF qtBaseArc::boundingRect() const
{
  const auto& cfg(*this->scene()->configuration());
  const qreal margin = cfg.arcWidth() + cfg.arcOutline();

  QRectF pb = this->path().boundingRect();
  return pb.adjusted(-margin, -margin, margin, margin);
}

qtDiagramScene* qtBaseArc::scene() const
{
  return m_generator ? m_generator->diagram()->diagramScene() : nullptr;
}

void qtBaseArc::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  (void)option;
  (void)widget;
  const auto& cfg(*this->scene()->configuration());

  QColor color = cfg.colorForArcType(m_arcType);
  qreal width = cfg.arcWidth();
  if (this->isSelected())
  {
    width *= 2;
    color.setAlphaF(1.0);
  }
  else
  {
    color.setAlphaF(0.75);
  }
  QPen pen;
  pen.setWidth(width);
  pen.setBrush(color);

  // painter->setPen(pen);
  painter->strokePath(this->path(), pen);
  painter->fillPath(m_arrowPath, pen.brush());
}

} // namespace extension
} // namespace smtk
