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
#include "qtTimeZoneRegionModel.h"

#include <QModelIndex>

// -----------------------------------------------------------------------------
namespace smtk {
  namespace extension {


//-----------------------------------------------------------------------------
qtTimeZoneSelectWidget::qtTimeZoneSelectWidget(QWidget* parent)
  : QWidget(parent)
{
  this->UI = new Ui_qtTimeZoneSelectWidget;
  this->UI->setupUi(this);

  qtTimeZoneRegionModel *model = new qtTimeZoneRegionModel(this);
  model->initialize();
  this->UI->ContinentView->setModel(model);
  QModelIndex rootIndex = model->index(0, 0);
  this->UI->ContinentView->setRootIndex(rootIndex);

  this->UI->RegionView->setModel(model);
  QModelIndex testIndex = model->index(1, 0, rootIndex);
  this->UI->RegionView->setRootIndex(testIndex);
}

//-----------------------------------------------------------------------------
qtTimeZoneSelectWidget::~qtTimeZoneSelectWidget()
{
}

  }  // namespace extension
}  // namespace smtk
