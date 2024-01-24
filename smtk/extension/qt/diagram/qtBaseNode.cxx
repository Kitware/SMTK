//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtBaseNode.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/task/Active.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Task.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsTextItem>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QPainter>
#include <QTimer>

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

qtBaseNode::qtBaseNode(qtDiagramGenerator* generator, QGraphicsItem* parent)
  : Superclass(parent)
  , m_generator(generator)
{
  this->setFlag(GraphicsItemFlag::ItemIsMovable);
  this->setFlag(GraphicsItemFlag::ItemIsSelectable);
  this->setFlag(GraphicsItemFlag::ItemSendsGeometryChanges);
  this->setCacheMode(CacheMode::DeviceCoordinateCache);
  this->setCursor(Qt::ArrowCursor);
}

qtBaseNode::~qtBaseNode() = default;

QRectF qtBaseNode::boundingRect() const
{
  QRectF dummy;
  return dummy;
}

qtDiagram* qtBaseNode::diagram() const
{
  return m_generator->diagram();
}

qtDiagramScene* qtBaseNode::scene() const
{
  return m_generator->diagram()->diagramScene();
}

void qtBaseNode::setContentStyle(ContentStyle cs)
{
  m_contentStyle = cs;
}

QVariant qtBaseNode::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == GraphicsItemChange::ItemPositionHasChanged)
  {
    Q_EMIT this->nodeMoved();
  }

  return QGraphicsItem::itemChange(change, value);
}

int qtBaseNode::updateSize()
{
  this->prepareGeometryChange();

  Q_EMIT this->nodeResized();

  return 1;
}

void qtBaseNode::addToScene()
{
  this->updateSize();
  this->scene()->addItem(this);
}

} // namespace extension
} // namespace smtk
