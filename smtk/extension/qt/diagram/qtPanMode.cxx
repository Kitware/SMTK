//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtPanMode.h"

#include "smtk/extension/qt/diagram/qtBaseNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramView.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/qtUtility.h"

#include "smtk/view/icons/mode_pan_cpp.h"

#include <QColor>
#include <QIcon>
#include <QKeyEvent>

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{

qtPanMode::qtPanMode(
  qtDiagram* diagram,
  qtDiagramView* view,
  QToolBar* toolbar,
  QActionGroup* modeGroup)
  : Superclass("pan", diagram, toolbar, modeGroup)
{
  (void)view;
  QColor background = toolbar->palette().window().color();
  QIcon panIcon(colorAdjustedIcon(mode_pan_svg(), background));
  this->modeAction()->setIcon(panIcon);
}

qtPanMode::~qtPanMode() = default;

void qtPanMode::enableSelectionSensitiveActions()
{
  auto nodes = m_diagram->diagramScene()->nodesOfSelection();
  this->changeSelectionSensitiveActions(!nodes.empty());
}

void qtPanMode::zoomToAll()
{
  auto bounds = m_diagram->diagramScene()->sceneRect();
  m_diagram->diagramWidget()->centerOn(bounds.center());
  m_diagram->diagramWidget()->fitInView(bounds, Qt::KeepAspectRatio);
}

void qtPanMode::zoomToSelected()
{
  auto qsel = m_diagram->diagramScene()->selectedItems();
  QRectF bounds;
  for (const auto& item : qsel)
  {
    if (auto* node = dynamic_cast<qtBaseNode*>(item))
    {
      bounds = bounds.united(node->sceneBoundingRect().united(
        (node->childrenBoundingRect() * node->transform()).boundingRect()));
    }
  }
  m_diagram->diagramWidget()->centerOn(bounds.center());
  m_diagram->diagramWidget()->fitInView(bounds, Qt::KeepAspectRatio);
}

bool qtPanMode::eventFilter(QObject* obj, QEvent* event)
{
  if (obj == m_diagram->diagramWidget())
  {
    if (this->isModeActive())
    {
      if (event->type() == QEvent::KeyPress)
      {
        auto* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent)
        {
          switch (keyEvent->key())
          {
            case Qt::Key_Shift:
              m_diagram->diagramWidget()->addModeSnapback(Qt::Key_Shift, "select"_token);
              break;
            case Qt::Key_Escape:
              m_diagram->requestModeChange(m_diagram->defaultMode());
              break;
            case Qt::Key_Backspace:
            case Qt::Key_Delete:
              this->removeSelectedObjects();
              break;
          }
        }
      }
    }
    return false;
  }
  return this->Superclass::eventFilter(obj, event);
}

void qtPanMode::enterMode()
{
  this->addModeButtons();
  this->showModeButtons(true);
  // When in pan mode, grab certain events from the view widget.
  m_diagram->diagramWidget()->installEventFilter(this);
}

void qtPanMode::exitMode()
{
  this->showModeButtons(false);
  // When not in pan mode, do not process events from the view widget.
  m_diagram->diagramWidget()->removeEventFilter(this);
}

void qtPanMode::addModeButtons()
{
  if (m_zoomToAll)
  {
    return;
  }

  m_zoomToAll = new QAction("Zoom to all");
  m_zoomToAll->setObjectName("ZoomAll");
  m_zoomToAll->setIcon(QIcon(":/icons/diagram/zoom_all.svg"));
  m_diagram->tools()->addAction(m_zoomToAll);
  QObject::connect(m_zoomToAll, &QAction::triggered, this, &qtPanMode::zoomToAll);

  m_zoomToSelected = new QAction("Zoom to selected items");
  m_zoomToSelected->setObjectName("ZoomSelected");
  m_zoomToSelected->setIcon(QIcon(":/icons/diagram/zoom_selected.svg"));
  m_diagram->tools()->addAction(m_zoomToSelected);
  QObject::connect(m_zoomToSelected, &QAction::triggered, this, &qtPanMode::zoomToSelected);

  QObject::connect(
    m_diagram->diagramScene(),
    &QGraphicsScene::selectionChanged,
    this,
    &qtPanMode::enableSelectionSensitiveActions);

  // Force button state to match current selection:
  this->enableSelectionSensitiveActions();
}

void qtPanMode::showModeButtons(bool show)
{
  if (!m_zoomToAll)
  {
    return;
  }

  m_zoomToAll->setVisible(show);
  m_zoomToSelected->setVisible(show);
}

void qtPanMode::changeSelectionSensitiveActions(bool enable)
{
  if (!m_zoomToAll)
  {
    return;
  }

  m_zoomToSelected->setEnabled(enable);
}

} // namespace extension
} // namespace smtk
