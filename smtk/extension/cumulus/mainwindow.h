//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME mainwindow.h
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_extension_cumulus_mainwindow_h
#define __smtk_extension_cumulus_mainwindow_h

#include <QMainWindow>

class QAction;
class QIcon;
class QLabel;
class QTimer;

namespace Ui {
class MainWindow;
}

namespace cumulus
{
class JobTableModel;
class CumulusProxy;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

  void girderUrl(const QString &url);

protected:
  void createMainMenu();
  void closeEvent(QCloseEvent *theEvent);


private slots:
  void displayInfo(const QString &msg);

private:
  Ui::MainWindow *m_ui;
};

} // end namespace

#endif
