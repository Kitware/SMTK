//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtOperatorDockWidget.h"
#include <QCloseEvent>

// -----------------------------------------------------------------------------
namespace smtk {
  namespace extension {

//-----------------------------------------------------------------------------
qtOperatorDockWidget::qtOperatorDockWidget(QWidget* p)
  : QDockWidget(p)
{
  this->setObjectName("operatorsDockWidget");
  this->setWindowTitle("Operator Window");
  this->setFloating(true);
}

//-----------------------------------------------------------------------------
qtOperatorDockWidget::~qtOperatorDockWidget()
{
}

//-----------------------------------------------------------------------------
void qtOperatorDockWidget::closeEvent(QCloseEvent* clevent)
{
  emit this->closing();
  this->QDockWidget::closeEvent(clevent);
}

  } // namespace model
} // namespace smtk
