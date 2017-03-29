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
#include <QtCore/QByteArray>
#include <QtNetwork/QNetworkReply>

#include "cJSON.h"

namespace cumulus
{

QString handleGirderError(QNetworkReply *reply,
    const QByteArray &bytes)
{
  cJSON *jsonReply = cJSON_Parse(bytes.constData());
  QString errorMessage;

  if (!jsonReply) {
    errorMessage =  reply->errorString();
  }
  else {
    cJSON *msgItem = cJSON_GetObjectItem(jsonReply, "message");
    if (msgItem) {
      errorMessage = QString("Girder error: %1").arg(QString(msgItem->valuestring));
    }
    else {
      errorMessage = QString(bytes);
    }
  }

  cJSON_Delete(jsonReply);

  return errorMessage;
}

} // end namespace
