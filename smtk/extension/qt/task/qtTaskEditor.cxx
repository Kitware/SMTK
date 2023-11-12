//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/qtTaskEditor.h"

#include "smtk/common/Managers.h"
#include "smtk/common/json/jsonUUID.h"

#include "smtk/extension/qt/qtManager.h"

#include "smtk/extension/qt/task/PanelConfiguration_cpp.h"
#include "smtk/extension/qt/task/qtDefaultTaskNode.h"
#include "smtk/extension/qt/task/qtTaskArc.h"
#include "smtk/extension/qt/task/qtTaskScene.h"
#include "smtk/extension/qt/task/qtTaskView.h"
#include "smtk/extension/qt/task/qtTaskViewConfiguration.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/json/jsonView.h"

#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"

#include "smtk/task/Active.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/io/Logger.h"

#include "nlohmann/json.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPointF>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

// Uncomment to get debug printouts from workflow events.
// #define SMTK_DBG_WORKFLOWS 1

namespace smtk
{
namespace extension
{

class qtTaskEditor::Internal
{
public:
  using Task = smtk::task::Task;

  Internal(qtTaskEditor* self, const smtk::view::Information& info)
  {
    (void)info;
    m_self = self;
    m_scene = new qtTaskScene(m_self);
    m_widget = new qtTaskView(m_scene, m_self);
    auto* layout = new QVBoxLayout;
    layout->setObjectName("taskEditor");
    m_self->Widget = m_widget;
    m_self->Widget->setLayout(layout);
    if (info.configuration())
    {
      m_scene->setConfiguration(new qtTaskViewConfiguration(*info.configuration()));
    }
  }

  void displayTaskManager(smtk::task::Manager* taskManager)
  {
    if (m_taskManager == taskManager)
    {
      return;
    }
    this->removeObservers();
    this->clear();
    m_taskManager = taskManager;

    if (!m_taskManager)
    {
      return;
    }

    this->installObservers();

    // TODO: Fetch UI state?
    QTimer::singleShot(0, [this]() {
      bool modified = this->computeNodeLayout();
      m_widget->ensureVisible(m_scene->sceneRect());

      // After layout complete, connect nodeMoved signal
      for (const auto& entry : m_taskIndex)
      {
        qtBaseTaskNode* taskNode = entry.second;
        QObject::connect(
          taskNode, &qtBaseTaskNode::nodeMoved, m_self, &qtTaskEditor::onNodeGeometryChanged);
      }

      if (modified)
      {
        m_self->onNodeGeometryChanged();
      }
    });
  }

  // Returns true if UI configuration was modified
  bool computeNodeLayout()
  {
    if (m_taskIndex.empty() || (!m_layoutMap.empty()))
    {
      return false;
    }

    // If geometry not configured, call the scenes method to layout nodes
    std::unordered_set<qtBaseTaskNode*> nodes;
    std::unordered_set<qtTaskArc*> arcs;
    for (const auto& entry : m_taskIndex)
    {
      nodes.insert(entry.second);
    }
    for (const auto& entry : m_arcIndex)
    {
      for (const auto& arc : entry.second)
      {
        arcs.insert(arc);
      }
    }
    return (m_scene->computeLayout(nodes, arcs) == 1);
  }

  void removeObservers()
  {
    // Reset task-manager observers
    if (m_taskManager)
    {
      m_taskManager->adaptorObservers().erase(m_adaptorObserverKey);
      m_taskManager->taskObservers().erase(m_instanceObserverKey);
      m_taskManager->workflowObservers().erase(m_workflowObserverKey);
      m_taskManager->active().observers().erase(m_activeObserverKey);
    }
    else
    {
      m_adaptorObserverKey.release();
      m_instanceObserverKey.release();
      m_workflowObserverKey.release();
      m_activeObserverKey.release();
    }
  }

