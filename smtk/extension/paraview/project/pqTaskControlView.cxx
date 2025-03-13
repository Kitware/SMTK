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
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceRepresentation.h"
#include "smtk/extension/paraview/project/Utility.h"

#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/Manager.h"

#include "smtk/project/Manager.h"
#include "smtk/task/Active.h"
#include "smtk/task/Manager.h"

#include "smtk/common/Managers.h"

#include "smtk/io/Logger.h"

#include "pqApplicationCore.h"
#include "pqSMAdaptor.h"
#include "pqView.h"

#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

#include <QFile>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSize>
#include <QSlider>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>

using namespace smtk::paraview;
using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{
namespace // anonymous
{

// Return true if \a entry references a resource (as opposed to a component).
//
// This is used by buttons in the view to determine whether to toggle visibility of
// the entire representation or just a block within the representation.
bool representationObjectMapContainsResource(const RepresentationObjectMap::value_type& entry)
{
  auto pvDRep = dynamic_cast<pqDataRepresentation*>(entry.first);
  auto pvRsrc = pvDRep ? dynamic_cast<pqSMTKResource*>(pvDRep->getInput()) : nullptr;
  if (!pvRsrc)
  {
    return false;
  }
  auto it = entry.second.find(pvRsrc->getResource().get());
  return (it != entry.second.end() || entry.second.find(nullptr) != entry.second.end());
}

} // anonymous namespace

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
  }

  void addTaskDescription(
    QLayout* layout,
    smtk::task::Task* task,
    const smtk::view::Configuration::Component& spec)
  {
    (void)spec; // Currently no options to look up.
    auto* description = new QLabel();
    smtk::task::Task::InformationOptions opt;
    description->setObjectName("taskDescription");
    description->setText(QString::fromStdString(task ? task->information(opt) : ""));
    description->setWordWrap(true); // Allow line breaks so panel is not forced to be crazy wide.
    layout->addWidget(description);
    ++this->numChildren;
  }

  void addTaskStatus(
    QFormLayout* formLayout,
    smtk::task::Task* task,
    const smtk::view::Configuration::Component& spec)
  {
    auto* label = new QLabel();
    label->setObjectName("taskName");
    auto txt = QString::fromStdString(task ? task->name() : "");
    if (spec.attributeAsBool("ReturnToDiagram"))
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
    ++this->numChildren;
  }

  void addRepresentationControl(
    QFormLayout* formLayout,
    smtk::task::Task* task,
    const smtk::view::Configuration::Component& spec)
  {
    (void)task;
    auto* field = new QHBoxLayout;
    std::string name;
    if (!spec.attribute("Name", name))
    {
      smtkWarningMacro(smtk::io::Logger::instance(), "RepresentationControl without name.");
      name = "Unknown";
    }
    std::string label;
    if (!spec.attribute("Label", label))
    {
      smtkWarningMacro(smtk::io::Logger::instance(), "RepresentationControl without label.");
      label = name;
    }
    for (const auto& controlSpec : spec.children())
    {
      if (controlSpec.name() == "Control")
      {
        if (controlSpec.attributeAsString("Type") == "Visibility")
        {
          auto* ctrl = new QPushButton;
          ctrl->setObjectName("visibility");
          ctrl->setText("Visibility");
          ctrl->setCheckable(true);
          ctrl->setToolTip("Toggle visibility");
          bool initiallyVisible{ true };
          controlSpec.attributeAsBool("InitiallyVisible", initiallyVisible);
          ctrl->setChecked(initiallyVisible);
          m_representationSpecs[name] = spec;
          QObject::connect(ctrl, &QPushButton::toggled, [this, name](bool toggled) {
            m_view->toggleVisibility(name, toggled);
          });
          field->addWidget(ctrl);
          // Now force state to match button upon view construction.
          m_view->toggleVisibility(name, initiallyVisible);
        }
        else if (controlSpec.attributeAsString("Type") == "Opacity")
        {
          auto* ctrl = new QSlider;
          ctrl->setObjectName("opacity");
          ctrl->setMinimum(0);
          ctrl->setMaximum(255);
          ctrl->setPageStep(16);
          ctrl->setOrientation(Qt::Horizontal);
          int initialValue{ 255 };
          controlSpec.attributeAsInt("InitialValue", initialValue);
          ctrl->setValue(initialValue);
          ctrl->setToolTip("Adjust opacity");
          m_representationSpecs[name] = spec;
          QObject::connect(ctrl, &QSlider::valueChanged, [this, name](int value) {
            m_view->updateOpacity(name, value / 255.0);
          });
          field->addWidget(ctrl);
          // Now force state to match slider upon view construction.
          m_view->updateOpacity(name, initialValue / 255.);
        }
      }
    }
    formLayout->addRow(QString::fromStdString(label), field);
    ++this->numChildren;
  }

  void addOperationControl(
    QLayout* layout,
    smtk::task::Task* task,
    const smtk::view::Configuration::Component& spec)
  {
    (void)layout;
    (void)task;
    (void)spec;
    ++this->numChildren;
  }

  pqTaskControlView* m_view{ nullptr };
  smtk::task::Manager* m_currentTaskManager{ nullptr };
  smtk::task::Task* m_currentTask{ nullptr };
  smtk::project::Observers::Key m_projectObserverKey;
  smtk::task::Active::Observers::Key m_activeTaskObserverKey;
  std::unordered_map<smtk::string::Token, smtk::view::Configuration::Component>
    m_representationSpecs;
  int numChildren{ 0 };
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
                                                            "RepresentationControl"_token };

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
    case "ActiveTaskDescription"_hash:
      this->addTaskDescription(layout, task, childSpec);
      break;
    case "ActiveTaskStatus"_hash:
      this->addTaskStatus(formLayout, task, childSpec);
      break;
    case "RepresentationControl"_hash:
      this->addRepresentationControl(formLayout, task, childSpec);
      break;
    case "OperationControl"_hash:
      this->addOperationControl(layout, task, childSpec);
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
  return m_p->numChildren == 0;
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

