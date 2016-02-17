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

JobRequest::JobRequest(const QString &girderUrl,  const QString &girderToken,
    Job job, QObject *parent) :
  QObject(parent),
  m_job(job),
  m_girderUrl(girderUrl),
  m_girderToken(girderToken)
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
  QNetworkAccessManager *manager = new QNetworkAccessManager(this);

  QString girderAuthUrl = QString("%1/jobs/%2")
      .arg(this->m_girderUrl).arg(this->m_job.id());

  QNetworkRequest request(girderAuthUrl);
  request.setRawHeader(QByteArray("Girder-Token"), this->m_girderToken.toUtf8());

  QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(finished(QNetworkReply *)));

  manager->deleteResource(request);
}

void DeleteJobRequest::finished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    emit error(handleGirderError(reply, bytes));
  }
  else {
    emit complete();
  }

  this->sender()->deleteLater();
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
  QNetworkAccessManager *manager = new QNetworkAccessManager(this);

  QString girderAuthUrl = QString("%1/jobs/%2/terminate")
      .arg(this->m_girderUrl).arg(this->m_job.id());

  QNetworkRequest request(girderAuthUrl);
  request.setRawHeader(QByteArray("Girder-Token"), this->m_girderToken.toUtf8());

  QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(finished(QNetworkReply *)));

  QByteArray empty;
  manager->put(request, empty);
}

void TerminateJobRequest::finished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    emit error(handleGirderError(reply, bytes));
  }
  else {
    emit complete();
  }

  this->sender()->deleteLater();
}

}
