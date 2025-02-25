//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtTaskPath.h"

#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtTaskEditor.h"

#include "smtk/task/State.h"
#include "smtk/task/Task.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>
#include <QPointer>
#include <QToolButton>
#include <QWidget>

namespace
{
QIcon renderStatusIcon(
  smtk::task::State state,
  int radius,
  smtk::extension::qtDiagramViewConfiguration* config)
{
  if (radius < 5)
  {
    radius = 5;
  }
  QPixmap pix(radius, radius);
  pix.fill(QColor(0, 0, 0, 0));

  QPainter painter(&pix);
  painter.setRenderHint(QPainter::Antialiasing, true);
  if (config)
  {
    painter.setBrush(QBrush(config->colorForState(state)));
  }
  else
  {
    painter.setBrush(QBrush(QColor("#000000")));
  }

  painter.drawEllipse(1, 1, radius - 2, radius - 2);
  painter.end();
  return QIcon(pix);
}

class qtTaskToolButton : public QToolButton
{
public:
  qtTaskToolButton(
    smtk::task::Task* t,
    smtk::extension::qtTaskPath* tpath,
    QWidget* parent = nullptr)
    : QToolButton(parent)
    , m_task(t)
    , m_taskPath(tpath)
  {
    // Set up an observer if we have been given a task
    if (m_task)
    {
      QPointer<qtTaskToolButton> tb(this);
      m_key = m_task->observers().insert(
        [tb](smtk::task::Task& t, smtk::task::State prev, smtk::task::State next) {
          (void)t;
          (void)prev;
          if (!tb)
          {
            return; // widget has been deleted
          }
          tb->setIcon(renderStatusIcon(
            next,
            tb->height() / 3,
            tb->m_taskPath->editor()->diagram()->diagramScene()->configuration()));
        });
    }
  }

  ~qtTaskToolButton() override
  {
    if (m_task)
    {
      m_task->observers().erase(m_key);
    }
  }
  smtk::task::Task* task() const { return m_task; }

protected:
  smtk::task::Task* m_task;
  smtk::task::Task::Observers::Key m_key;
  smtk::extension::qtTaskPath* m_taskPath;
};

} // namespace

