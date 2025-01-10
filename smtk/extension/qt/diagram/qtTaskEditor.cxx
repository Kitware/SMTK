//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtTaskEditor.h"

#include "smtk/extension/qt/SVGIconEngine.h"
#include "smtk/extension/qt/qtManager.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtWorkletPalette.h"

#include "smtk/extension/qt/diagram/PanelConfiguration_cpp.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramLegend.h"
#include "smtk/extension/qt/diagram/qtDiagramLegendEntry.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramView.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtTaskArc.h"
#include "smtk/extension/qt/diagram/qtTaskPath.h"
#include "smtk/extension/qt/diagram/qtTaskPortArc.h"
#include "smtk/extension/qt/diagram/qtTaskPortNode.h"

// nodes
#include "smtk/extension/qt/diagram/qtResourceNode.h"
#include "smtk/extension/qt/diagram/qtTaskNode.h"

// modes
#include "smtk/extension/qt/diagram/qtConnectMode.h"
#include "smtk/extension/qt/diagram/qtDisconnectMode.h"
#include "smtk/extension/qt/diagram/qtPanMode.h"
#include "smtk/extension/qt/diagram/qtSelectMode.h"
#include "smtk/extension/qt/diagram/qtTaskPath.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/json/jsonView.h"

#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"

#include "smtk/task/Active.h"
#include "smtk/task/Adaptor.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/ResultOps.h"
#include "smtk/operation/groups/ArcCreator.h"

#include "smtk/resource/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/io/Logger.h"

#include "smtk/common/Managers.h"
#include "smtk/common/json/jsonUUID.h"

#include "smtk/Regex.h"

#include "nlohmann/json.hpp"

#include <QAction>
#include <QActionGroup>
#include <QBrush>
#include <QColor>
#include <QComboBox>
#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QLayout>
#include <QMimeData>
#include <QPalette>
#include <QPointF>
#include <QTimer>
#include <QToolBar>
#include <QWidget>
#include <QWidgetAction>

#define SMTK_ARC_TASK_DEP "task dependency"
#define SMTK_ARC_TASK_DEP_TOK "task dependency"_token
#define SMTK_ARC_PORT_CONN "port connection"
#define SMTK_ARC_PORT_CONN_TOK "port connection"_token

// Uncomment to get debug printouts from workflow events.
// #define SMTK_DBG_WORKFLOWS 1

using namespace smtk::string::literals;

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
    auto managers = info.get<std::shared_ptr<smtk::common::Managers>>();
    if (managers)
    {
      m_managers = managers;
      m_operationManager = managers->get<smtk::operation::Manager::Ptr>();
    }
    this->addWorkletPalette(info);
  }

  ~Internal() = default;

  void addWorkletPalette(const smtk::view::Information& generatorInfo)
  {
    // Use the parent widget's UI manager, but create new view information
    // and configuration objects.
    auto* uiMgr = generatorInfo.get<smtk::extension::qtUIManager*>();
    smtk::view::Information workletInfo;
    nlohmann::json jsonConfig = {
      { "Type", "qtWorkletPalette" },
      { "Name", "Worklets" },
      { "Component",
        { { "Name", "Details" },
          { "Attributes",
            { { "SearchBar", true }, { "Title", "Worklets" }, { "ShowTitle", true } } },
          { "Children",
            { { { "Name", "Model" }, { "Attributes", { { "Autorun", "true" } } } } } } } }
    };
    smtk::view::ConfigurationPtr workletConfig = jsonConfig;
    QWidget* ww = new QWidget;
    auto* ll = new QVBoxLayout;
    ll->setMargin(0);
    ww->setLayout(ll);
    ww->setObjectName("Worklets");
    m_self->diagram()->sidebar()->layout()->addWidget(ww);
    workletInfo.insert(ww);
    workletInfo.insert(workletConfig);
    workletInfo.insert(m_managers);
    workletInfo.insert(uiMgr);
    auto* view = uiMgr->setSMTKView(workletInfo);
    m_worklets = dynamic_cast<smtk::extension::qtWorkletPalette*>(view);

    if (m_worklets)
    {
      // When the palette's model has Autorun=true set (see jsonConfig above),
      // then we must provide the model with the qtTaskEditor via which it can
      // emplace worklets:
      QObject::connect(
        m_worklets.data(),
        &smtk::extension::qtWorkletPalette::emplaceWorklet,
        m_self,
        &smtk::extension::qtTaskEditor::addWorklet);
    }
  }

  qtTaskEditor* m_self;
  // The task manager for the active project (if any project is active).
  smtk::task::Manager* m_taskManager{ nullptr };
  // Observers that keep task- and arc-items up to date.
  smtk::task::Active::Observers::Key m_activeObserverKey;
  // The operation manager obtained from the view information.
  std::shared_ptr<smtk::operation::Manager> m_operationManager;
  // The common::Managers object obtained from the view information.
  std::shared_ptr<smtk::common::Managers> m_managers;

  QPointer<smtk::extension::qtWorkletPalette> m_worklets;
};

