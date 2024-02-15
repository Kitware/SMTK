//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtSelectMode.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramView.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/qtUtility.h"

#include "smtk/Regex.h"

#include "smtk/view/icons/mode_selection_cpp.h"

#include <QColor>
#include <QIcon>
#include <QKeyEvent>

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{

qtSelectMode::qtSelectMode(
  qtDiagram* diagram,
  qtDiagramView* view,
  QToolBar* toolbar,
  QActionGroup* modeGroup)
  : Superclass("select", diagram, toolbar, modeGroup)
{
  (void)view;
  QColor background = toolbar->palette().window().color();
  QIcon selectionIcon(colorAdjustedIcon(mode_selection_svg(), background));
  this->modeAction()->setIcon(selectionIcon);
}

qtSelectMode::~qtSelectMode() = default;

bool qtSelectMode::eventFilter(QObject* obj, QEvent* event)
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
              m_diagram->diagramWidget()->addModeSnapback(Qt::Key_Shift, "pan"_token);
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

void qtSelectMode::enterMode()
{
  m_diagram->diagramWidget()->setDragMode(QGraphicsView::RubberBandDrag);

  // When in connect mode, grab certain events from the view widget.
  m_diagram->diagramWidget()->installEventFilter(this);
}

void qtSelectMode::exitMode()
{
  // When not in connect mode, do not process events from the view widget.
  m_diagram->diagramWidget()->removeEventFilter(this);

  m_diagram->diagramWidget()->setDragMode(QGraphicsView::ScrollHandDrag);
}

} // namespace extension
} // namespace smtk
