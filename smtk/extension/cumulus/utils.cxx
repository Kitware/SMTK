#include "utils.h"
#include <QtNetwork/QNetworkReply>
#include <QtCore/QByteArray>

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
    char *msg = cJSON_GetObjectItem(jsonReply, "message")->valuestring;
    if (msg) {
      errorMessage = QString("Girder error: %1").arg(QString(msg));
    }
    else {
      errorMessage = QString(bytes);
    }
  }

  cJSON_Delete(jsonReply);

  return errorMessage;
}

} // end namespace
