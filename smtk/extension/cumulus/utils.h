#ifndef __smtk_extension_cumulus_utils_h
#define __smtk_extension_cumulus_utils_h

class QNetworkReply;
class QByteArray;
class QString;

namespace cumulus
{
  QString handleGirderError(QNetworkReply *reply, const QByteArray &bytes);

}

#endif