  void installObservers()
  {
    if (!m_taskManager)
    {
      return;
    }

    QPointer<qtTaskEditor> parent(m_self);
    m_instanceObserverKey = m_taskManager->taskObservers().insert(
      [this,
       parent](smtk::common::InstanceEvent event, const std::shared_ptr<smtk::task::Task>& task) {
        if (!parent)
        {
          return;
        }
        switch (event)
        {
          case smtk::common::InstanceEvent::Managed:
          {
            // std::cout << "Add task instance node " << task << " " << task->name() << "\n";
            // Determine the qtTaskNode constructed needed for this Task.
            std::string taskNodeType = "smtk::extension::qtDefaultTaskNode";
            for (const auto& style : task->style())
            {
              const auto& styleConfig = m_taskManager->getStyle(style);
              auto it = styleConfig.find("task-panel");
              if (it == styleConfig.end())
              {
                continue;
              }
              auto it1 = it->find("node-class");
              if (it1 != it->end())
              {
                taskNodeType = it1->get<std::string>();
                break;
              }
            }

            auto qtmgr = m_taskManager->managers()->get<smtk::extension::qtManager::Ptr>();
            auto* tnode =
              qtmgr->taskNodeFactory()
                .createFromName(taskNodeType, m_scene, task.get(), (QGraphicsItem*)nullptr)
                .release();
            if (!tnode)
            {
              tnode = new qtDefaultTaskNode(m_scene, task.get());
              smtkWarningMacro(
                smtk::io::Logger::instance(),
                "Could not find Task Node Class " << taskNodeType
                                                  << " creating Default Task Node!");
            }

            m_taskIndex[task.get()] = tnode;
            // If a layout map has been set, use it to position the Task Node
            if (!m_layoutMap.empty())
            {
              // Does this node have a location in the layout?
              auto it = m_layoutMap.find(task->id());
              if (it != m_layoutMap.end())
              {
                tnode->setPos(it->second.first, it->second.second);
              }
            }
          }
          break;
          case smtk::common::InstanceEvent::Unmanaged:
          {
            // std::cout << "Remove task instance " << task << " " << task->name() << "\n";
            auto it = m_taskIndex.find(task.get());
            if (it != m_taskIndex.end())
            {
              this->removeArcsAttachedTo(it->second);
              delete it->second;
              m_taskIndex.erase(it);
            }
          }
          break;
          case smtk::common::InstanceEvent::Modified:
          {
            // std::cout << "Update task " << task << " " << task->name() << "\n";
            auto it = m_taskIndex.find(task.get());
            if (it != m_taskIndex.end())
            {
              // Update name, update arcs, etc.
              it->second->updateToMatchModifiedTask();
            }
          }
          break;
        }
      },
      "qtTaskEditor watching task instances.");
    // Observe task-manager's taskInstances() and active() objects.
    m_workflowObserverKey = m_taskManager->workflowObservers().insert(
      [this, parent](
        const std::set<smtk::task::Task*>& workflow,
        smtk::task::WorkflowEvent workflowEvent,
        smtk::task::Task* task) {
        (void)workflow;
        (void)task;
        if (!parent)
        {
          return;
        }
        switch (workflowEvent)
        {
          case smtk::task::WorkflowEvent::Created:
#ifdef SMTK_DBG_WORKFLOWS
            std::cout << "Workflow created, task " << task << ":\n";
#endif
            break;
          case smtk::task::WorkflowEvent::TaskAdded:
#ifdef SMTK_DBG_WORKFLOWS
            std::cout << "Add task " << task << " with " << workflow.size() << " tasks in flow:\n";
#endif
            // new qtBaseTaskNode(m_scene, task);
            break;
          case smtk::task::WorkflowEvent::TaskRemoved:
#ifdef SMTK_DBG_WORKFLOWS
            std::cout << "Task " << task << " removed:\n";
#endif
            break;
          case smtk::task::WorkflowEvent::Destroyed:
#ifdef SMTK_DBG_WORKFLOWS
            std::cout << "Workflow destroyed, task " << task << "\n";
#endif
            break;
          case smtk::task::WorkflowEvent::Resuming:
            this->resetArcs(qtTaskArc::ArcType::Dependency);
#ifdef SMTK_DBG_WORKFLOWS
            std::cout << "Resuming w " << workflow.size() << " tasks in flow:\n";
#endif
            break;
        }
#ifdef SMTK_DBG_WORKFLOWS
        for (const auto& wtask : workflow)
        {
          std::cout << "  " << wtask << " " << wtask->name() << "\n";
        }
#endif
      },
      "qtTaskEditor watching task workflows.");

    m_adaptorObserverKey = m_taskManager->adaptorObservers().insert(
      [this, parent](
        smtk::common::InstanceEvent event, const std::shared_ptr<smtk::task::Adaptor>& adaptor) {
        if (!parent)
        {
          return;
        }
        auto fromIt = m_taskIndex.find(adaptor->from());
        auto toIt = m_taskIndex.find(adaptor->to());
        if (fromIt == m_taskIndex.end() || toIt == m_taskIndex.end())
        {
          smtkWarningMacro(
            smtk::io::Logger::instance(),
            "An adaptor's arc could not be "
              << (event == smtk::common::InstanceEvent::Managed ? "added" : "removed")
              << " because the nodes don't exist yet.");
          return;
        }
        switch (event)
        {
          case smtk::common::InstanceEvent::Managed:
            m_arcIndex[fromIt->second].insert(
              new qtTaskArc(m_scene, fromIt->second, toIt->second, adaptor.get()));
            break;
          case smtk::common::InstanceEvent::Unmanaged:
          {
            qtTaskArc* match = nullptr;
            auto it = m_arcIndex.find(fromIt->second);
            if (it != m_arcIndex.end())
            {
              for (const auto& arcItem : it->second)
              {
                if (arcItem->adaptor() == adaptor.get())
                {
                  match = arcItem;
                  m_arcIndex.erase(it);
                  break;
                }
              }
            }
            if (!match)
            {
              smtkWarningMacro(
                smtk::io::Logger::instance(),
                "An adaptor's arc could not be removed because the arc don't exist or is "
                "improperly indexed!");
              return;
            }
            delete match;
          }
          break;
          case smtk::common::InstanceEvent::Modified:
            // TODO: Update modified arc? What could change? from/to?
            break;
        }
      },
      "qtTaskEditor watching task adaptors.");

    m_activeObserverKey = m_taskManager->active().observers().insert(
      [this, parent](smtk::task::Task* prev, smtk::task::Task* next) {
        if (!parent)
        {
          return;
        }
        // std::cout << "Switch active task from " << prev << " to " << next << "\n";
        auto prevNode = m_taskIndex.find(prev);
        auto nextNode = m_taskIndex.find(next);
        if (prevNode != m_taskIndex.end())
        {
          auto prevState = prev->state();
          prevNode->second->updateTaskState(prevState, prevState, false);
          prevNode->second->setOutlineStyle(qtBaseTaskNode::OutlineStyle::Normal);
        }
        if (nextNode != m_taskIndex.end())
        {
          auto nextState = next->state();
          nextNode->second->updateTaskState(nextState, nextState, true);
          nextNode->second->setOutlineStyle(qtBaseTaskNode::OutlineStyle::Active);
        }
      },
      0,
      true,
      "qtTaskEditor watching active task.");
  }

