//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/cumulus/pqCumulusJobsPanel.h"

#include "smtk/extension/cumulus/cumuluswidget.h"
#include "smtk/extension/cumulus/job.h"

#include <QAction>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSslSocket>
#include <QString>
#include <QVBoxLayout>
#include <QtGlobal>

class pqCumulusJobsPanel::pqCumulusJobsPanelInternal
{
public:
  pqCumulusJobsPanelInternal();

  QWidget* MainWidget;
  QVBoxLayout* MainLayout;

  QWidget* FirstWidget;
  QLineEdit* CumulusUrlEdit;

  cumulus::CumulusWidget* CumulusWidget;
};

pqCumulusJobsPanel::pqCumulusJobsPanelInternal::pqCumulusJobsPanelInternal()
  : MainWidget(0)
  , MainLayout(0)
  , FirstWidget(0)
  , CumulusUrlEdit(0)
  , CumulusWidget(0)
{
}

pqCumulusJobsPanel::pqCumulusJobsPanel(QWidget* parent)
  : QDockWidget(parent)
{
  this->setObjectName("CumulusJobs");
  this->setWindowTitle("Cumulus Job Tracker");
  this->Internal = new pqCumulusJobsPanelInternal;

  // Initialize main widget
  this->Internal->MainWidget = new QWidget(parent);
  this->Internal->MainLayout = new QVBoxLayout;

  // Instantiate first page (displayed at startup)
  this->Internal->FirstWidget = new QWidget(this->Internal->MainWidget);
  QFormLayout* firstLayout = new QFormLayout;
  this->Internal->CumulusUrlEdit = new QLineEdit(this->Internal->FirstWidget);
  this->Internal->CumulusUrlEdit->setText("http://localhost:8080/api/v1");
  firstLayout->addRow("Cumulus URL", this->Internal->CumulusUrlEdit);
  QPushButton* authenticateButton = new QPushButton("Authenticate", this->Internal->FirstWidget);
  firstLayout->addRow("NERSC Logon", authenticateButton);
  this->Internal->FirstWidget->setLayout(firstLayout);
  this->Internal->MainLayout->addWidget(this->Internal->FirstWidget);

  // Instantiate cumulus-monitoring objects
  this->Internal->CumulusWidget = new cumulus::CumulusWidget(this);
  this->Internal->MainLayout->addWidget(this->Internal->CumulusWidget);
  connect(this->Internal->CumulusWidget, &cumulus::CumulusWidget::info, this,
    &pqCumulusJobsPanel::infoSlot);

  // Add context menu item to load results
  QAction* action = new QAction("Load Simulation Results", this);
  this->Internal->CumulusWidget->addContextMenuAction("downloaded", action);
  QObject::connect(
    action, &QAction::triggered, this, &pqCumulusJobsPanel::requestSimulationResults);

  QObject::connect(authenticateButton, SIGNAL(clicked()), this, SLOT(authenticateHPC()));
  connect(this->Internal->CumulusWidget, SIGNAL(resultDownloaded(const QString&)), this,
    SIGNAL(resultDownloaded(const QString&)));

  // Finish main widget
  this->Internal->MainWidget->setLayout(this->Internal->MainLayout);
  this->setWidget(this->Internal->MainWidget);

  this->setObjectName("Jobs Panel");
}

pqCumulusJobsPanel::~pqCumulusJobsPanel()
{
  delete this->Internal;
}

void pqCumulusJobsPanel::infoSlot(const QString& msg)
{
  // Would like to emit signal to main window status bar
  // but that is not currently working. Instead, qInfo()
  // messages are sent to the CMB Log Window:
  qInfo() << "JobsPanel:" << msg;
}

void pqCumulusJobsPanel::authenticateHPC()
{
  // Check for SSL
  if (!QSslSocket::supportsSsl())
  {
    QMessageBox::critical(NULL, QObject::tr("SSL support"),
      QObject::tr("SSL support is required, you must rebuild Qt with SSL support."));
    return;
  }

  // Get cumulus/girder url
  QString cumulusUrl = this->Internal->CumulusUrlEdit->text();
  this->Internal->CumulusWidget->girderUrl(cumulusUrl);

  // Check for cumulus server
  if (!this->Internal->CumulusWidget->isGirderRunning())
  {
    QString msg = QString("Cumulus server NOT FOUND at %1").arg(cumulusUrl);
    QMessageBox::critical(NULL, QObject::tr("Cumulus Server Not Found"), msg);
    return;
  }

  // Open NERSC login dialog
  this->Internal->CumulusWidget->showLoginDialog();
}

void pqCumulusJobsPanel::requestSimulationResults()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action)
  {
    QMessageBox::critical(this, "Error", "Not connected to QAction?");
    return;
  }

  cumulus::Job job = action->data().value<cumulus::Job>();
  // QString message;
  // QTextStream qs(&message);
  // qs << "The slot named test() was called for job " << job.id() << ".\n\n"
  //    << "Download folder: " << job.downloadFolder();
  // QMessageBox::information(this, "Test", message);

  emit this->loadSimulationResults(job.downloadFolder());
}
