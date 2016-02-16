#include "cumulusproxy.h"
#include "cJSON.h"
#include "job.h"

#include <QtCore/QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookie>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QTimer>

namespace cumulus {

CumulusProxy::CumulusProxy(QObject *parent) :
  QObject(parent)
{

}

CumulusProxy::~CumulusProxy()
{

}

void CumulusProxy::girderUrl(const QString &url)
{
  this->m_girderUrl = url;
}

void CumulusProxy::handleGirderError(QNetworkReply *reply,
    const QByteArray &bytes)
{
  cJSON *jsonReply = cJSON_Parse(bytes.constData());

  if (!reply) {
    emit error(reply->errorString());
    return;
  }

  char *msg = cJSON_GetObjectItem(jsonReply, "message")->valuestring;
  if (msg) {
    emit error(QString("Girder error: %1").arg(QString(msg)));
  }
  else {
    emit error(QString(bytes));
  }

  cJSON_Delete(jsonReply);

}

void CumulusProxy::authenticateNewt(const QString &username, const QString &password)
{
  QNetworkAccessManager *manager = new QNetworkAccessManager(this);

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
    this->handleGirderError(reply, bytes);
  }
  else {
    cJSON *reply = cJSON_Parse(bytes.constData());
    bool auth = cJSON_GetObjectItem(reply, "auth")->valueint == 1;

    if (auth) {
      char *sessionId = cJSON_GetObjectItem(reply, "newt_sessionid")->valuestring;
      m_newtSessionId.clear();
      m_newtSessionId.append(sessionId);

      // Now authenticate with Girder
      qDebug() << m_newtSessionId;
      this->authenticateGirder(m_newtSessionId);
    }
    else {
      emit newtAuthenticationError(QString("Invalid login"));
    }
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
  QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(authenticationGirderFinished(QNetworkReply *)));

  QByteArray empty;
  manager->put(request, empty);
}

void CumulusProxy::authenticationGirderFinished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    this->handleGirderError(reply, bytes);
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

    QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(fetchJobsFinished(QNetworkReply *)));

    manager->get(request);
}

void CumulusProxy::fetchJobsFinished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    this->handleGirderError(reply, bytes);
  }
  else {
    cJSON *jsonResponse = cJSON_Parse(bytes.constData());

    if (jsonResponse && jsonResponse->type == cJSON_Array) {
      QList<Job> jobs;

      for (cJSON* job = jsonResponse->child; job; job = job->next) {
        cJSON *idItem = cJSON_GetObjectItem(job, "_id");
        if (!idItem || idItem->type != cJSON_String) {
          continue;
        }
        QString id(idItem->valuestring);

        cJSON *nameItem = cJSON_GetObjectItem(job, "name");
        if (!nameItem || nameItem->type != cJSON_String) {
          continue;
        }
        QString name(nameItem->valuestring);

        cJSON *statusItem = cJSON_GetObjectItem(job, "status");
        if (!statusItem || statusItem->type != cJSON_String) {
          continue;
        }
        QString status(statusItem->valuestring);

        jobs.append(Job(id, name, status));
      }

      emit jobsUpdated(jobs);

    }
    else {
      emit error("Girder send JSON response of the incorrect format.");
    }

    cJSON_Delete(jsonResponse);
    this->sender()->deleteLater();
  }
}

bool CumulusProxy::isAuthenticated()
{
  return this->m_girderToken.isEmpty();
}



} // end namespace
