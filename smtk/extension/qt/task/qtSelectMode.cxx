//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/qtSelectMode.h"

#include "smtk/extension/qt/qtUtility.h"
#include "smtk/extension/qt/task/qtTaskEditor.h"
#include "smtk/extension/qt/task/qtTaskView.h"
#include "smtk/extension/qt/task/qtTaskViewConfiguration.h"

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
  qtTaskEditor* editor,
  qtTaskView* view,
  QToolBar* toolbar,
  QActionGroup* modeGroup)
  : Superclass("select", editor, toolbar, modeGroup)
{
  (void)view;
  QColor background = toolbar->palette().window().color();
  QIcon selectionIcon(colorAdjustedIcon(mode_selection_svg(), background));
  this->modeAction()->setIcon(selectionIcon);
}

qtSelectMode::~qtSelectMode() = default;

bool qtSelectMode::eventFilter(QObject* obj, QEvent* event)
{
  if (obj == m_editor->taskWidget())
  {
    if (this->isModeActive())
    {
      if (event->type() == QEvent::KeyPress)
      {
        auto* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent && keyEvent->key() == Qt::Key_Shift)
        {
          m_editor->taskWidget()->addModeSnapback(Qt::Key_Shift, "select"_token);
        }
      }
    }
    return false;
  }
  return this->Superclass::eventFilter(obj, event);
}

void qtSelectMode::enterMode()
{
  m_editor->taskWidget()->setDragMode(QGraphicsView::RubberBandDrag);

  // When in connect mode, grab certain events from the view widget.
  m_editor->taskWidget()->installEventFilter(this);
}

void qtSelectMode::exitMode()
{
  // When not in connect mode, do not process events from the view widget.
  m_editor->taskWidget()->removeEventFilter(this);

  m_editor->taskWidget()->setDragMode(QGraphicsView::ScrollHandDrag);
}

} // namespace extension
} // namespace smtk
