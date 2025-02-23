//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/project/pqTaskControlView.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKDiagramPanel.h"

#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/Manager.h"

#include "smtk/project/Manager.h"
#include "smtk/task/Active.h"
#include "smtk/task/Manager.h"

#include "smtk/common/Managers.h"

#include "smtk/io/Logger.h"

#include "pqApplicationCore.h"

#include <QFile>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSize>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{

/// Private storage for a task-control views.
class pqTaskControlView::Internal
{
public:
  Internal(pqTaskControlView* self)
    : m_view(self)
  {
  }

  /// Add a widget for a \a childSpec.
  ///
  /// If \a layout is non-null and appropriate for the \a childSpec,
  /// the current \a layout and \a widget are used. Otherwise, a new
  /// layout is created.
  bool addChildItem(
    const smtk::view::Configuration::Component& childSpec,
    QLayout*& layout,
    QWidget*& parent,
    int& childNum,
    smtk::task::Task* task);

  /// Called when the active task changes.
  void activeTaskChange(smtk::task::Task* prev, smtk::task::Task* next)
  {
    (void)prev;
    (void)next;
    m_view->updateWithActiveTask(next);
    // m_view->buildUI();
  }

  pqTaskControlView* m_view{ nullptr };
  smtk::project::Observers::Key m_projectObserverKey;
  smtk::task::Active::Observers::Key m_activeTaskObserverKey;
};

bool pqTaskControlView::Internal::addChildItem(
  const smtk::view::Configuration::Component& childSpec,
  QLayout*& layout,
  QWidget*& parent,
  int& childNum,
  smtk::task::Task* task)
{
  bool needLayout = !layout;
  smtk::string::Token childType = childSpec.name();
  static std::unordered_set<smtk::string::Token> formItems{ "ActiveTaskStatus"_token,
                                                            "ObjectControl"_token };

  bool isFormItem = (formItems.find(childType) != formItems.end());
  auto* formLayout = dynamic_cast<QFormLayout*>(layout);
  needLayout |= (isFormItem && !formLayout) || (!isFormItem && formLayout);
  if (needLayout)
  {
    parent = new QWidget();
    std::ostringstream pname;
    pname << "TaskControlWidget" << childNum++;
    parent->setObjectName(pname.str().c_str());
    if (isFormItem)
    {
      formLayout = new QFormLayout(parent);
      layout = formLayout;
    }
    else
    {
      layout = new QVBoxLayout(parent);
    }
    m_view->widget()->layout()->addWidget(parent);
  }
  switch (childType.id())
  {
    case "ActiveTaskStatus"_hash:
    {
      auto* label = new QLabel();
      label->setObjectName("taskName");
      auto txt = QString::fromStdString(task ? task->name() : "");
      if (childSpec.attributeAsBool("ReturnToDiagram"))
      {
        txt += QString::fromStdString("<a href=\"#returnToDiagram\">⤴</a>");
      }
      label->setText(txt);
      label->setOpenExternalLinks(false);
      QObject::connect(label, &QLabel::linkActivated, m_view, &pqTaskControlView::returnToDiagram);
      auto* ctrl = new QPushButton();
      ctrl->setObjectName("taskCompleter");
      ctrl->setText(task ? (task->isCompleted() ? "Completed" : "Complete") : "Complete");
      ctrl->setCheckable(true);
      ctrl->setChecked(task ? task->isCompleted() : false);
      ctrl->setEnabled(task ? task->state() >= smtk::task::State::Completable : false);
      QObject::connect(
        ctrl, &QAbstractButton::toggled, m_view, &pqTaskControlView::updateTaskCompletion);
      formLayout->addRow(label, ctrl);
    }
    break;
    case "ActiveTaskDescription"_hash:
    {
      auto* description = new QLabel();
      smtk::task::Task::InformationOptions opt;
      description->setObjectName("taskDescription");
      description->setText(QString::fromStdString(task ? task->information(opt) : ""));
      description->setWordWrap(true); // Allow line breaks so panel is not forced to be crazy wide.
      layout->addWidget(description);
    }
    break;
    case "ObjectControl"_hash:
      break;
    case "OperationControl"_hash:
      break;
    default:
      return false;
      break;
  }
  return true;
}

qtBaseView* pqTaskControlView::createViewWidget(const smtk::view::Information& info)
{
  if (!qtBaseView::validateInformation(info))
  {
    return nullptr; // \a info is not suitable for this View
  }
  auto* view = new pqTaskControlView(info);
  view->buildUI();
  return view;
}

