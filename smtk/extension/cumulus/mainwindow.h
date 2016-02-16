#ifndef __smtk_extension_cumulus_mainwindow_h
#define __smtk_extension_cumulus_mainwindow_h

#include "logindialog.h"

#include <QtGui/QMainWindow>


class QAction;
class QIcon;
class QLabel;
class QNetworkReply;
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
  void createJobTable();
  void createMainMenu();
  void closeEvent(QCloseEvent *theEvent);

  Ui::MainWindow *m_ui;

private slots:
  void startJobFetchLoop();
  void displayAuthError(const QString &msg);
  void displayError(const QString &msg);

private:
  JobTableModel *m_jobTableModel;
  CumulusProxy *m_cumulusProxy;
  QTimer *m_timer;
  LoginDialog m_loginDialog;
};

} // end namespace

#endif
