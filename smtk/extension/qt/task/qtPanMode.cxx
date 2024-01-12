//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/qtPanMode.h"

#include "smtk/extension/qt/qtUtility.h"
#include "smtk/extension/qt/task/qtTaskEditor.h"
#include "smtk/extension/qt/task/qtTaskView.h"
#include "smtk/extension/qt/task/qtTaskViewConfiguration.h"

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
  qtTaskEditor* editor,
  qtTaskView* view,
  QToolBar* toolbar,
  QActionGroup* modeGroup)
  : Superclass("pan", editor, toolbar, modeGroup)
{
  (void)view;
  QColor background = toolbar->palette().window().color();
  QIcon panIcon(colorAdjustedIcon(mode_pan_svg(), background));
  this->modeAction()->setIcon(panIcon);
}

qtPanMode::~qtPanMode() = default;

bool qtPanMode::eventFilter(QObject* obj, QEvent* event)
{
  if (obj == m_editor->taskWidget())
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
              m_editor->taskWidget()->addModeSnapback(Qt::Key_Shift, "select"_token);
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
  // When in pan mode, grab certain events from the view widget.
  m_editor->taskWidget()->installEventFilter(this);
}

void qtPanMode::exitMode()
{
  // When not in pan mode, do not process events from the view widget.
  m_editor->taskWidget()->removeEventFilter(this);
}

} // namespace extension
} // namespace smtk
