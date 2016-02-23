#include "cumulusproxy.h"
#include "cJSON.h"
#include "jobrequest.h"
#include "utils.h"

#include <QtCore/QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookie>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QTimer>

namespace cumulus {

JobRequest::JobRequest(const QString &girderUrl, const QString &girderToken,
    Job job, QObject *parent) :
    GirderRequest(girderUrl, girderToken, parent),
  m_job(job)
{

}

JobRequest::~JobRequest()
{

}

DeleteJobRequest::DeleteJobRequest(const QString &girderUrl,
    const QString &girderToken, Job job, QObject *parent) :
    JobRequest(girderUrl, girderToken, job, parent)
{

}

DeleteJobRequest::~DeleteJobRequest()
{

}

void DeleteJobRequest::send()
{
  QString girderAuthUrl = QString("%1/jobs/%2")
      .arg(this->m_girderUrl).arg(this->m_job.id());

  QNetworkRequest request(girderAuthUrl);
  request.setRawHeader(QByteArray("Girder-Token"), this->m_girderToken.toUtf8());

  QObject::connect(this->m_networkManager, SIGNAL(finished(QNetworkReply *)),
      this, SLOT(finished(QNetworkReply *)));

  this->m_networkManager->deleteResource(request);
}

void DeleteJobRequest::finished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    emit error(handleGirderError(reply, bytes),
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).value<int>());
  }
  else {
    emit complete();
  }
}

TerminateJobRequest::TerminateJobRequest(const QString &girderUrl,
    const QString &girderToken, Job job, QObject *parent) :
    JobRequest(girderUrl, girderToken, job, parent)
{


}

TerminateJobRequest::~TerminateJobRequest()
{

}

void TerminateJobRequest::send()
{
  QString girderAuthUrl = QString("%1/jobs/%2/terminate")
      .arg(this->m_girderUrl).arg(this->m_job.id());

  QNetworkRequest request(girderAuthUrl);
  request.setRawHeader(QByteArray("Girder-Token"), this->m_girderToken.toUtf8());

  QObject::connect(this->m_networkManager, SIGNAL(finished(QNetworkReply *)),
      this, SLOT(finished(QNetworkReply *)));

  QByteArray empty;
  this->m_networkManager->put(request, empty);
}

void TerminateJobRequest::finished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    emit error(handleGirderError(reply, bytes),
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).value<int>());
  }
  else {
    emit complete();
  }
}

DownloadJobRequest::DownloadJobRequest(QNetworkCookieJar *cookieJar,
    const QString &girderUrl, const QString &girderToken,
    const QString &downloadPath, Job job, QObject *parent) :
  JobRequest(girderUrl, girderToken, job, parent),
  m_downloadPath(downloadPath), m_cookieJar(cookieJar)
{

}

DownloadJobRequest::~DownloadJobRequest()
{

}

void DownloadJobRequest::send()
{
  foreach(QString folderId, this->m_job.outputFolderIds()) {
    this->m_foldersToDownload << folderId;

    DownloadFolderRequest *request = new DownloadFolderRequest(this->m_cookieJar,
        this->m_girderUrl, this->m_girderToken, this->m_downloadPath,
        folderId, this);
    connect(request, SIGNAL(complete()), this, SLOT(downloadFolderFinished()));
    connect(request, SIGNAL(error(const QString, int)), this,
        SIGNAL(error(const QString, int)));
    connect(request, SIGNAL(info(const QString)), this,
        SIGNAL(info(const QString)));

    request->send();
  }
}

void DownloadJobRequest::downloadFolderFinished()
{
  DownloadFolderRequest *request
    = qobject_cast<DownloadFolderRequest*>(this->sender());

  this->m_foldersToDownload.remove(request->folderId());

  if (this->m_foldersToDownload.isEmpty()) {
    emit complete();
  }

  request->deleteLater();
}
}
