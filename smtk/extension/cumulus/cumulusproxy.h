//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME cumulusproxy.h
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_extension_cumulus_cumulusproxy_h
#define __smtk_extension_cumulus_cumulusproxy_h

#include "smtk/extension/cumulus/Exports.h"
#include "smtk/extension/cumulus/job.h"

#include <QList>
#include <QObject>

class QNetworkCookieJar;
class QNetworkReply;
class QSslError;
class QNetworkAccessManager;

namespace cumulus
{

class SMTKCUMULUSEXT_EXPORT CumulusProxy : public QObject
{
  Q_OBJECT

public:
  CumulusProxy(QObject* parent = 0);
  ~CumulusProxy();

  void girderUrl(const QString& url);
  bool isGirderRunning(int timeoutSec = 5);

public slots:
  void authenticateGirder(const QString& newtSessionId);
  void authenticateNewt(const QString& username, const QString& password);
  bool isAuthenticated();
  void fetchJobs();
  void fetchJob(const QString& id);
  void deleteJob(Job job);
  void patchJobs(QList<Job> jobs);
  void terminateJob(Job job);
  void downloadJob(const QString& downloadDirectory, Job job);

signals:
  void jobsUpdated(QList<Job> jobs);
  void newtAuthenticationError(const QString& msg);
  void authenticationFinished();
  void error(const QString& msg, QNetworkReply* networkReply = NULL);
  void jobUpdated(cumulus::Job job);
  void jobDeleted(cumulus::Job job);
  void jobTerminated(cumulus::Job job);
  void jobDownloaded(cumulus::Job job, const QString& path);
  void info(const QString& msg);

private slots:
  void authenticationNewtFinished();
  void authenticationGirderFinished();
  void fetchJobsFinished();
  void fetchJobFinished();
  void deleteJobFinished();
  void terminateJobFinished();
  void sslErrors(const QList<QSslError>& errors);
  void downloadJobFinished();

private:
  QString m_girderUrl;
  QString m_newtSessionId;
  QString m_girderToken;
  QNetworkAccessManager* m_networkManager;
};

} // end namespace

#endif