void pqTaskControlView::toggleVisibility(const std::string& name, bool show)
{
  if (m_p->m_currentTaskManager)
  {
    auto objMap = m_p->m_currentTaskManager->workflowObjects(
      m_p->m_representationSpecs[name], m_p->m_currentTask);
    auto repMap = smtk::paraview::representationsOfObjects(objMap);
    for (const auto& entry : repMap)
    {
      bool needRender = false;
      // Toggle the whole representation's visibility if either
      // (1) entry.second contains a null pointer (indicating the resource itself
      //     is supposed to have its visibility changed) or
      // (2) \a show is set to true (indicating that some components that were
      //     hidden should now be shown – which cannot happen without the
      //     resource-representation visibility being set to true).
      if (show || representationObjectMapContainsResource(entry))
      {
        if (entry.first->isVisible() != show)
        {
          entry.first->setVisible(show);
          needRender = true;
        }
      }
      auto pvDRep = dynamic_cast<pqSMTKResourceRepresentation*>(entry.first);
      if (!pvDRep)
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "Could not downcast representation " << entry.first
                                               << " to an SMTK resource representation.");
        continue;
      }
      // Now, tell the representation to toggle individual component visibilities.
      for (const auto& obj : entry.second)
      {
        if (!obj)
        {
          continue;
        }
        if (auto comp = dynamic_cast<smtk::resource::Component*>(obj)->shared_from_this())
        {
          needRender |= pvDRep->setVisibility(comp, show);
        }
      }
      if (needRender)
      {
        entry.first->getView()->render();
      }
    }
  }
}

void pqTaskControlView::updateOpacity(const std::string& name, double opacity)
{
  if (m_p->m_currentTaskManager)
  {
    auto objMap = m_p->m_currentTaskManager->workflowObjects(
      m_p->m_representationSpecs[name], m_p->m_currentTask);
    auto repMap = smtk::paraview::representationsOfObjects(objMap);
    for (const auto& entry : repMap)
    {
      bool needRender = false;
      // Toggle the whole representation's visibility if either
      // (1) entry.second contains a null pointer (indicating the resource itself
      //     is supposed to have its visibility changed) or
      // (2) \a show is set to true (indicating that some components that were
      //     hidden should now be shown – which cannot happen without the
      //     resource-representation visibility being set to true).
      if (representationObjectMapContainsResource(entry))
      {
        pqSMAdaptor::setElementProperty(entry.first->getProxy()->GetProperty("Opacity"), opacity);
        entry.first->getProxy()->UpdateVTKObjects();
        needRender = true;
      }
      // TODO: Handle per-block opacity settings for components.
      //       Iterate over entry.second, find component inside the
      //       pqSMTKRepresentation and set per-block opacity.
      if (needRender)
      {
        entry.first->getView()->render();
      }
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
  m_p->m_currentTaskManager = task ? task->manager() : nullptr;
  m_p->m_currentTask = task;

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