  void removeArcsAttachedTo(qtBaseTaskNode* node)
  {
    // Examine node->task()->dependencies() for upstream arcs
    auto deps = node->task()->dependencies();
    // std::cout << "  Task " << node->task() << " has " << deps.size() << " dependencies.\n";
    for (const auto& predecessor : deps)
    {
      auto predIt = m_taskIndex.find(predecessor.get());
      if (predIt == m_taskIndex.end())
      {
        continue;
      }
      auto arcIt = m_arcIndex.find(predIt->second);
      if (arcIt != m_arcIndex.end())
      {
        std::unordered_set<qtTaskArc*> toErase;
        for (const auto& arcItem : arcIt->second)
        {
          if (arcItem->successor() == node)
          {
            toErase.insert(arcItem);
            delete arcItem;
          }
        }
        for (const auto& entry : toErase)
        {
          arcIt->second.erase(entry);
        }
      }
    }
    // Examine m_arcIndex[node] for downstream arcs
    auto arcIt = m_arcIndex.find(node);
    if (arcIt != m_arcIndex.end())
    {
      for (const auto& arcItem : arcIt->second)
      {
        delete arcItem;
      }
      m_arcIndex.erase(node);
    }
  }

  void resetArcs(qtTaskArc::ArcType arcsToReset)
  {
    // Erase all arcs and repopulate.
    for (auto& entry : m_arcIndex)
    {
      std::unordered_set<qtTaskArc*> toErase;
      for (const auto& arc : entry.second)
      {
        if (arc->arcType() == arcsToReset)
        {
          toErase.insert(arc);
          delete arc;
        }
      }
      for (const auto& arcToErase : toErase)
      {
        entry.second.erase(arcToErase);
      }
    }

    m_taskManager->taskInstances().visit([this](const std::shared_ptr<Task>& successor) {
      auto succIt = m_taskIndex.find(successor.get());
      if (succIt == m_taskIndex.end())
      {
        // std::cout << "  Skip tasks " << successor.get() << "\n";
        return smtk::common::Visit::Continue;
      }
      auto deps = successor->dependencies();
      // std::cout << "  Task " << successor << " has " << deps.size() << " dependencies.\n";
      for (const auto& predecessor : deps)
      {
        // std::cout << "    Task " << predecessor << " is a dependency.\n";
        auto predIt = m_taskIndex.find(predecessor.get());
        if (predIt == m_taskIndex.end())
        {
          // std::cout << "  Skip task " << predecessor.get() << "\n";
          continue;
        }
        m_arcIndex[predIt->second].insert(new qtTaskArc(m_scene, predIt->second, succIt->second));
      }
      return smtk::common::Visit::Continue;
    });
  }

