//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqToolboxEventTranslator.h"

#include "smtk/extension/qt/qtDoubleClickButton.h"
#include "smtk/extension/qt/qtOperationAction.h"
#include "smtk/extension/qt/qtOperationPalette.h"
#include "smtk/extension/qt/qtOperationTypeModel.h"
#include "smtk/extension/qt/qtOperationTypeView.h"

#include "pqApplicationCore.h"
#include "pqCoreTestUtility.h"
#include "pqCoreUtilities.h"
#include "pqEventTypes.h"
#include "pqFileDialog.h"

#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

pqToolboxEventTranslator::pqToolboxEventTranslator(QObject* p)
  : pqWidgetEventTranslator(p)
{
}

pqToolboxEventTranslator::~pqToolboxEventTranslator() = default;

bool pqToolboxEventTranslator::translateEvent(
  QObject* Object,
  QEvent* Event,
  int eventType,
  bool& error)
{
  // Only translate events for toolbox buttons, which are
  // qtDoubleClickButton instances with a qtOperationAction.
  auto* widget = qobject_cast<qtDoubleClickButton*>(Object);
  if (!widget)
  {
    return false;
  }
  auto actions = widget->actions();
  if (actions.empty())
  {
    return false;
  }
  qtOperationAction* action = nullptr;
  for (auto* aa : actions)
  {
    if ((action = qobject_cast<qtOperationAction*>(aa)))
    {
      break;
    }
  }
  if (!action)
  {
    return false;
  }
  // Find the owning qtOperationTypeView as it will emit
  // the event rather than the button.
  qtOperationTypeView* otview = nullptr;
  for (QWidget* pw = widget; pw && !otview; pw = qobject_cast<QWidget*>(pw->parent()))
  {
    otview = qobject_cast<qtOperationTypeView*>(pw);
  }
  if (!otview)
  {
    return false;
  }
  if (eventType == pqEventTypes::ACTION_EVENT)
  {
    auto opIndex = action->operationIndex();
    auto opName = action->operationModel()
                    ->findByTypeIndex(opIndex)
                    .siblingAtColumn(static_cast<int>(qtOperationTypeModel::Column::TypeName))
                    .data()
                    .toString();
    switch (Event->type())
    {
      case QEvent::MouseButtonRelease:
        Q_EMIT recordEvent(
          otview, "editOperationParameters", QString("(%1,%2)").arg(opIndex).arg(opName));
        return true;
        break;
      case QEvent::MouseButtonDblClick:
        Q_EMIT recordEvent(
          otview, "runOperationWithDefaults", QString("(%1,%2)").arg(opIndex).arg(opName));
        return true;
        break;
      default:
        break;
    }
  }
  return this->Superclass::translateEvent(Object, Event, eventType, error);
}
