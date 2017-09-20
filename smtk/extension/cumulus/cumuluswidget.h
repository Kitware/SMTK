//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME cumuluswidget.h
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_extension_cumulus_cumuluswidget_h
#define __smtk_extension_cumulus_cumuluswidget_h

#include "logindialog.h"

#include <QString>

class QAction;
class QIcon;
class QLabel;
class QNetworkReply;
class QTimer;

namespace Ui
{
class CumulusWidget;
}

namespace cumulus
{
class JobTableModel;
class CumulusProxy;
class Job;

class CumulusWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CumulusWidget(QWidget* parentObject = 0);
  ~CumulusWidget();

  void girderUrl(const QString& url);
  bool isGirderRunning() const;
  void showLoginDialog();

  // Allows applications to add custom items to the context menu.
  // One QAction can be added per status type (created, queued, etc.)
  // The intended use is for applications to add an item to load/open
  // simulation results when the status is "downloaded".
  void addContextMenuAction(const QString& status, QAction* action);

signals:
  void info(const QString& msg);
  void resultDownloaded(const QString& path);

protected:
  void createJobTable();

  Ui::CumulusWidget* m_ui;

private slots:
  void startJobFetchLoop();
  void displayAuthError(const QString& msg);
  void handleError(const QString& msg, QNetworkReply* networkReply);
  void handleDownloadResult(const cumulus::Job&, const QString&);

private:
  JobTableModel* m_jobTableModel;
  CumulusProxy* m_cumulusProxy;
  QTimer* m_timer;
  LoginDialog m_loginDialog;
};

} // end namespace

#endif
