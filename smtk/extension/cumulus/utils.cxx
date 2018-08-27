//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "utils.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QtCore/QByteArray>
#include <QtNetwork/QNetworkReply>

namespace cumulus
{

QString handleGirderError(QNetworkReply* reply, const QByteArray& bytes)
{
  QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes.constData());

  QString errorMessage;

  if (!jsonResponse.isObject())
  {
    errorMessage = reply->errorString();
  }
  else
  {
    const QJsonObject& object = jsonResponse.object();
    QString message = object.value("message").toString();
    if (!message.isEmpty())
      errorMessage = QString("Girder error: %1").arg(message);
    else
      errorMessage = QString(bytes);
  }

  return errorMessage;
}

} // end namespace
