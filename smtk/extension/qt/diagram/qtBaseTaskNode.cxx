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

#include <cassert>

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
  assert(dynamic_cast<qtTaskEditor*>(generator));
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

void qtBaseTaskNode::updateTaskState(smtk::task::State prev, smtk::task::State next, bool active)
{
  (void)prev;
  (void)next;
  (void)active;
  // Update the tool tip with diagnostic information
  smtk::task::Task::InformationOptions opt;
  opt.m_includeTitle = false;
  this->setToolTip(QString::fromStdString(m_task->information(opt)));
}

void qtBaseTaskNode::dataUpdated()
{
  this->Superclass::dataUpdated();

  // Update the tool tip with diagnostic information
  smtk::task::Task::InformationOptions opt;
  opt.m_includeTitle = false;
  this->setToolTip(QString::fromStdString(m_task->information(opt)));
}

} // namespace extension
} // namespace smtk
