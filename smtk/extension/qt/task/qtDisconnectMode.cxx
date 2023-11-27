//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/qtDisconnectMode.h"

#include "smtk/extension/qt/qtUtility.h"
#include "smtk/extension/qt/task/qtBaseTaskNode.h"
#include "smtk/extension/qt/task/qtPreviewArc.h"
#include "smtk/extension/qt/task/qtTaskEditor.h"
#include "smtk/extension/qt/task/qtTaskScene.h"
#include "smtk/extension/qt/task/qtTaskView.h"
#include "smtk/extension/qt/task/qtTaskViewConfiguration.h"

#include "smtk/task/operators/RemoveDependency.h"

// #include "smtk/operation/groups/ArcDeleter.h"
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

qtDisconnectMode::qtDisconnectMode(
  const std::shared_ptr<smtk::operation::Manager>& operationManager,
  qtTaskEditor* editor,
  qtTaskView* view,
  QToolBar* toolbar,
  QActionGroup* modeGroup)
  : Superclass("disconnect", editor, toolbar, modeGroup)
  , m_operationManager(operationManager)
{
  (void)view;
  QColor background = toolbar->palette().window().color();
  QIcon connectIcon(colorAdjustedIcon(mode_disconnection_svg(), background));
  this->modeAction()->setIcon(connectIcon);
}

bool qtDisconnectMode::eventFilter(QObject* obj, QEvent* event)
{
  auto* taskView = m_editor->taskWidget();
  if (obj == taskView)
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
            case Qt::Key_Backspace:
              this->removeSelectedArcs();
              break;
            case Qt::Key_Escape:
              if (m_editor->taskScene()->selectedItems().empty())
              {
                m_editor->requestModeChange(m_editor->defaultMode());
              }
              else
              {
                m_editor->taskScene()->clearSelection();
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
  // Use the scene-graph object type to determine which group of operations
  // to search for arc deletion; arcs that provide a component should search
  // the DeleterGroup while arcs that rely on components at endpoints should
  // search the ArcDeleter group.
  std::unordered_map<smtk::operation::Operation::Index, std::shared_ptr<smtk::operation::Operation>>
    deleters;
  auto delDep = m_operationManager->create<smtk::task::RemoveDependency>();
  deleters[delDep->index()] = delDep;

  bool ok = true;
  for (auto* arc : m_editor->taskScene()->selectedItems())
  {
    if (auto* taskArc = dynamic_cast<qtTaskArc*>(arc))
    {
      if (taskArc->arcType() == qtTaskArc::ArcType::Dependency)
      {
        // Dependency arcs can always be deleted, so this should never happen.
        if (
          !delDep->parameters()->associate(taskArc->predecessor()->task()->shared_from_this()) ||
          !delDep->parameters()->findComponent("to")->appendValue(
            taskArc->successor()->task()->shared_from_this()))
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Cannot delete dependency between \"" << taskArc->predecessor()->task()->name()
                                                  << "\" and \""
                                                  << taskArc->successor()->task()->name() << "\".");
          ok = false;
          continue;
        }
      }
      else
      {
        auto opType = componentDeleters.matchingOperation(*taskArc->adaptor());
        if (!opType)
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Cannot find deleter for \"" << taskArc->adaptor()->name() << "\".");
          ok = false;
          continue;
        }
        auto it = deleters.find(opType);
        if (it == deleters.end())
        {
          auto op = m_operationManager->create(opType);
          if (!op)
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Cannot create deleter for \"" << taskArc->adaptor()->name() << "\".");
            ok = false;
            continue;
          }
          deleters[opType] = op;
          it = deleters.find(opType);
        }
        if (!it->second->parameters()->associate(taskArc->adaptor()->shared_from_this()))
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Cannot associate \"" << taskArc->adaptor()->name() << "\" to deleter.");
          ok = false;
          continue;
        }
      }
    }
    else
    {
      // TODO: Handle other arc types.
    }

    // Now check that every operation we created is able to operate.
    for (const auto& entry : deleters)
    {
      if (!entry.second->ableToOperate())
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "Cannot run \"" << entry.second->typeName() << "\" deleter.");
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
  m_editor->taskScene()->clearSelection();
  for (const auto& entry : deleters)
  {
    m_operationManager->launchers()(entry.second);
  }
}

void qtDisconnectMode::enterMode()
{
  // Disable tasks so arc-preview works.
  m_editor->enableTasks(false);

  // Enable selection of arcs.
  m_editor->enableArcSelection(true);

  // Click+drag should rubber-band select arcs.
  m_editor->taskWidget()->setDragMode(QGraphicsView::RubberBandDrag);

  // When in connect mode, grab certain events from the view widget
  // (for keyboard events).
  m_editor->taskWidget()->installEventFilter(this);
}

void qtDisconnectMode::exitMode()
{
  // When not in connect mode, do not process events from the view widget.
  m_editor->taskWidget()->removeEventFilter(this);

  // Enable tasks. (Next mode can re-disable them as needed.)
  m_editor->enableTasks(true);
  // Disable arc selection. (Next mode can re-enable as needed.)
  m_editor->enableArcSelection(false);

  m_editor->taskWidget()->setDragMode(QGraphicsView::ScrollHandDrag);
}

} // namespace extension
} // namespace smtk
