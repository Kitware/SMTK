#ifndef __smtk_extension_cumulus_jobrequest_h
#define __smtk_extension_cumulus_jobrequest_h

#include "job.h"
#include "girderrequest.h"

#include <QtCore/QList>
#include <QtCore/QSet>

class QNetworkReply;

namespace cumulus
{

class JobRequest : public GirderRequest
{
  Q_OBJECT

public:
  JobRequest(const QString &girderUrl, const QString &girderToken,
      Job job, QObject *parent = 0);
  ~JobRequest();

  void virtual send() = 0;
  Job job() const { return this->m_job; };

protected:
  Job m_job;

};

class DeleteJobRequest: public JobRequest
{
  Q_OBJECT

public:
  DeleteJobRequest(const QString &girderUrl, const QString &girderToken,
      Job job, QObject *parent = 0);
  ~DeleteJobRequest();

  void send();

private slots:
  void finished(QNetworkReply *reply);
};

class TerminateJobRequest: public JobRequest
{
  Q_OBJECT

public:
  TerminateJobRequest(const QString &girderUrl, const QString &girderToken,
      Job job, QObject *parent = 0);
  ~TerminateJobRequest();

  void send();

private slots:
  void finished(QNetworkReply *reply);

};

class DownloadJobRequest: public JobRequest
{
  Q_OBJECT

public:
  DownloadJobRequest(QNetworkCookieJar *cookieJar,const QString &girderUrl,
      const QString &girderToken, const QString &downloadPath, Job job,
      QObject *parent = 0);
  ~DownloadJobRequest();
  QString path() { return m_downloadPath; };

  void send();

private slots:
  void downloadFolderFinished();

private:
  QSet<QString> m_foldersToDownload;
  QString m_downloadPath;
  QNetworkCookieJar *m_cookieJar;



};


} // end namespace

#endif
