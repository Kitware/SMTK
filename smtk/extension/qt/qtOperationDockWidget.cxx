//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtOperationDockWidget.h"

#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"

#include "smtk/model/SessionRef.h"

#include <QCloseEvent>
#include <QScrollArea>

namespace smtk
{
namespace extension
{

qtOperationDockWidget::qtOperationDockWidget(QWidget* p)
  : QDockWidget(p)
{
  this->setObjectName("operatorsDockWidget");
  this->setWindowTitle("Operation Window");
  this->setFloating(true);
}

qtOperationDockWidget::~qtOperationDockWidget()
{
}

void qtOperationDockWidget::reset()
{
  smtk::model::SessionRef activeSession = qtActiveObjects::instance().activeModel().owningSession();
  if (activeSession.isValid())
  {
    this->setWindowTitle(activeSession.flagSummary().c_str());
  }
  else
  {
    this->setWindowTitle("Operation Window");
    this->hide();
  }

  auto scroll = qobject_cast<QScrollArea*>(this->widget());
  if (scroll && scroll->widget())
  {
    scroll->setWidgetResizable(true);
    auto operationWidget = qobject_cast<qtModelOperationWidget*>(scroll->widget());
    operationWidget->resetUI();
  }
}

void qtOperationDockWidget::closeEvent(QCloseEvent* clevent)
{
  emit this->closing();
  this->QDockWidget::closeEvent(clevent);
}

} // namespace model
} // namespace smtk
