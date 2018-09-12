//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <QByteArray>
#include <QDebug>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QVariant>

#include "girderauthenticator.h"

namespace cumulus
{

GirderAuthenticator::GirderAuthenticator(QNetworkAccessManager* networkManager, QObject* parent)
  : QObject(parent)
  , m_networkManager(networkManager)
{
}

GirderAuthenticator::~GirderAuthenticator() = default;

void GirderAuthenticator::authenticateApiKey(const QString& apiUrl, const QString& apiKey)
{
  // Only submit one request at a time.
  if (m_pendingReply)
    return;

  static const QString& tokenDuration = "90";

  QByteArray postData;
  postData.append(("key=" + apiKey + "&").toUtf8());
  postData.append(("duration=" + tokenDuration).toUtf8());

  QUrl url(QString("%1/api_key/token").arg(apiUrl));

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

  // No more authentication requests can be sent until m_pendingReply is null
  m_pendingReply.reset(m_networkManager->post(request, postData));

  connect(m_pendingReply.get(), &QNetworkReply::finished, this,
    &GirderAuthenticator::finishAuthentication);

  // Cache this so finishAuthentication can read it
  m_apiUrl = apiUrl;
}

void GirderAuthenticator::authenticatePassword(
  const QString& apiUrl, const QString& username, const QString& password)
{
  // Only submit one request at a time.
  if (m_pendingReply)
    return;

  QUrl url(QString("%1/user/authentication").arg(apiUrl));

  // Basic http authentication scheme
  QString concatenated = username + ":" + password;
  QByteArray data = concatenated.toLocal8Bit().toBase64();
  QString headerData = "Basic " + data;

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
  request.setRawHeader("Authorization", headerData.toLocal8Bit());

  // No more authentication requests can be sent until m_pendingReply is null
  m_pendingReply.reset(m_networkManager->get(request));

  connect(m_pendingReply.get(), &QNetworkReply::finished, this,
    &GirderAuthenticator::finishAuthentication);

  // Cache this so finishAuthentication can read it
  m_apiUrl = apiUrl;
}

void GirderAuthenticator::finishAuthentication()
{
  // This should be cached from earlier
  QString apiUrl = m_apiUrl;

  // Take ownership of the reply. This will also allow more authentication
  // requests to occur.
  std::unique_ptr<QNetworkReply> reply = std::move(m_pendingReply);

  QString girderToken;
  QString errorMessage;

  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    errorMessage += "Error: authentication failed!\n";
    errorMessage += "Response from server was:\n" + bytes + "\n";
    emit authenticationErrored(errorMessage);
    return;
  }
  else
  {
    QVariant v = reply->header(QNetworkRequest::SetCookieHeader);
    QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(v);
    foreach (QNetworkCookie cookie, cookies)
    {
      if (cookie.name() == "girderToken")
      {
        girderToken = cookie.value();
      }
    }

    if (girderToken.isEmpty())
    {
      errorMessage += "Error: Girder response did not set girderToken!\n";
      emit authenticationErrored(errorMessage);
      return;
    }
  }

  if (girderToken.isEmpty() || !errorMessage.isEmpty())
  {
    errorMessage += "Authentication failed!\n";
    emit authenticationErrored(errorMessage);
    return;
  }

  emit authenticationSucceeded(apiUrl, girderToken);
}

} // end namespace
