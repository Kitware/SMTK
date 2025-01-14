//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtBaseTaskNode.h"

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

qtBaseTaskNode::qtBaseTaskNode(
  qtDiagramGenerator* generator,
  smtk::task::Task* task,
  QGraphicsItem* parent)
  : Superclass(generator, task, parent)
  , m_task(task)
{
}

qtBaseTaskNode::~qtBaseTaskNode() = default;

smtk::common::UUID qtBaseTaskNode::nodeId() const
{
  return m_task->id();
}

smtk::resource::PersistentObject* qtBaseTaskNode::object() const
{
  return m_task;
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

void qtBaseTaskNode::setSnapPorts(bool val)
{
  m_snapPorts = val;
}
} // namespace extension
} // namespace smtk
