//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/extension/qt/qtTableWidget.h"

#include <QKeyEvent>
#include <QHeaderView>

//-----------------------------------------------------------------------------
qtTableWidget::qtTableWidget(QWidget* p)
  : QTableWidget(p)
{
  //we want the table to always fill the frame
  this->horizontalHeader()->setStretchLastSection( true );
}

//-----------------------------------------------------------------------------
qtTableWidget::~qtTableWidget()
{
}

//----------------------------------------------------------------------------
void qtTableWidget::keyPressEvent(QKeyEvent* e)
{
  emit this->keyPressed(e);
}
