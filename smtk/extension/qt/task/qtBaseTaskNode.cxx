//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/qtBaseTaskNode.h"

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/task/qtTaskEditor.h"
#include "smtk/extension/qt/task/qtTaskScene.h"
#include "smtk/extension/qt/task/qtTaskViewConfiguration.h"

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

qtBaseTaskNode::qtBaseTaskNode(qtTaskScene* scene, smtk::task::Task* task, QGraphicsItem* parent)
  : Superclass(parent)
  , m_scene(scene)
  , m_task(task)
{
  this->setFlag(GraphicsItemFlag::ItemIsMovable);
  this->setFlag(GraphicsItemFlag::ItemSendsGeometryChanges);
  this->setCacheMode(CacheMode::DeviceCoordinateCache);
  this->setCursor(Qt::ArrowCursor);
  this->setObjectName(QString("node") + QString::fromStdString(m_task->title()));

  // Configure timer to rate-limit nodeMoved signal
  m_moveSignalTimer = new QTimer(this);
  m_moveSignalTimer->setSingleShot(true);
  m_moveSignalTimer->setInterval(100);
  QObject::connect(m_moveSignalTimer, &QTimer::timeout, this, &qtBaseTaskNode::nodeMoved);
}

qtBaseTaskNode::~qtBaseTaskNode() = default;

void qtBaseTaskNode::addToScene()
{
  this->updateSize();
  m_scene->addItem(this);
}

void qtBaseTaskNode::setContentStyle(ContentStyle cs)
{
  m_contentStyle = cs;
}

void qtBaseTaskNode::setOutlineStyle(OutlineStyle os)
{
  m_outlineStyle = os;
}

bool qtBaseTaskNode::isActive() const
{
  if (!m_task)
  {
    return false;
  }
  auto* taskManager = m_task->manager();
  if (!taskManager)
  {
    return false;
  }
  return taskManager->active().task() == m_task;
}

QVariant qtBaseTaskNode::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == GraphicsItemChange::ItemPositionHasChanged)
  {
    m_moveSignalTimer->start();
    Q_EMIT this->nodeMovedImmediate();
  }

  return QGraphicsItem::itemChange(change, value);
}

int qtBaseTaskNode::updateSize()
{
  this->prepareGeometryChange();

  Q_EMIT this->nodeResized();

  return 1;
}

} // namespace extension
} // namespace smtk
