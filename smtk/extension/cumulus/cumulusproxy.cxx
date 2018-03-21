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
#include "girderrequest.h"
#include "job.h"
#include "jobrequest.h"
#include "utils.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QThread>
#include <QtCore/QUrlQuery>
#include <QtCore/QVariant>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace
{
// Local class to provide sleep method for wait loops
class qtSleeper : public QThread
{
public:
  static void msleep(unsigned long msecs) { QThread::msleep(msecs); }
};
} // namespace

namespace cumulus
{

CumulusProxy::CumulusProxy(QObject* parent)
  : QObject(parent)
  , m_networkManager(new QNetworkAccessManager(this))
{
}

CumulusProxy::~CumulusProxy()
{
}

void CumulusProxy::girderUrl(const QString& url)
{
  m_girderUrl = url;
}

bool CumulusProxy::isGirderRunning(int timeoutSec)
{
  bool running = false; // return value

  // Request system version
  QString girderVersionUrl = QString("%1/system/version").arg(m_girderUrl);

  QNetworkRequest request(girderVersionUrl);
  QNetworkReply* reply = m_networkManager->get(request);

  // Wait for reply to finish
  qtSleeper sleeper;
  int timeoutMsec = timeoutSec * 1000;
  int deltaMsec = 100;
  sleeper.msleep(deltaMsec);
  for (int waitMsec = 0; !reply->isFinished() && waitMsec < timeoutMsec; waitMsec += deltaMsec)
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

void CumulusProxy::authenticateNewt(const QString& username, const QString& password)
{
  QUrl url("https://newt.nersc.gov/newt/auth");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

  QUrlQuery params;
  params.addQueryItem("username", username);
  params.addQueryItem("password", password);

  auto reply = m_networkManager->post(request, params.query().toUtf8());
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(authenticationNewtFinished()));
  QObject::connect(reply, SIGNAL(sslErrors(const QList<QSslError>&)), this,
    SLOT(sslErrors(const QList<QSslError>&)));
}

void CumulusProxy::authenticationNewtFinished()
{
  auto reply = qobject_cast<QNetworkReply*>(this->sender());
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply, bytes), reply);
  }
  else
  {
    cJSON* reply = cJSON_Parse(bytes.constData());
    bool auth = cJSON_GetObjectItem(reply, "auth")->valueint == 1;

    if (auth)
    {
      char* sessionId = cJSON_GetObjectItem(reply, "newt_sessionid")->valuestring;
      m_newtSessionId.clear();
      m_newtSessionId.append(sessionId);

      // Now authenticate with Girder
      this->authenticateGirder(m_newtSessionId);
    }
    else
    {
      emit newtAuthenticationError(QString("Invalid login"));
    }

    cJSON_Delete(reply);
  }

  reply->deleteLater();
}

void CumulusProxy::authenticateGirder(const QString& /*newtSessionId*/)
{
  m_girderToken.clear();

  QString girderAuthUrl = QString("%1/newt/authenticate/%2").arg(m_girderUrl).arg(m_newtSessionId);

  QNetworkRequest request(girderAuthUrl);

  QByteArray empty;
  auto reply = m_networkManager->put(request, empty);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(authenticationGirderFinished()));
}

void CumulusProxy::authenticationGirderFinished()
{
  auto reply = qobject_cast<QNetworkReply*>(this->sender());
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply, bytes), reply);
  }
  else
  {
    QVariant v = reply->header(QNetworkRequest::SetCookieHeader);
    QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(v);
    foreach (QNetworkCookie cookie, cookies)
    {
      if (cookie.name() == "girderToken")
      {
        m_girderToken = cookie.value();
      }
    }

    if (m_girderToken.isEmpty())
    {
      emit error(QString("Girder response did not set girderToken"));
    }
    else
    {
      emit authenticationFinished();
    }
  }

  reply->deleteLater();
}

void CumulusProxy::fetchJobs()
{
  QString girderAuthUrl = QString("%1/jobs").arg(m_girderUrl);

  QNetworkRequest request(girderAuthUrl);
  request.setRawHeader(QByteArray("Girder-Token"), m_girderToken.toUtf8());

  auto reply = m_networkManager->get(request);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(fetchJobsFinished()));
}

void CumulusProxy::fetchJobsFinished()
{
  auto reply = qobject_cast<QNetworkReply*>(this->sender());
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply, bytes), reply);
  }
  else
  {
    //qDebug() << "fetchJobsReply:" << bytes.constData();
    cJSON* jsonResponse = cJSON_Parse(bytes.constData());

    if (jsonResponse && jsonResponse->type == cJSON_Array)
    {
      QList<Job> jobs;

      for (cJSON* jsonJob = jsonResponse->child; jsonJob; jsonJob = jsonJob->next)
      {
        Job job = Job::fromJSON(jsonJob);

        if (job.isValid())
        {
          jobs.append(job);
        }
        else
        {
          emit error(QString("Error parsing job JSON."));
        }
      }

      emit jobsUpdated(jobs);
    }
    else
    {
      emit error(QString("Girder send JSON response of the incorrect format."));
    }

    cJSON_Delete(jsonResponse);
  }

  reply->deleteLater();
}

bool CumulusProxy::isAuthenticated()
{
  return !m_girderToken.isEmpty();
}