namespace smtk
{
namespace extension
{

qtTaskPath::qtTaskPath(qtTaskEditor* editor, QWidget* parent)
  : Superclass(parent)
  , m_editor(editor)
  , m_config(editor->diagram()->diagramScene()->configuration())
{
  m_main = new QFrame();
  m_mainLayout = new QHBoxLayout();
  m_mainLayout->setContentsMargins(0, 0, 0, 0);
  m_mainLayout->addStretch();
  m_main->setLayout(m_mainLayout);
  this->setWidget(m_main);
  this->setWidgetResizable(true);
  this->setContentsMargins(0, 0, 0, 0);
  this->setFrameShape(QFrame::NoFrame);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto* tb = new qtTaskToolButton(nullptr, this, m_main);
  QIcon home(":/icons/diagram/house-solid.svg");
  tb->setIcon(home);
  tb->setObjectName("Home");
  m_mainLayout->insertWidget(0, tb);
  connect(tb, &QToolButton::released, this, &qtTaskPath::gotoRoot);
}

QToolButton* qtTaskPath::addTask(smtk::task::Task* t)
{
  if (t == nullptr)
  {
    return nullptr;
  }
  int i = m_mainLayout->count() - 1;
  if (i < 0)
  {
    i = 0;
  }
  auto* tb = new qtTaskToolButton(t, this, m_main);
  tb->setCheckable(true);
  tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  tb->setText(t->name().c_str());
  tb->setIcon(renderStatusIcon(t->state(), tb->height() / 3, m_config));
  m_mainLayout->insertWidget(i, tb);
  tb->setChecked(m_editor->manager()->active().task() == t);
  tb->setObjectName(t->name().c_str());
  connect(tb, &QToolButton::toggled, this, &qtTaskPath::updateActiveTask);
  return tb;
}

smtk::task::Task* qtTaskPath::lastTask() const
{
  // Get the last Task Tool Button - remember that the last item in the
  // layout is a stretch item which is why we subtract 2
  int i = m_mainLayout->count() - 2;
  if (i >= 0)
  {
    auto* tb = dynamic_cast<qtTaskToolButton*>(m_mainLayout->itemAt(i)->widget());
    if (tb)
    {
      return tb->task();
    }
  }
  return nullptr;
}

void qtTaskPath::setActiveTask(smtk::task::Task* task)
{
  int pathSize = m_mainLayout->count() - 2;

  if (task == nullptr)
  {
    // There is no active task set - make sure the last task is "off"
    if (pathSize <= 0)
    {
      return; // There are no tasks in the path
    }
    auto* tb = dynamic_cast<qtTaskToolButton*>(m_mainLayout->itemAt(pathSize)->widget());
    if (tb)
    {
      tb->blockSignals(true);
      tb->setChecked(false);
      tb->blockSignals(false);
    }
    return;
  }
  // Simple check 1 - see if the active task is tail of the path and that it still accepts/has children
  // or has internal ports
  auto* lt = this->lastTask();
  if ((lt == task) && this->includeInPath(task))
  {
    auto* lastButton = dynamic_cast<qtTaskToolButton*>(m_mainLayout->itemAt(pathSize)->widget());
    lastButton->blockSignals(true);
    lastButton->setChecked(true);
    lastButton->blockSignals(false);
    return;
  }
  // Make sure the tail of the path is not checked
  if (lt != nullptr)
  {
    auto* lastButton = dynamic_cast<qtTaskToolButton*>(m_mainLayout->itemAt(pathSize)->widget());
    lastButton->blockSignals(true);
    lastButton->setChecked(false);
    lastButton->blockSignals(false);
  }
  // Simple check 2 - see if the active task is a child of the last task
  if (lt == task->parent())
  {
    // If this new task accepts/has children or has internal ports we need to add it to the path
    if (this->includeInPath(task))
    {
      auto* tb = this->addTask(task);
      tb->blockSignals(true);
      tb->setChecked(true);
      tb->blockSignals(false);
    }
    return;
  }
  auto lineage = task->lineage();
  // Simplest method is to clear all of the current task related nodes and rebuild the
  // path - this could be optimized in the future if needed
  this->clearPath();
  for (std::size_t j = 0; j < lineage.size(); j++)
  {
    this->addTask(lineage[j]);
  }
  if (this->includeInPath(task))
  {
    auto* tb = this->addTask(task);
    tb->blockSignals(true);
    tb->setChecked(true);
    tb->blockSignals(false);
  }
}

void qtTaskPath::clearPath()
{
  int pathSize = m_mainLayout->count() - 2;
  for (int i = pathSize; i > 0; i--)
  {
    auto* item = m_mainLayout->takeAt(i);
    if (item)
    {
      item->widget()->deleteLater();
      delete item;
    }
  }
}

void qtTaskPath::gotoRoot()
{
  auto* currentLast = this->lastTask();
  this->clearPath();
  if (!m_editor->manager())
  {
    // It is not an error for there to be no task-manager.
    return;
  }
  m_editor->manager()->active().switchTo(nullptr);
  if (currentLast != nullptr)
  {
    m_editor->updateVisibility(currentLast, nullptr);
  }
}

void qtTaskPath::updateActiveTask(bool makeActive)
{
  if (!makeActive)
  {
    // unset the active task
    m_editor->manager()->active().switchTo(nullptr);
    return;
  }
  auto* tb = dynamic_cast<qtTaskToolButton*>(QObject::sender());
  if (tb)
  {
    m_editor->manager()->active().switchTo(tb->task());
  }
}

bool qtTaskPath::includeInPath(const smtk::task::Task* task) const
{
  return (task->hasChildren() || task->hasInternalPorts() || task->canAcceptWorklets());
}

} // namespace extension
} // namespace smtk
