//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "cumuluswidget.h"
#include "ui_cumuluswidget.h"
#include "jobtablemodel.h"
#include "cumulusproxy.h"

#include <QDesktopWidget>
#include <QMessageBox>
#include <QNetworkReply>
#include <QList>
#include <QTimer>


namespace cumulus {

CumulusWidget::CumulusWidget(QWidget *parentObject)
  : QWidget(parentObject),
    m_ui(new Ui::CumulusWidget),
    m_loginDialog(this),
    m_jobTableModel(new JobTableModel(this)),
    m_cumulusProxy(new CumulusProxy(this)),
    m_timer(NULL)
{
  m_ui->setupUi(this);

  this->createJobTable();

  connect(&m_loginDialog, SIGNAL(entered(QString, QString)),
            this->m_cumulusProxy, SLOT(authenticateNewt(QString, QString)));
  connect(this->m_cumulusProxy, SIGNAL(authenticationFinished()),
          this, SLOT(startJobFetchLoop()));
  connect(this->m_cumulusProxy, SIGNAL(jobsUpdated(QList<Job>)),
          this->m_jobTableModel, SLOT(jobsUpdated(QList<Job>)));
  connect(this->m_cumulusProxy, SIGNAL(error(QString, QNetworkReply*)),
          this, SLOT(handleError(QString, QNetworkReply*)));
  connect(this->m_cumulusProxy, SIGNAL(newtAuthenticationError(QString)),
          this, SLOT(displayAuthError(QString)));
  connect(this->m_cumulusProxy, SIGNAL(info(QString)),
            this, SIGNAL(info(QString)));
  connect(this->m_cumulusProxy, SIGNAL(jobDownloaded(cumulus::Job, const QString &)),
            this, SLOT(handleDownloadResult(cumulus::Job, const QString &)));
}

CumulusWidget::~CumulusWidget()
{
  delete m_ui;
}

void CumulusWidget::girderUrl(const QString &url)
{
  this->m_cumulusProxy->girderUrl(url);
}

bool CumulusWidget::isGirderRunning() const
{
  return this->m_cumulusProxy->isGirderRunning();
}

void CumulusWidget::showLoginDialog()
{
  m_loginDialog.show();
}

void CumulusWidget::createJobTable()
{
  this->m_ui->jobTableWidget->setModel(this->m_jobTableModel);
  this->m_ui->jobTableWidget->setCumulusProxy(this->m_cumulusProxy);
}

void CumulusWidget::startJobFetchLoop()
{
  this->m_cumulusProxy->fetchJobs();
  this->m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this->m_cumulusProxy, SLOT(fetchJobs()));
  this->m_timer->start(10000);
}

void CumulusWidget::displayAuthError(const QString &msg)
{
  this->m_loginDialog.setErrorMessage(msg);
  this->m_loginDialog.show();
}

void CumulusWidget::handleError(const QString &msg,
    QNetworkReply *networkReply)
{
  if (networkReply) {
    int statusCode = networkReply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).value<int>();

    // Forbidden, ask the user to authenticate again
    if (statusCode == 403) {
      this->m_loginDialog.show();
    }
    else if (networkReply->error() == QNetworkReply::ConnectionRefusedError) {
      QMessageBox::critical(NULL, QObject::tr("Connection refused"),
             QObject::tr("Unable to connect to server at %1:%2, please ensure server is running.")
                .arg(networkReply->url().host()).arg(networkReply->url().port()));
      this->m_loginDialog.show();
    }
    else {
        QMessageBox::critical(this, "", msg, QMessageBox::Ok);
    }
  }
  else {
    QMessageBox::critical(this, "", msg, QMessageBox::Ok);
  }
}

void CumulusWidget::handleDownloadResult(const cumulus::Job& job, const QString &path)
{
  (void)job;
  emit this->resultDownloaded(path);
}

} // end namespace
