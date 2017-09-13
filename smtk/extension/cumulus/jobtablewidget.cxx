//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "ui_jobtablewidget.h"

#include "cumulusproxy.h"
#include "jobtablemodel.h"
#include "jobtablewidget.h"

#include <QMessageBox>
#include <QSortFilterProxyModel>

namespace cumulus
{

JobTableWidget::JobTableWidget(QWidget* parentObject)
  : QWidget(parentObject)
  , ui(new Ui::JobTableWidget)
  , m_proxyModel(NULL)
{
  ui->setupUi(this);

  ui->table->setSortingEnabled(true);
}

JobTableWidget::~JobTableWidget()
{
  delete ui;
}

void JobTableWidget::setModel(QAbstractItemModel* model)
{
  this->m_proxyModel = new QSortFilterProxyModel(this);
  this->m_proxyModel->setSourceModel(model);
  ui->table->setModel(this->m_proxyModel);
}

void JobTableWidget::setCumulusProxy(CumulusProxy* cumulusProxy)
{
  ui->table->setCumulusProxy(cumulusProxy);
}

void JobTableWidget::addContextMenuAction(const QString& status, QAction* action)
{
  ui->table->addContextMenuAction(status, action);
}

} // end namespace
