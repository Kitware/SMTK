//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "cumulusproxy.h"
#include "cJSON.h"
#include "job.h"
#include "utils.h"
#include "jobrequest.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkCookieJar>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QThread>
#include <QtCore/QTimer>

namespace
{
// Local class to provide sleep method for wait loops
class qtSleeper : public QThread
{
public:
  static void msleep(unsigned long msecs)
  {
    QThread::msleep(msecs);
  }
};
}  // namespace

namespace cumulus {

CumulusProxy::CumulusProxy(QObject *parent) :
  QObject(parent), m_cookieJar(new QNetworkCookieJar())
{

}

CumulusProxy::~CumulusProxy()
{
  delete this->m_cookieJar;
}

void CumulusProxy::girderUrl(const QString &url)
{
  this->m_girderUrl = url;
}

bool CumulusProxy::isGirderRunning(int timeoutSec)
{
  bool running = false;  // return value

  // Request system version
  QNetworkAccessManager *manager = new QNetworkAccessManager(this);
  QString girderVersionUrl = QString("%1/system/version")
      .arg(this->m_girderUrl);

  QNetworkRequest request(girderVersionUrl);
  QNetworkReply *reply = manager->get(request);

  // Wait for reply to finish
  qtSleeper sleeper;
  int timeoutMsec = timeoutSec * 1000;
  int deltaMsec = 100;
  sleeper.msleep(deltaMsec);
  for (int waitMsec=0;
       !reply->isFinished() && waitMsec < timeoutMsec;
       waitMsec += deltaMsec)
    {
    QCoreApplication::sendPostedEvents();
#ifdef WIN32
    QCoreApplication::processEvents();
#endif
    //qDebug() << "wait" << waitMsec << "msec";
    sleeper.msleep(deltaMsec);
    }

  // Check for timeout
  if (reply->isFinished() && reply->error() == QNetworkReply::NoError)
    {
    running = true;
    }

  reply->deleteLater();
  return running;
}

void CumulusProxy::authenticateNewt(const QString &username, const QString &password)
{
  QNetworkAccessManager *manager = new QNetworkAccessManager(this);
  manager->setCookieJar(this->m_cookieJar);
  this->m_cookieJar->setParent(NULL);

  QUrl url("https://newt.nersc.gov/newt/auth");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

  QUrl params;
  params.addQueryItem("username", username);
  params.addQueryItem("password", password);

  QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(authenticationNewtFinished(QNetworkReply *)));

  manager->post(request, params.encodedQuery());
}

void CumulusProxy::authenticationNewtFinished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    emit error(handleGirderError(reply, bytes), reply);

  }
  else {
    cJSON *reply = cJSON_Parse(bytes.constData());
    bool auth = cJSON_GetObjectItem(reply, "auth")->valueint == 1;

    if (auth) {
      char *sessionId = cJSON_GetObjectItem(reply, "newt_sessionid")->valuestring;
      m_newtSessionId.clear();
      m_newtSessionId.append(sessionId);

      // Now authenticate with Girder
      this->authenticateGirder(m_newtSessionId);
    }
    else {
      emit newtAuthenticationError(QString("Invalid login"));
    }

    cJSON_Delete(reply);
  }

  this->sender()->deleteLater();
}

void CumulusProxy::authenticateGirder(const QString &newtSessionId)
{
  m_girderToken.clear();

  QNetworkAccessManager *manager = new QNetworkAccessManager(this);

  QString girderAuthUrl = QString("%1/newt/authenticate/%2")
      .arg(this->m_girderUrl).arg(this->m_newtSessionId);

  QNetworkRequest request(girderAuthUrl);
  QObject::connect(manager, SIGNAL(finished(QNetworkReply *)),
      this, SLOT(authenticationGirderFinished(QNetworkReply *)));

  QByteArray empty;
  manager->put(request, empty);
}

void CumulusProxy::authenticationGirderFinished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    emit error(handleGirderError(reply, bytes), reply);
  }
  else {
    QVariant v = reply->header(QNetworkRequest::SetCookieHeader);
    QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(v);
    foreach (QNetworkCookie cookie, cookies) {
      if (cookie.name() == "girderToken") {
        m_girderToken = cookie.value();
      }
    }

    if (m_girderToken.isEmpty()) {
      emit error(QString("Girder response did not set girderToken"));
    }
    else {
      emit authenticationFinished();
    }
  }

  this->sender()->deleteLater();
}

void CumulusProxy::fetchJobs()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QString girderAuthUrl = QString("%1/jobs")
        .arg(this->m_girderUrl);

    QNetworkRequest request(girderAuthUrl);
    request.setRawHeader(QByteArray("Girder-Token"), this->m_girderToken.toUtf8());

    QObject::connect(manager, SIGNAL(finished(QNetworkReply *)),
        this, SLOT(fetchJobsFinished(QNetworkReply *)));

    manager->get(request);
}