qtTaskEditor::qtTaskEditor(
  const smtk::view::Information& info,
  const smtk::view::Configuration::Component& config,
  qtDiagram* parent)
  : Superclass(info, config, parent)
  , m_p(new Internal(this, info))
{
  // Register an arc type for dependencies
  this->diagram()->legend()->addEntry(
    new qtDiagramLegendEntry("arc"_token, SMTK_ARC_TASK_DEP, this));
  // Register an arc type for port connections
  this->diagram()->legend()->addEntry(
    new qtDiagramLegendEntry("arc"_token, SMTK_ARC_PORT_CONN, this));
  m_taskPath = new qtTaskPath(this);
  this->diagram()->topFrame()->layout()->addWidget(m_taskPath);
}

qtTaskEditor::~qtTaskEditor() = default;

smtk::task::Manager* qtTaskEditor::manager() const
{
  return m_p->m_taskManager;
}

template<bool RemoveUnusedArcs>
bool qtTaskEditor::updateArcs(
  smtk::resource::PersistentObject* object,
  QRectF& modBounds,
  ArcLegendEntries& legendInfo)
{
  bool diagramModified = false;
  if (auto* adaptor = dynamic_cast<smtk::task::Adaptor*>(object))
  {
    // Adaptors get an arc.
    auto* fromTaskNode = this->findTaskNode(adaptor->from());
    auto* toTaskNode = this->findTaskNode(adaptor->to());
    if (!(fromTaskNode && toTaskNode))
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "An adaptor's arc could not be added because the nodes don't exist (yet?).");
      return diagramModified;
    }
    qtTaskArc* arc = nullptr;
    const auto* arcs = this->diagram()->findArcs(fromTaskNode, toTaskNode);
    if (arcs)
    {
      auto it = arcs->find(smtk::common::typeName<qtTaskArc>());
      if (it != arcs->end())
      {
        for (const auto& testArc : it->second)
        {
          arc = dynamic_cast<qtTaskArc*>(testArc);
          if (arc && arc->adaptor() == adaptor)
          {
            // TODO: Any updates to apply to the arc?
            break;
          }
        }
      }
    }
    if (!arc)
    {
      auto arcType = adaptor->typeToken();
      if (legendInfo.find(arcType) == legendInfo.end())
      {
        qtDiagramLegendEntry* entry{ nullptr };
        auto rsrcMgr = m_diagram->information()
                         .get<smtk::common::Managers::Ptr>()
                         ->get<smtk::resource::Manager::Ptr>();
        if (rsrcMgr)
        {
          const auto& typeLabels = rsrcMgr->objectTypeLabels();
          auto labelIt = typeLabels.find(arcType);
          if (labelIt != typeLabels.end())
          {
            entry = new qtDiagramLegendEntry("arc"_token, arcType, this, labelIt->second);
          }
        }
        if (!entry)
        {
          entry = new qtDiagramLegendEntry("arc"_token, arcType, this);
        }
        if (m_diagram->legend()->addEntry(entry))
        {
          legendInfo[arcType] = entry;
        }
      }
      arc = new qtTaskArc(this, fromTaskNode, toTaskNode, adaptor);
      this->diagram()->addArc(arc);
      diagramModified = true;
      modBounds = modBounds.united(arc->boundingRect());
    }
  }
  else if (auto* task = dynamic_cast<smtk::task::Task*>(object))
  {
    // Task dependencies get arcs.
    auto* taskNode = dynamic_cast<qtBaseTaskNode*>(m_diagram->findNode(task->id()));
    if (!taskNode)
    {
      // TODO: Warn?
      return diagramModified;
    }
    taskNode->dataUpdated();
    auto depTasks = task->dependencies();
    // Use m_diagram->predecessorsOf(taskNode) to visit all tasks that \a task
    // depends on; any arcs not present in depTasks should be removed.
    auto predecessors = this->diagram()->predecessorsOf(taskNode);
    for (auto* basePredecessor : predecessors)
    {
      auto* predecessor = dynamic_cast<qtBaseTaskNode*>(basePredecessor);
      if (!predecessor)
      {
        // TODO: Warn?
        continue;
      }
      if (
        depTasks.find(std::static_pointer_cast<smtk::task::Task>(
          predecessor->task()->shared_from_this())) != depTasks.end())
      {
        // This predecessor is still a dependency. No need to remove it.
        continue;
      }
      // predecessor is not a dependency, so remove any attached dependency arcs.
      if (const auto* arcMap = this->diagram()->findArcs(predecessor, taskNode))
      {
        auto it = arcMap->find(SMTK_ARC_TASK_DEP_TOK);
        if (it != arcMap->end())
        {
          std::unordered_set<qtBaseArc*> arcsToErase(it->second.begin(), it->second.end());
          for (auto* arc : arcsToErase)
          {
            auto* taskArc = dynamic_cast<qtTaskArc*>(arc);
            diagramModified = true;
            modBounds = modBounds.united(taskArc->boundingRect());
            this->diagram()->removeArc(taskArc);
          }
        }
      }
    }
    // Create any missing dependency arcs on newly-created tasks.
    for (const auto& depTask : depTasks)
    {
      auto* depNode = dynamic_cast<qtBaseTaskNode*>(m_diagram->findNode(depTask->id()));
      if (!depNode)
      {
        continue;
      } // TODO: Warn?
      const auto* arcMapSet = m_diagram->findArcs(depNode, taskNode);
      bool shouldCreate = true;
      if (arcMapSet)
      {
        auto it = arcMapSet->find(SMTK_ARC_TASK_DEP_TOK);
        if (it != arcMapSet->end())
        {
          // The dependency arc already exists.
          shouldCreate = false;
        }
      }
      if (shouldCreate)
      {
        auto* arc = new qtTaskArc(this, depNode, taskNode);
        this->diagram()->addArc(arc);
        diagramModified = true;
        modBounds = modBounds.united(arc->boundingRect());
      }
    }
  }
  else if (auto* port = dynamic_cast<smtk::task::Port*>(object))
  {
    // Port connections get arcs.
    // Only attach arcs to input ports; output ports must always attach
    // to an input port if a connection exists at all, so we are guaranteed
    // to enumerate all arcs considering only one port direction.
    // Furthermore, input ports may accept arcs from non-port objects
    // (e.g., resources or components), which are captured if we always
    // consider input ports but not vice-versa.
    auto* portNode = dynamic_cast<qtTaskPortNode*>(m_diagram->findNode(port->id()));
    if (!portNode || port->direction() != smtk::task::Port::Direction::In)
    {
      // TODO: Warn?
      return diagramModified;
    }
    portNode->dataUpdated();
    const auto& portConns = port->connections();
    // Use m_diagram->predecessorsOf(portNode) to visit all objects that \a port
    // connects to; any arcs not present in portConns should be removed.
    auto predecessors = this->diagram()->predecessorsOf(portNode);
    for (auto* basePredecessor : predecessors)
    {
      auto* predecessor = dynamic_cast<qtBaseObjectNode*>(basePredecessor);
      if (!predecessor)
      {
        // TODO: Warn?
        continue;
      }
      if (portConns.find(predecessor->object()) != portConns.end())
      {
        // This predecessor is still a dependency. No need to remove it.
        continue;
      }
      // predecessor is not a dependency, so remove any attached dependency arcs.
      if (const auto* arcMap = this->diagram()->findArcs(predecessor, portNode))
      {
        auto it = arcMap->find(SMTK_ARC_PORT_CONN_TOK);
        if (it != arcMap->end())
        {
          std::unordered_set<qtBaseArc*> arcsToErase(it->second.begin(), it->second.end());
          for (auto* arc : arcsToErase)
          {
            auto* portArc = dynamic_cast<qtTaskPortArc*>(arc);
            diagramModified = true;
            modBounds = modBounds.united(portArc->boundingRect());
            this->diagram()->removeArc(portArc);
          }
        }
      }
    }
    // Create any missing dependency arcs on newly-created ports.
    for (const auto& connObj : portConns)
    {
      if (!connObj)
      {
        continue;
      }

      auto* depNode = dynamic_cast<qtBaseObjectNode*>(m_diagram->findNode(connObj->id()));
      if (!depNode)
      {
        continue;
      } // TODO: Warn?
      const auto* arcMapSet = m_diagram->findArcs(depNode, portNode);
      bool shouldCreate = true;
      if (arcMapSet)
      {
        auto it = arcMapSet->find(SMTK_ARC_TASK_DEP_TOK);
        if (it != arcMapSet->end())
        {
          // The dependency arc already exists.
          shouldCreate = false;
        }
      }
      if (shouldCreate)
      {
        auto* arc = new qtTaskPortArc(this, depNode, portNode);
        this->diagram()->addArc(arc);
        diagramModified = true;
        modBounds = modBounds.united(arc->boundingRect());
        this->updateArcStatus(arc);
      }
    }
  }
  return diagramModified;
}

