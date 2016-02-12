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


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

  void girderUrl(const QString &url);

public slots:

protected:
  void createJobTable();
  void createMainMenu();
  void closeEvent(QCloseEvent *theEvent);

  Ui::MainWindow *m_ui;

private slots:
  void fetchJobs();
  void authenticateNewt(const QString &username, const QString &password);
  void authenticationNewtFinished(QNetworkReply *reply);
  void authenticateGirder(const QString &newtSessionId);
  void authenticationGirderFinished(QNetworkReply *reply);
  void fetchJobsFinished(QNetworkReply *reply);
private:
  JobTableModel *m_jobTableModel;
  QTimer *m_timer;
  QString m_girderUrl;
  QString m_newtSessionId;
  QString m_girderToken;
  LoginDialog m_loginDialog;
};

} // end namespace

#endif
