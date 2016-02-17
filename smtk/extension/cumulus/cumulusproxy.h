#ifndef __smtk_extension_cumulus_cumulusproxy_h
#define __smtk_extension_cumulus_cumulusproxy_h

#include "job.h"

#include <QtCore/QList>

class QNetworkReply;

namespace cumulus
{

class CumulusProxy : public QObject
{
  Q_OBJECT

public:
  CumulusProxy(QObject *parent = 0);
  ~CumulusProxy();

  void girderUrl(const QString &url);

public slots:
  void authenticateGirder(const QString &newtSessionId);
  void authenticateNewt(const QString &username, const QString &password);
  bool isAuthenticated();
  void fetchJobs();
  void fetchJob(const QString &id);
  void deleteJob(Job job);
  void terminateJob(Job job);

signals:
  void jobsUpdated(QList<Job> jobs);
  void newtAuthenticationError(const QString &msg);
  void authenticationFinished();
  void error(const QString &msg);
  void jobUpdated(cumulus::Job job);
  void jobDeleted(cumulus::Job job);
  void jobTerminated(cumulus::Job job);

private slots:
  void authenticationNewtFinished(QNetworkReply *reply);
  void authenticationGirderFinished(QNetworkReply *reply);
  void fetchJobsFinished(QNetworkReply *reply);
  void fetchJobFinished(QNetworkReply *reply);
  void deleteJobFinished();
  void terminateJobFinished();


private:
  QString m_girderUrl;
  QString m_newtSessionId;
  QString m_girderToken;
};

} // end namespace

#endif
