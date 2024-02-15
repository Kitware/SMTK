//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtDisconnectMode.h"

#include "smtk/extension/qt/diagram/qtBaseTaskNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramView.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/qtUtility.h"

#include "smtk/task/operators/RemoveDependency.h"

#include "smtk/operation/groups/ArcDeleter.h"
#include "smtk/operation/groups/DeleterGroup.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/io/Logger.h"

#include "smtk/view/icons/mode_disconnection_cpp.h"

#include <QColor>
#include <QComboBox>
#include <QIcon>
#include <QKeyEvent>

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{
namespace // anonymous
{

/// Create an operation and add it to a deleter map.
template<typename DeleterMap>
smtk::operation::Operation::Ptr addOpToDeleters(
  const std::shared_ptr<smtk::operation::Manager>& operationManager,
  DeleterMap& deleters,
  smtk::operation::Operation::Index opType,
  smtk::string::Token arcType,
  const std::string& targetName)
{
  auto op = operationManager->create(opType);
  if (op)
  {
    deleters[opType][arcType] = op;
  }
  else
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Cannot create deleter for \"" << targetName << "\".");
  }
  return op;
}

} // anonymous namespace

qtDisconnectMode::qtDisconnectMode(
  qtDiagram* diagram,
  qtDiagramView* view,
  QToolBar* toolbar,
  QActionGroup* modeGroup)
  : Superclass("disconnect", diagram, toolbar, modeGroup)
{
  (void)view;
  if (diagram->information().contains<smtk::common::Managers::Ptr>())
  {
    auto managers = diagram->information().get<smtk::common::Managers::Ptr>();
    if (managers && managers->contains<smtk::operation::Manager::Ptr>())
    {
      m_operationManager = managers->get<smtk::operation::Manager::Ptr>();
    }
  }
  QColor background = toolbar->palette().window().color();
  QIcon connectIcon(colorAdjustedIcon(mode_disconnection_svg(), background));
  this->modeAction()->setIcon(connectIcon);
}

bool qtDisconnectMode::eventFilter(QObject* obj, QEvent* event)
{
  auto* diagramView = m_diagram->diagramWidget();
  if (obj == diagramView)
  {
    switch (event->type())
    {
      case QEvent::KeyRelease:
      {
        auto* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent)
        {
          switch (keyEvent->key())
          {
            case Qt::Key_Delete:
            case Qt::Key_Backspace:
              this->removeSelectedArcs();
              break;
            case Qt::Key_Escape:
              if (m_diagram->diagramScene()->selectedItems().empty())
              {
                m_diagram->requestModeChange(m_diagram->defaultMode());
              }
              else
              {
                m_diagram->diagramScene()->clearSelection();
              }
              break;
            default:
              break;
          }
        }
      }
      break;
      default:
        break;
    }
  }
  return this->Superclass::eventFilter(obj, event);
}

