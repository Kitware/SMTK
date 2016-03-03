#ifndef __smtk_extension_cumulus_cumuluswidget_h
#define __smtk_extension_cumulus_cumuluswidget_h

#include "logindialog.h"

#include <QtGui/QMainWindow>
#include <QtNetwork/QNetworkReply>

class QAction;
class QIcon;
class QLabel;
class QTimer;

namespace Ui {
class CumulusWidget;
}

namespace cumulus
{
class JobTableModel;
class CumulusProxy;

class CumulusWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CumulusWidget(QWidget *parentObject = 0);
  ~CumulusWidget();

  void girderUrl(const QString &url);

signals:
  void info(const QString &msg);

protected:
  void createJobTable();

  Ui::CumulusWidget *m_ui;

private slots:
  void startJobFetchLoop();
  void displayAuthError(const QString &msg);
  void handleError(const QString &msg, QNetworkReply *networkReply);

private:
  JobTableModel *m_jobTableModel;
  CumulusProxy *m_cumulusProxy;
  QTimer *m_timer;
  LoginDialog m_loginDialog;
};

} // end namespace

#endif
