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

#include <QHeaderView>
#include <QKeyEvent>
using namespace smtk::extension;

qtTableWidget::qtTableWidget(QWidget* p)
  : QTableWidget(p)
{
  // Table should always fills the frame and resizes to contents
  this->horizontalHeader()->setStretchLastSection(true);
  this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

qtTableWidget::~qtTableWidget() = default;

void qtTableWidget::keyPressEvent(QKeyEvent* e)
{
  Q_EMIT this->keyPressed(e);
}