void qtDisconnectMode::removeSelectedArcs()
{
  if (!m_operationManager)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Cannot delete arcs without an operation manager.");
    return;
  }

  smtk::operation::DeleterGroup componentDeleters(m_operationManager);
  smtk::operation::ArcDeleter arcDeleters(m_operationManager);
  // Use the scene-graph object type to determine which group of operations
  // to search for arc deletion; arcs that provide an object should search
  // the DeleterGroup while arcs that rely on components at endpoints should
  // search the ArcDeleter group.
  // In either case, we should find or create an operation for each type of
  // arc to be deleted and add it to the \a deleters map. Once all the items
  // are processed, we can launch all the operations in the \a deleters map.
  std::unordered_map<
    smtk::operation::Operation::Index,
    std::unordered_map<smtk::string::Token, std::shared_ptr<smtk::operation::Operation>>>
    deleters;

  bool ok = true;
  for (auto* item : m_diagram->diagramScene()->selectedItems())
  {
    auto* arc = dynamic_cast<qtBaseArc*>(item);
    if (!arc)
    {
      continue;
    }
    bool foundDeleter = false;
    smtk::operation::Operation::Ptr op;
    smtk::string::Token arcType = arc->arcType();
    if (auto* objectArc = dynamic_cast<qtObjectArc*>(arc))
    {
      if (auto* object = objectArc->object())
      {
        auto opType = componentDeleters.matchingOperation(*object);
        if (opType)
        {
          auto it = deleters.find(opType);
          if (it == deleters.end())
          {
            op = addOpToDeleters(m_operationManager, deleters, opType, arcType, object->name());
          }
          else
          {
            auto tt = it->second.find(arcType.data());
            if (tt == it->second.end())
            {
              op = addOpToDeleters(m_operationManager, deleters, opType, arcType, object->name());
            }
            else
            {
              op = tt->second;
            }
          }
          if (!op || !op->parameters()->associate(object->shared_from_this()))
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Cannot associate \"" << object->name() << "\" to deleter.");
            ok = false;
            continue;
          }
          foundDeleter = true;
        }
      }
    }
    if (!foundDeleter)
    {
      auto* predNode = dynamic_cast<qtBaseObjectNode*>(arc->predecessor());
      auto* succNode = dynamic_cast<qtBaseObjectNode*>(arc->successor());
      if (predNode && predNode->object() && succNode && succNode->object())
      {
        smtk::resource::PersistentObject::Ptr predObj = predNode->object()->shared_from_this();
        smtk::resource::PersistentObject::Ptr succObj = succNode->object()->shared_from_this();
        for (auto opType : arcDeleters.matchingOperations(arcType, *predObj, *succObj))
        {
          auto it = deleters.find(opType);
          if (it == deleters.end())
          {
            op = addOpToDeleters(m_operationManager, deleters, opType, arcType, arcType.data());
          }
          else
          {
            auto tt = it->second.find(arcType);
            if (tt == it->second.end())
            {
              op = addOpToDeleters(m_operationManager, deleters, opType, arcType, arcType.data());
            }
            else
            {
              op = tt->second;
            }
          }
          if (!op || !smtk::operation::ArcDeleter::appendArc(*op, arcType, predObj, succObj))
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Cannot append arc \"" << arcType.data()
                                     << "\" "
                                        "to operation \""
                                     << op->typeName() << "\".");
            ok = false;
          }
          // We got a matching operation configured; do not try to
          // delete the arc multiple times if multiple deleters exist.
          foundDeleter = true;
          break;
        }
      }
    }
    if (!foundDeleter)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Could not find deleter for arc \"" << arcType.data() << "\".");
      ok = false;
    }
  }

  if (!ok)
  {
    return;
  }

  // Now check that every operation we created is able to operate.
  for (const auto& opEntry : deleters)
  {
    for (const auto& arcEntry : opEntry.second)
    {
      if (!arcEntry.second->ableToOperate())
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "Cannot run \"" << arcEntry.second->typeName() << "\" deleter.");
        // Give up rather than deleting a partial set of arcs:
        ok = false;
      }
    }
  }

  if (!ok)
  {
    return;
  }

  // Finally, launch each operation.
  m_diagram->diagramScene()->clearSelection();
  for (const auto& opEntry : deleters)
  {
    for (const auto& arcEntry : opEntry.second)
    {
      m_operationManager->launchers()(arcEntry.second);
    }
  }
}

void qtDisconnectMode::enterMode()
{
  // Disable nodes so arc-preview works.
  m_diagram->enableNodeSelection(false);

  // Enable selection of arcs.
  m_diagram->enableArcSelection(true);

  // Click+drag should rubber-band select arcs.
  m_diagram->diagramWidget()->setDragMode(QGraphicsView::RubberBandDrag);

  // When in connect mode, grab certain events from the view widget
  // (for keyboard events).
  m_diagram->diagramWidget()->installEventFilter(this);
}

void qtDisconnectMode::exitMode()
{
  // When not in connect mode, do not process events from the view widget.
  m_diagram->diagramWidget()->removeEventFilter(this);

  // Enable tasks. (Next mode can re-disable them as needed.)
  m_diagram->enableNodeSelection(true);
  // Disable arc selection. (Next mode can re-enable as needed.)
  m_diagram->enableArcSelection(false);

  m_diagram->diagramWidget()->setDragMode(QGraphicsView::ScrollHandDrag);
}

} // namespace extension
} // namespace smtk
