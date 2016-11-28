//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtTimeZoneSelectWidget.h"

#include "ui_qtTimeZoneSelectWidget.h"

// -----------------------------------------------------------------------------
namespace smtk {
  namespace extension {


//-----------------------------------------------------------------------------
qtTimeZoneSelectWidget::qtTimeZoneSelectWidget(QWidget* parent)
  : QWidget(parent)
{
  this->UI = new Ui_qtTimeZoneSelectWidget;
  this->UI->setupUi(this);
}

//-----------------------------------------------------------------------------
qtTimeZoneSelectWidget::~qtTimeZoneSelectWidget()
{
}

  }  // namespace extension
}  // namespace smtk