  void clear()
  {
    m_scene->clear();
    m_taskIndex.clear();
    m_arcIndex.clear();
  }

  bool configure(const nlohmann::json& data)
  {
    m_layoutMap = data.get<std::unordered_map<smtk::common::UUID, std::pair<double, double>>>();
    return true;
  }

  nlohmann::json configuration()
  {
    nlohmann::json config;
    // Need to update the layout map
    m_layoutMap.clear();
    for (const auto& entry : m_taskIndex)
    {
      auto* node = entry.second;
      auto* task = entry.first;
      auto qpoint = node->pos();
      std::pair<double, double> pos(qpoint.x(), qpoint.y());
      m_layoutMap[task->id()] = pos;
    }
    config = m_layoutMap;
    return config;
  }

  qtTaskEditor* m_self;
  smtk::task::Manager* m_taskManager{ nullptr };
  qtTaskScene* m_scene{ nullptr };
  qtTaskView* m_widget{ nullptr };
  smtk::task::TaskManagerAdaptorObservers::Key m_adaptorObserverKey;
  smtk::task::TaskManagerWorkflowObservers::Key m_workflowObserverKey;
  smtk::task::TaskManagerTaskObservers::Key m_instanceObserverKey;
  smtk::task::Active::Observers::Key m_activeObserverKey;
  std::unordered_map<Task*, qtBaseTaskNode*> m_taskIndex;
  std::unordered_map<qtBaseTaskNode*, std::unordered_set<qtTaskArc*>>
    m_arcIndex; // Arcs grouped by their predecessor task.
  std::unordered_map<smtk::common::UUID, std::pair<double, double>> m_layoutMap;
};

qtTaskEditor::qtTaskEditor(const smtk::view::Information& info)
  : qtBaseView(info)
  , m_p(new Internal(this, info))
{
}

qtTaskEditor::~qtTaskEditor() = default;

qtBaseView* qtTaskEditor::createViewWidget(const smtk::view::Information& info)
{
  qtTaskEditor* editor = new qtTaskEditor(info);
  // editor->buildUI();
  return editor;
}

void qtTaskEditor::displayProject(const std::shared_ptr<smtk::project::Project>& project)
{
  if (project)
  {
    this->displayTaskManager(&project->taskManager());
    // TODO: Observe project's manager and clear this view if a different project is loaded.
  }
  else
  {
    // TODO: Unobserve any project manager
    this->displayTaskManager(nullptr);
  }
}

void qtTaskEditor::displayTaskManager(smtk::task::Manager* taskManager)
{
  m_p->displayTaskManager(taskManager);
}

qtTaskScene* qtTaskEditor::taskScene() const
{
  return m_p->m_scene;
}

qtTaskView* qtTaskEditor::taskWidget() const
{
  return m_p->m_widget;
}

qtBaseTaskNode* qtTaskEditor::findNode(smtk::task::Task* task) const
{
  auto it = m_p->m_taskIndex.find(task);
  if (it == m_p->m_taskIndex.end())
  {
    return nullptr;
  }
  return it->second;
}

std::shared_ptr<smtk::view::Configuration> qtTaskEditor::defaultConfiguration()
{
  std::shared_ptr<smtk::view::Configuration> result;
  auto jsonConfig = nlohmann::json::parse(taskPanelConfiguration())[0];
  result = jsonConfig;
  return result;
}

nlohmann::json qtTaskEditor::uiStateForTask(const smtk::task::Task* task) const
{
  auto iter = m_p->m_taskIndex.find(const_cast<smtk::task::Task*>(task));
  if (iter == m_p->m_taskIndex.end())
  {
    return nlohmann::json();
  }

  qtBaseTaskNode* taskNode = iter->second;
  QPointF pos = taskNode->scenePos();
  nlohmann::json jUI = nlohmann::json::object();
  jUI["position"] = { pos.x(), pos.y() };
  return jUI;
}

bool qtTaskEditor::addWorklet(const std::string& workletName, std::array<double, 2> location)
{
  if (!m_p->m_taskManager)
  {
    return false;
  }
  auto worklet = m_p->m_taskManager->gallery().find(workletName);
  if (!worklet)
  {
    // No such worklet?!
    return false;
  }
  auto opMgr = m_p->m_taskManager->managers()->get<smtk::operation::Manager::Ptr>();
  auto op = opMgr->create(worklet->operationName());
  if (!op)
  {
    return false;
  }
  if (!op->parameters()->associate(worklet))
  {
    return false;
  }
  if (!op->ableToOperate())
  {
    return false;
  }
  // TODO: Add location to operation parameters
  (void)location;
  opMgr->launchers()(op);
  return true;
}

bool qtTaskEditor::configure(const nlohmann::json& data)
{
  return m_p->configure(data);
}

nlohmann::json qtTaskEditor::configuration()
{
  return m_p->configuration();
}

void qtTaskEditor::onNodeGeometryChanged()
{
#if 0
  auto managers = m_p->m_taskManager->managers();
  if (managers == nullptr)
  {
    return;
  }

  auto* taskNode = dynamic_cast<qtBaseTaskNode*>(this->sender());
  if (taskNode != nullptr)
  {
    // Check if modified
    smtk::string::Token classToken = this->typeName();
    nlohmann::json origData = m_p->m_taskManager->uiState().getData(classToken, taskNode->task());
    nlohmann::json newData = this->uiStateForTask(taskNode->task());
    if (origData == newData)
    {
      return;
    }
  }

  // Set project's modified flag
  auto projectManager = managers->get<smtk::project::Manager::Ptr>();
  if (projectManager == nullptr)
  {
    return;
  }

  auto projects = projectManager->projects();
  if (projects.size() != 1)
  {
    return;
  }

  auto project = *projects.begin();
  auto mutableProject = dynamic_pointer_cast<smtk::project::Project>(project);
  mutableProject->setClean(false);
#endif
}

} // namespace extension
} // namespace smtk