void CumulusProxy::fetchJobsFinished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    emit error(handleGirderError(reply, bytes), reply);
  }
  else {
    cJSON *jsonResponse = cJSON_Parse(bytes.constData());

    if (jsonResponse && jsonResponse->type == cJSON_Array) {
      QList<Job> jobs;

      for (cJSON* jsonJob = jsonResponse->child; jsonJob; jsonJob = jsonJob->next) {
        Job job = Job::fromJSON(jsonJob);

        if (job.isValid()) {
          jobs.append(job);
        }
        else {
          emit error(QString("Error parsing job JSON."));
        }
      }

      emit jobsUpdated(jobs);

    }
    else {
      emit error(QString("Girder send JSON response of the incorrect format."));
    }

    cJSON_Delete(jsonResponse);
    this->sender()->deleteLater();
  }
}

bool CumulusProxy::isAuthenticated()
{
  return !this->m_girderToken.isEmpty();
}

void CumulusProxy::fetchJob(const QString &id)
{
  QNetworkAccessManager *manager = new QNetworkAccessManager(this);

  QString girderAuthUrl = QString("%1/jobs/%2")
      .arg(this->m_girderUrl).arg(id);

  QNetworkRequest request(girderAuthUrl);
  request.setRawHeader(QByteArray("Girder-Token"), this->m_girderToken.toUtf8());

  QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(fetchJobStatusFinished(QNetworkReply *)));

  manager->get(request);
}

void CumulusProxy::fetchJobFinished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    emit error(handleGirderError(reply, bytes), reply);
  }
  else {
    cJSON *jsonResponse = cJSON_Parse(bytes.constData());

   if (jsonResponse) {
     Job job = Job::fromJSON(jsonResponse);

     if (job.isValid()) {
       emit jobUpdated(job);
     }
     else {
       emit error(QString("Error parsing job JSON."));
     }
   }
   else {
     emit error(QString("Girder send JSON response of the incorrect format."));
   }

   cJSON_Delete(jsonResponse);
   this->sender()->deleteLater();
  }
}

void CumulusProxy::deleteJob(Job job)
{
  DeleteJobRequest *request = new DeleteJobRequest(this->m_girderUrl,
      this->m_girderToken, job, this);
  connect(request, SIGNAL(complete()), this, SLOT(deleteJobFinished()));
  connect(request, SIGNAL(error(const QString, QNetworkReply*)),
      this, SIGNAL(error(const QString, QNetworkReply*)));
  request->send();
}

void CumulusProxy::deleteJobFinished()
{
  DeleteJobRequest *request = qobject_cast<DeleteJobRequest*>(this->sender());

  emit jobDeleted(request->job());
  request->deleteLater();
  this->fetchJobs();
}

void CumulusProxy::terminateJob(Job job)
{
  TerminateJobRequest *request = new TerminateJobRequest(this->m_girderUrl,
      this->m_girderToken, job, this);
  connect(request, SIGNAL(complete()), this, SLOT(terminateJobFinished()));
  connect(request, SIGNAL(error(const QString, QNetworkReply*)),
      this, SIGNAL(error(const QString, QNetworkReply*)));
  request->send();
}

void CumulusProxy::terminateJobFinished()
{
  TerminateJobRequest *request = qobject_cast<TerminateJobRequest*>(sender());

  emit jobTerminated(request->job());
  request->deleteLater();
  this->fetchJobs();
}

void CumulusProxy::sslErrors(QNetworkReply * reply,
    const QList<QSslError> & errors)
{
  emit error(reply->errorString());
  this->sender()->deleteLater();
}


void CumulusProxy::downloadJob(const QString &downloadDirectory, Job job)
{

  DownloadJobRequest *request = new DownloadJobRequest(this->m_cookieJar,
      this->m_girderUrl, this->m_girderToken, downloadDirectory, job, this);
  connect(request, SIGNAL(complete()), this, SLOT(downloadJobFinished()));
  connect(request, SIGNAL(error(const QString, QNetworkReply*)),
      this, SIGNAL(error(const QString, QNetworkReply*)));
  connect(request, SIGNAL(info(const QString)),
      this, SIGNAL(info(const QString)));
  request->send();
}

void CumulusProxy::downloadJobFinished()
{
  DownloadJobRequest *request = qobject_cast<DownloadJobRequest*>(sender());

  emit jobDownloaded(request->job(), request->path());
  emit info("Job download complete.");
  request->deleteLater();
}

} // end namespace