pqTaskControlView::pqTaskControlView(const smtk::view::Information& info)
  : qtBaseView(info)
{
  m_p = new Internal(this);
  // auto viewConfig = this->configuration();
  // if (viewConfig)
  // {
  // }
}

pqTaskControlView::~pqTaskControlView()
{
  delete m_p;
}

bool pqTaskControlView::isEmpty() const
{
  // TODO
  return false;
}

bool pqTaskControlView::isValid() const
{
  // This view is always valid for now.
  return true;
}

void pqTaskControlView::updateUI()
{
  auto projMgr = this->uiManager()->managers().get<smtk::project::Manager::Ptr>();
  auto project = projMgr ? *projMgr->projects().begin() : nullptr;
  auto* taskMgr = project ? &project->taskManager() : nullptr;
  auto* activeTask = taskMgr ? taskMgr->active().task() : nullptr;
  this->updateWithActiveTask(activeTask);
}

void pqTaskControlView::showAdvanceLevelOverlay(bool show)
{
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

void pqTaskControlView::onShowCategory()
{
  this->Superclass::onShowCategory();
}

void pqTaskControlView::returnToDiagram()
{
  auto* pqCore = pqApplicationCore::instance();
  if (!pqCore)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Cannot return to diagram panel as there is no application core.");
    return;
  }
  auto* panel = dynamic_cast<pqSMTKDiagramPanel*>(pqCore->manager("smtk task panel"));
  if (!panel)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Cannot return to diagram panel as there is no diagram panel.");
    return;
  }
  panel->focusPanel();
}

void pqTaskControlView::updateTaskCompletion(bool completed)
{
  auto projMgr = this->uiManager()->managers().get<smtk::project::Manager::Ptr>();
  auto project = projMgr ? *projMgr->projects().begin() : nullptr;
  auto* taskMgr = project ? &project->taskManager() : nullptr;
  if (taskMgr)
  {
    auto* task = taskMgr->active().task();
    if (task && task->state() >= smtk::task::State::Completable)
    {
      QTimer::singleShot(0, [this, task, completed]() {
        if (task->markCompleted(completed) && completed)
        {
          this->returnToDiagram();
        }
      });
    }
  }
}

void pqTaskControlView::buildUI()
{
  this->createWidget();

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());
  if (!parentlayout)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No parent layout for task control view.");
    return;
  }

  if (!this->isTopLevel())
  {
    parentlayout->setAlignment(Qt::AlignTop);
    parentlayout->addWidget(this->Widget);
    return;
  }
  // XXX TODO.
}

void pqTaskControlView::createWidget()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view)
  {
    return;
  }
  // If we have a pre-existing widget, destroy it as we are about to recreate it.
  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());
  if (this->Widget)
  {
    if (parentlayout)
    {
      parentlayout->removeWidget(this->Widget);
    }
    delete this->Widget;
  }

  this->Widget = new QFrame(this->parentWidget());
  this->Widget->setObjectName(view->name().c_str());

  // Create the layout for the frame
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);

  // Start observing changes in the active task:
  auto projMgr = this->uiManager()->managers().get<smtk::project::Manager::Ptr>();
  auto project = projMgr ? *projMgr->projects().begin() : nullptr;
  auto* taskMgr = project ? &project->taskManager() : nullptr;
  if (taskMgr)
  {
    m_p->m_activeTaskObserverKey = taskMgr->active().observers().insert(
      [&](smtk::task::Task* prev, smtk::task::Task* next) { m_p->activeTaskChange(prev, next); },
      0,
      false,
      "TaskControlView");
  }
  else
  {
    m_p->m_activeTaskObserverKey.release();
  }

  this->updateUI();
}

void pqTaskControlView::updateWithActiveTask(smtk::task::Task* task)
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view)
  {
    return;
  }
  // Remove all previous children (if any).
  if (this->widget())
  {
    for (auto* child :
         this->widget()->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
    {
      delete child;
    }
  }

  // Now process all of this view's entries
  QLayout* activeLayout = nullptr;
  QWidget* activeWidget = nullptr;
  int childNum = 0;
  for (const auto& child : view->details().children())
  {
    m_p->addChildItem(child, activeLayout, activeWidget, childNum, task);
  }
}

} // namespace extension
} // namespace smtk