void qtTaskEditor::updateArcStatus(qtTaskPortArc* arc)
{
  if (!arc)
  {
    return;
  }
  bool visible = arc->successor()->isVisible() && arc->predecessor()->isVisible();
  arc->setVisible(visible);
  bool selectable = (m_diagram->mode() == "disconnect"_token);
  if (selectable)
  {
    arc->setFlags(arc->flags() | QGraphicsItem::ItemIsSelectable);
  }
  else
  {
    arc->setFlags(arc->flags() & ~QGraphicsItem::ItemIsSelectable);
  }
}

qtBaseTaskNode* qtTaskEditor::findTaskNode(const smtk::task::Task* task) const
{
  if (!task)
  {
    return nullptr;
  }

  return dynamic_cast<qtBaseTaskNode*>(m_diagram->findNode(task->id()));
}

void qtTaskEditor::updateSceneNodes(
  std::unordered_set<smtk::resource::PersistentObject*>& created,
  std::unordered_set<smtk::resource::PersistentObject*>& modified,
  std::unordered_set<smtk::resource::PersistentObject*>& expunged,
  const smtk::operation::Operation& operation,
  const smtk::operation::Operation::Result& result)
{
  (void)modified;
  (void)operation;
  (void)result;

  // Get the parent task (if there is one) that will be used to set node visibility
  smtk::task::Task* targetParent = m_taskPath->lastTask();
  // Track whether changes were made to the diagram. If so, we
  // will ask the view to scale/scroll to include the task-diagram.
  bool diagramModified = false;
  QRectF modBounds;

  auto managers = this->diagram()->managers();
  auto qtmgr = managers ? managers->get<smtk::extension::qtManager::Ptr>() : nullptr;
  if (!qtmgr)
  {
    return;
  }

  // First, create nodes for tasks that have been created.
  for (auto* obj : created)
  {
    if (auto* task = dynamic_cast<smtk::task::Task*>(obj))
    {
      if (auto* taskManager = task->manager())
      {
        std::string taskNodeType = "smtk::extension::qtTaskNode";
        for (const auto& style : task->style())
        {
          const auto& styleConfig = taskManager->getStyle(style);
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
        auto* tnode = qtmgr->taskNodeFactory()
                        .createFromName(
                          taskNodeType,
                          static_cast<qtDiagramGenerator*>(this),
                          task,
                          static_cast<QGraphicsItem*>(nullptr))
                        .release();
        if (!tnode)
        {
          tnode = new qtTaskNode(this, task);
          smtkWarningMacro(
            smtk::io::Logger::instance(),
            "Could not find task node class " << taskNodeType << " creating default task node!");
        }

        tnode->setEnabled(this->diagram()->nodesEnabled());
        tnode->setVisible(task->parent() == targetParent);
        if (this->diagram()->arcSelectionEnabled())
        {
          tnode->setFlags(tnode->flags() & ~QGraphicsItem::ItemIsSelectable);
        }
        else
        {
          tnode->setFlags(tnode->flags() | QGraphicsItem::ItemIsSelectable);
        }
        // Index the task for everyone
        this->diagram()->addNode(tnode);
        QObject::connect(
          tnode, &qtBaseNode::nodeMoved, this, &qtTaskEditor::updateArcsOfSendingNodeRecursive);
        diagramModified = true;
        modBounds = modBounds.united(tnode->boundingRect());
        for (const auto& pinfo : task->ports())
        {
          // See if we have already built the TaskPortNode for this port
          if (m_diagram->findNode(pinfo.second->id()))
          {
            continue;
          }

          qtTaskPortNode* pnode;
          if (pinfo.second->access() == smtk::task::Port::Internal)
          {
            pnode = new qtTaskPortNode(this, pinfo.second, nullptr);
            // by default internal ports are not displayed
            pnode->setVisible(false);
          }
          else
          {
            pnode = new qtTaskPortNode(this, pinfo.second, tnode);
          }
          pnode->setEnabled(this->diagram()->nodesEnabled());
          if (this->diagram()->arcSelectionEnabled())
          {
            pnode->setFlags(pnode->flags() & ~QGraphicsItem::ItemIsSelectable);
          }
          else
          {
            pnode->setFlags(pnode->flags() | QGraphicsItem::ItemIsSelectable);
          }
          // Index the port for everyone
          this->diagram()->addNode(pnode);
          modBounds = modBounds.united(pnode->boundingRect());
        }
      }
    }
    else if (auto* project = dynamic_cast<smtk::project::Project*>(obj))
    {
      // NB: Only one project is allowed at a time, so it is safe to simply
      // re-assign our active-task observer key to watch this new project's tasks.
      QPointer<qtTaskEditor> self(this);
      m_p->m_taskManager = &project->taskManager();
      m_p->m_activeObserverKey = project->taskManager().active().observers().insert(
        [this, self](smtk::task::Task* prev, smtk::task::Task* next) {
          if (!self)
          {
            return;
          }
          // std::cout << "Switch active task from " << prev << " to " << next << "\n";
          // What is the current tail of the task path
          auto* currentLastTask = m_taskPath->lastTask();
          auto* prevTaskNode = this->findTaskNode(prev);
          auto* nextTaskNode = this->findTaskNode(next);
          if (prevTaskNode)
          {
            auto prevState = prev->state();
            prevTaskNode->updateTaskState(prevState, prevState, false);
            prevTaskNode->setOutlineStyle(qtBaseTaskNode::OutlineStyle::Normal);
          }
          if (nextTaskNode)
          {
            auto nextState = next->state();
            nextTaskNode->updateTaskState(nextState, nextState, true);
            nextTaskNode->setOutlineStyle(qtBaseTaskNode::OutlineStyle::Active);
          }
          this->m_taskPath->setActiveTask(next);
          // Update visibility if needed
          auto* newLastTask = m_taskPath->lastTask();
          this->updateVisibility(currentLastTask, newLastTask);
        },
        0,
        true,
        "qtTaskEditor watching active task.");
    }
  }

  // Second, create ports now that nodes have been established.
  for (auto* obj : created)
  {
    if (auto* port = dynamic_cast<smtk::task::Port*>(obj))
    {
      // See if we have already built the TaskPortNode for this port
      if (m_diagram->findNode(port->id()))
      {
        continue;
      }

      auto* tnode = this->findTaskNode(port->parent());
      qtTaskPortNode* pnode;
      if (port->access() == smtk::task::Port::Internal)
      {
        pnode = new qtTaskPortNode(this, port, nullptr);
        // by default internal ports are not displayed
        pnode->setVisible(false);
      }
      else
      {
        pnode = new qtTaskPortNode(this, port, tnode);
      }

      pnode->setEnabled(this->diagram()->nodesEnabled());
      if (this->diagram()->arcSelectionEnabled())
      {
        pnode->setFlags(pnode->flags() & ~QGraphicsItem::ItemIsSelectable);
      }
      else
      {
        pnode->setFlags(pnode->flags() | QGraphicsItem::ItemIsSelectable);
      }
      // Index the port for everyone
      this->diagram()->addNode(pnode);
      diagramModified = true;
      modBounds = modBounds.united(pnode->boundingRect());
    }
  }

  // Currently only deal with the project being deleted
  for (auto* obj : expunged)
  {
    if (dynamic_cast<smtk::project::Project*>(obj) != nullptr)
    {
      // Make sure that the task path has been cleared.
      m_taskPath->gotoRoot();
    }
  }

  // Finally, if changes were made, draw attention to the task-diagram.
  if (diagramModified)
  {
    this->diagram()->includeInView(modBounds);
  }
}

void qtTaskEditor::updateSceneArcs(
  std::unordered_set<smtk::resource::PersistentObject*>& created,
  std::unordered_set<smtk::resource::PersistentObject*>& modified,
  std::unordered_set<smtk::resource::PersistentObject*>& expunged,
  const smtk::operation::Operation& operation,
  const smtk::operation::Operation::Result& result)
{
  (void)operation;
  (void)result;

  // Track whether changes were made to the diagram. If so, we
  // will ask the view to scale/scroll to include the task-diagram.
  bool diagramModified = false;
  QRectF modBounds;

  auto managers = this->diagram()->managers();
  auto qtmgr = managers ? managers->get<smtk::extension::qtManager::Ptr>() : nullptr;
  if (!qtmgr)
  {
    return;
  }

  // First, create arcs now that ports and tasks have been established.
  ArcLegendEntries legendInfo = m_diagram->legend()->typesInGroup("arc"_token);
  for (auto* obj : created)
  {
    diagramModified |= this->updateArcs<false>(obj, modBounds, legendInfo);
  }
  // Second, process modified tasks/adaptors
  for (auto* obj : modified)
  {
    diagramModified |= this->updateArcs<true>(obj, modBounds, legendInfo);
  }
  // Third, process expunged tasks/adaptors
  for (auto* obj : expunged)
  {
    if (auto* adaptor = dynamic_cast<smtk::task::Adaptor*>(obj))
    {
      auto* fromTaskNode = this->findTaskNode(adaptor->from());
      auto* toTaskNode = this->findTaskNode(adaptor->to());
      if (fromTaskNode && toTaskNode)
      {
        const auto* candidates = this->diagram()->findArcs(fromTaskNode, toTaskNode);
        if (candidates)
        {
          std::unordered_set<qtTaskArc*> arcsToErase;
          for (const auto& arcEntry : *candidates)
          {
            for (auto* arc : arcEntry.second)
            {
              if (auto* taskArc = dynamic_cast<qtTaskArc*>(arc))
              {
                if (taskArc->adaptor() == adaptor)
                {
                  arcsToErase.insert(taskArc);
                }
              }
            }
          }
          for (auto* taskArc : arcsToErase)
          {
            modBounds = modBounds.united(taskArc->boundingRect());
            diagramModified = true;
            this->diagram()->removeArc(taskArc);
          }
        }
      }
    }
    else if (auto* task = dynamic_cast<smtk::task::Task*>(obj))
    {
      if (auto* node = this->diagram()->findNode(task->id()))
      {
        QRectF nodeBounds = node->boundingRect();
        nodeBounds = nodeBounds.united(node->childrenBoundingRect());
        modBounds = modBounds.united(node->sceneTransform().mapRect(nodeBounds));
        diagramModified = true;
        // Remove all of the task's ports from the diagram before the task.
        // This also forces removal of port-port arcs.
        for (auto portEntry : task->ports())
        {
          this->diagram()->removeNode(this->diagram()->findNode(portEntry.second->id()));
        }
        this->diagram()->removeNode(node);
      }
    }
    else if (auto* port = dynamic_cast<smtk::task::Port*>(obj))
    {
      if (auto* node = this->diagram()->findNode(port->id()))
      {
        QRectF nodeBounds = node->boundingRect();
        modBounds = modBounds.united(node->sceneTransform().mapRect(nodeBounds));
        diagramModified = true;
        this->diagram()->removeNode(node);
      }
    }
    else if (auto* project = dynamic_cast<smtk::project::Project*>(obj))
    {
      // NB: If we want to support multiple projects, we should check
      // that project == the project we were observing.
      project->taskManager().active().observers().erase(m_p->m_activeObserverKey);
      m_p->m_taskManager = nullptr;
    }
  }

  // Finally, if changes were made, draw attention to the task-diagram.
  if (diagramModified)
  {
    this->diagram()->includeInView(modBounds);
  }
}

std::shared_ptr<smtk::view::Configuration> qtTaskEditor::defaultConfiguration()
{
  std::shared_ptr<smtk::view::Configuration> result;
  auto jsonConfig = nlohmann::json::parse(taskPanelConfiguration())[0];
  result = jsonConfig;
  return result;
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
  if (auto locationItem = op->parameters()->findDouble("location"))
  {
    if (std::isfinite(location[0]) && std::isfinite(location[1]))
    {
      for (int ii = 0; ii < 2; ++ii)
      {
        locationItem->setValue(ii, location[ii]);
      }
    }
    else
    {
      // TODO: Choose an appropriate location.
      for (int ii = 0; ii < 2; ++ii)
      {
        locationItem->setValue(ii, 0.0);
      }
    }
  }
  if (!op->ableToOperate())
  {
    return false;
  }
  opMgr->launchers()(op);
  return true;
}

bool qtTaskEditor::acceptDropProposal(QDragEnterEvent* event)
{
  if (event->mimeData()->hasFormat("application/x-smtk-worklet-name"))
  {
    event->acceptProposedAction();
    return true;
  }
  return false;
}

void qtTaskEditor::moveDropPoint(QDragMoveEvent* event)
{
  (void)event;
}

void qtTaskEditor::abortDrop(QDragLeaveEvent* event)
{
  (void)event;
}

bool qtTaskEditor::acceptDrop(QDropEvent* event)
{
  bool didAdd = false;
  bool didFail = false;
  QPointF mappedPt = this->diagram()->diagramWidget()->mapToScene(event->pos());
  std::array<double, 2> location{ { mappedPt.x(), mappedPt.y() } };
  QByteArray encodedData = event->mimeData()->data("application/x-smtk-worklet-name");
  QDataStream stream(&encodedData, QIODevice::ReadOnly);

  while (!stream.atEnd())
  {
    QString text;
    stream >> text;
    if (this->addWorklet(text.toStdString(), location))
    {
      didAdd = true;
    }
    else
    {
      didFail = true;
    }
  }
  if (didAdd)
  {
    event->acceptProposedAction();
  }
  if (didFail)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Failed to emplace one or more dropped worklets.");
  }
  return didAdd && !didFail;
}

