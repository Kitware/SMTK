//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "mainwindow.h"
#include "job.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QStatusBar>
#include <QString>
#include <QTextStream>

//#define TEST_CUSTOM_MENU 1

namespace cumulus
{

MainWindow::MainWindow()
  : m_ui(new Ui::MainWindow)
{
  m_ui->setupUi(this);

  QRect screenGeometry = QApplication::desktop()->screenGeometry();
  int x = (screenGeometry.width() - this->width()) / 2;
  int y = (screenGeometry.height() - this->height()) / 2;
  this->move(x, y);

  this->createMainMenu();

  connect(this->m_ui->cumulusWidget, SIGNAL(info(QString)), this, SLOT(displayInfo(QString)));

#ifdef TEST_CUSTOM_MENU
  QAction* action = new QAction("Custom/Complete", this);
  m_ui->cumulusWidget->addContextMenuAction("complete", action);
  QObject::connect(action, &QAction::triggered, this, &MainWindow::test);
#endif

  m_ui->cumulusWidget->showLoginDialog();
}

MainWindow::~MainWindow()
{
  delete m_ui;
}

void MainWindow::girderUrl(const QString& url)
{
  this->m_ui->cumulusWidget->girderUrl(url);
}

void MainWindow::createMainMenu()
{
  connect(m_ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void MainWindow::closeEvent(QCloseEvent* theEvent)
{
  qApp->quit();
}

void MainWindow::displayInfo(const QString& msg)
{
  this->statusBar()->showMessage(msg);
}

void MainWindow::test()
{
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action)
  {
    QMessageBox::critical(this, "Test", "Not connected to QAction?");
    return;
  }

  Job job = action->data().value<Job>();
  QString message;
  QTextStream qs(&message);
  qs << "The slot named test() was called for job " << job.id();
  QMessageBox::information(this, "Test", message);
}

} // end namespace