void CumulusProxy::fetchJob(const QString& id)
{
  QString girderAuthUrl = QString("%1/jobs/%2").arg(m_girderUrl).arg(id);

  QNetworkRequest request(girderAuthUrl);
  request.setRawHeader(QByteArray("Girder-Token"), m_girderToken.toUtf8());

  auto reply = m_networkManager->get(request);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(fetchJobStatusFinished()));
}

void CumulusProxy::fetchJobFinished()
{
  auto reply = qobject_cast<QNetworkReply*>(this->sender());
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply, bytes), reply);
  }
  else
  {
    cJSON* jsonResponse = cJSON_Parse(bytes.constData());

    if (jsonResponse)
    {
      Job job = Job::fromJSON(jsonResponse);

      if (job.isValid())
      {
        emit jobUpdated(job);
      }
      else
      {
        emit error(QString("Error parsing job JSON."));
      }
    }
    else
    {
      emit error(QString("Girder send JSON response of the incorrect format."));
    }

    cJSON_Delete(jsonResponse);
  }
  reply->deleteLater();
}

void CumulusProxy::deleteJob(Job job)
{
  DeleteJobRequest* request =
    new DeleteJobRequest(m_networkManager, m_girderUrl, m_girderToken, job, this);
  connect(request, SIGNAL(complete()), this, SLOT(deleteJobFinished()));
  connect(request, SIGNAL(error(const QString, QNetworkReply*)), this,
    SIGNAL(error(const QString, QNetworkReply*)));
  request->send();
}

void CumulusProxy::deleteJobFinished()
{
  DeleteJobRequest* request = qobject_cast<DeleteJobRequest*>(this->sender());

  emit jobDeleted(request->job());
  request->deleteLater();
  this->fetchJobs();
}

void CumulusProxy::patchJobs(QList<Job> jobs)
{
  // Patch only applies to cmb-specific data, stored as job metadata
  foreach (Job job, jobs)
  {
    //qDebug() << "Patching job" << job.id();
    cJSON* body = cJSON_CreateObject();

    cJSON* cmbData = job.cmbDataToJSON();
    cJSON_AddItemToObject(body, "metadata", cmbData);

    PatchJobRequest* updateRequest =
      new PatchJobRequest(m_networkManager, m_girderUrl, m_girderToken, job, body);

    connect(updateRequest, SIGNAL(error(const QString, QNetworkReply*)), this,
      SIGNAL(error(const QString, QNetworkReply*)));
    connect(updateRequest, SIGNAL(info(const QString)), this, SIGNAL(info(const QString)));
    updateRequest->send();
    cJSON_Delete(body);
  }
}

void CumulusProxy::terminateJob(Job job)
{
  TerminateJobRequest* request =
    new TerminateJobRequest(m_networkManager, m_girderUrl, m_girderToken, job, this);
  connect(request, SIGNAL(complete()), this, SLOT(terminateJobFinished()));
  connect(request, SIGNAL(error(const QString, QNetworkReply*)), this,
    SIGNAL(error(const QString, QNetworkReply*)));
  request->send();
}

void CumulusProxy::terminateJobFinished()
{
  TerminateJobRequest* request = qobject_cast<TerminateJobRequest*>(sender());

  emit jobTerminated(request->job());
  request->deleteLater();
  this->fetchJobs();
}

void CumulusProxy::sslErrors(const QList<QSslError>& /*errors*/)
{
  auto reply = qobject_cast<QNetworkReply*>(this->sender());
  emit error(reply->errorString());
  reply->deleteLater();
}

void CumulusProxy::downloadJob(const QString& downloadDirectory, Job job)
{

  DownloadJobRequest* request = new DownloadJobRequest(
    m_networkManager, m_girderUrl, m_girderToken, downloadDirectory, job, this);
  connect(request, SIGNAL(complete()), this, SLOT(downloadJobFinished()));
  connect(request, SIGNAL(error(const QString, QNetworkReply*)), this,
    SIGNAL(error(const QString, QNetworkReply*)));
  connect(request, SIGNAL(info(const QString)), this, SIGNAL(info(const QString)));
  request->send();
}

void CumulusProxy::downloadJobFinished()
{
  DownloadJobRequest* request = qobject_cast<DownloadJobRequest*>(sender());
  Job job = request->job();
  job.setDownloadFolder(request->path());

  emit jobDownloaded(request->job(), request->path());
  emit info("Job download complete.");
  request->deleteLater();

  // Update status to "downloaded"
  cJSON* body = cJSON_CreateObject();
  cJSON_AddStringToObject(body, "status", "downloaded");

  // Update cmb-specific data
  cJSON* cmbData = job.cmbDataToJSON();
  cJSON_AddItemToObject(body, "metadata", cmbData);

  PatchJobRequest* updateRequest =
    new PatchJobRequest(m_networkManager, m_girderUrl, m_girderToken, request->job(), body);

  connect(updateRequest, SIGNAL(error(const QString, QNetworkReply*)), this,
    SIGNAL(error(const QString, QNetworkReply*)));
  connect(updateRequest, SIGNAL(info(const QString)), this, SIGNAL(info(const QString)));
  updateRequest->send();
  cJSON_Delete(body);
}

} // end namespace