void qtTaskEditor::updateVisibility(
  const smtk::task::Task* prevTailTask,
  const smtk::task::Task* nextTailTask)
{
  // If the tasks are the same then there is nothing that needs to be done
  if (prevTailTask == nextTailTask)
  {
    return;
  }

  // We are changing the display so make sure the diagram is in its default mode
  m_diagram->requestModeChange(m_diagram->defaultMode());

  // If the previous tail of the task path is null then we were showing the toplevel
  // tasks (and arcs) which need now need to be hidden.
  std::unordered_set<smtk::task::Task*> tasks;
  if (prevTailTask == nullptr)
  {
    tasks = m_p->m_taskManager->taskInstances().topLevelTasks();
  }
  else
  {
    tasks = prevTailTask->children();
    // Hide the node's internal ports and arcs
    for (auto pinfo : prevTailTask->ports())
    {
      if (pinfo.second->access() == smtk::task::Port::Internal)
      {
        auto* node = m_diagram->findNode(pinfo.second->id());
        if (node)
        {
          node->setVisible(false);
          // Turn off its arcs
          auto nodeArcs = m_diagram->arcsOfNode(node);
          for (auto* arc : nodeArcs)
          {
            arc->setVisible(false);
          }
        }
      }
    }
  }

  // Hide previously visible tasks, plus their external ports and associated arcs.
  for (auto* t : tasks)
  {
    // Turn off the corresponding Task Node
    auto* node = m_diagram->findNode(t->id());
    if (node)
    {
      node->setVisible(false);
      // Turn off its arcs
      auto nodeArcs = m_diagram->arcsOfNode(node);
      for (auto* arc : nodeArcs)
      {
        arc->setVisible(false);
      }
      // Now handle ports of task and port arcs
      for (const auto& entry : t->ports())
      {
        if (auto* portNode = m_diagram->findNode(entry.second->id()))
        {
          // Hide external ports and attached arcs.
          portNode->setVisible(false);
          auto nodeArcs = m_diagram->arcsOfNode(portNode);
          for (auto* arc : nodeArcs)
          {
            arc->setVisible(false);
          }
        }
      }
    }
  }

  if (nextTailTask == nullptr)
  {
    tasks = m_p->m_taskManager->taskInstances().topLevelTasks();
  }
  else
  {
    // Turn on the node's internal ports
    for (auto pinfo : nextTailTask->ports())
    {
      // Togggle visibility of ports based on whether
      // they are internal (becoming visible) or external
      // (becoming hidden).
      bool visibility = (pinfo.second->access() == smtk::task::Port::Internal);
      auto* node = m_diagram->findNode(pinfo.second->id());
      if (node)
      {
        node->setVisible(visibility);
        // Turn off its arcs
        auto nodeArcs = m_diagram->arcsOfNode(node);
        for (auto* arc : nodeArcs)
        {
          arc->setVisible(visibility);
        }
      }
    }
    tasks = nextTailTask->children();
  }

  // Show the tasks selected above, plus these tasks' external
  // ports and associated arcs.
  for (auto* t : tasks)
  {
    auto* node = m_diagram->findNode(t->id());
    if (node)
    {
      node->setVisible(true);
      auto nodeArcs = m_diagram->arcsOfNode(node);
      for (auto* arc : nodeArcs)
      {
        arc->setVisible(true);
      }
      // Now handle ports of task and port arcs
      for (const auto& entry : t->ports())
      {
        if (auto* portNode = m_diagram->findNode(entry.second->id()))
        {
          // Show external ports and attached arcs.
          bool visibility = (entry.second->access() == smtk::task::Port::Access::External);
          portNode->setVisible(visibility);
          auto nodeArcs = m_diagram->arcsOfNode(portNode);
          for (auto* arc : nodeArcs)
          {
            arc->setVisible(visibility);
          }
        }
      }
    }
  }
}

} // namespace extension
} // namespace smtk
