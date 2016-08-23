#include "girderrequest.h"
#include "utils.h"
#include "cJSON.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookieJar>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QDebug>


namespace cumulus {

GirderRequest::GirderRequest(const QString &girderUrl,
    const QString &girderToken, QObject *parent) :
  QObject(parent),
  m_girderUrl(girderUrl),
  m_girderToken(girderToken),
  m_networkManager(new QNetworkAccessManager(this))
{

}

GirderRequest::~GirderRequest()
{

}


ListItemsRequest::ListItemsRequest(const QString &girderUrl,
    const QString &girderToken, const QString folderId,
     QObject *parent) :
    GirderRequest(girderUrl, girderToken, parent),
    m_folderId(folderId)
{

}

ListItemsRequest::~ListItemsRequest()
{

}

void ListItemsRequest::send()
{
  QUrl url(QString("%1/item").arg(this->m_girderUrl));
  url.addQueryItem("folderId", this->m_folderId);


  QNetworkRequest request(url);
  request.setRawHeader(QByteArray("Girder-Token"), this->m_girderToken.toUtf8());

  QObject::connect(this->m_networkManager, SIGNAL(finished(QNetworkReply *)),
      this, SLOT(finished(QNetworkReply *)));

  this->m_networkManager->get(request);
}

void ListItemsRequest::finished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    emit error(handleGirderError(reply, bytes), reply);
  }
  else {
    cJSON *jsonResponse = cJSON_Parse(bytes.constData());

    if (!jsonResponse || jsonResponse->type != cJSON_Array) {
      emit error(QString("Invalid response to listItems."));
      cJSON_Delete(jsonResponse);
      return;
    }

    QList<QString> itemList;
    for (cJSON* jsonItem = jsonResponse->child; jsonItem; jsonItem = jsonItem->next) {

      cJSON *idItem = cJSON_GetObjectItem(jsonItem, "_id");
      if (!idItem || idItem->type != cJSON_String) {
        emit error(QString("Unable to extract item id."));
        break;
      }

      itemList.append(QString((idItem->valuestring)));
    }

    emit items(itemList);

    cJSON_Delete(jsonResponse);
  }

  this->sender()->deleteLater();
}

ListFilesRequest::ListFilesRequest(const QString &girderUrl,
    const QString &girderToken, const QString itemId,
    QObject *parent) :
    GirderRequest(girderUrl, girderToken, parent), m_itemId(itemId)
{

}

ListFilesRequest::~ListFilesRequest()
{

}

void ListFilesRequest::send()
{
  QString girderAuthUrl = QString("%1/item/%2/files")
      .arg(this->m_girderUrl).arg(this->m_itemId);

  QNetworkRequest request(girderAuthUrl);
  request.setRawHeader(QByteArray("Girder-Token"), this->m_girderToken.toUtf8());

  QObject::connect(this->m_networkManager, SIGNAL(finished(QNetworkReply *)),
      this, SLOT(finished(QNetworkReply *)));

  this->m_networkManager->get(request);
}

void ListFilesRequest::finished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    emit error(handleGirderError(reply, bytes), reply);
  }
  else {
    cJSON *jsonResponse = cJSON_Parse(bytes.constData());

    if (!jsonResponse || jsonResponse->type != cJSON_Array) {
      emit error(QString("Invalid response to listFiles."));
      cJSON_Delete(jsonResponse);
      return;
    }

    QMap<QString, QString> fileMap;
    for (cJSON* jsonFile = jsonResponse->child; jsonFile; jsonFile = jsonFile->next) {

      cJSON *idItem = cJSON_GetObjectItem(jsonFile, "_id");
      if (!idItem || idItem->type != cJSON_String) {
        emit error(QString("Unable to extract file id."));
        break;
      }
      QString id(idItem->valuestring);

      cJSON *nameItem = cJSON_GetObjectItem(jsonFile, "name");
      if (!nameItem || nameItem->type != cJSON_String) {
        emit error(QString("Unable to extract file id."));
        break;
      }
      QString name(nameItem->valuestring);

      fileMap[id] = name;
    }

    emit files(fileMap);

    cJSON_Delete(jsonResponse);
  }
}

ListFoldersRequest::ListFoldersRequest(const QString &girderUrl,
    const QString &girderToken, const QString folderId,
    QObject *parent) :
    GirderRequest(girderUrl, girderToken, parent), m_folderId(folderId)
{

}

ListFoldersRequest::~ListFoldersRequest()
{

}

void ListFoldersRequest::send()
{
  QUrl url(QString("%1/folder").arg(this->m_girderUrl));
  url.addQueryItem("parentId", this->m_folderId);
  url.addQueryItem("parentType", "folder");

  QNetworkRequest request(url);
  request.setRawHeader(QByteArray("Girder-Token"), this->m_girderToken.toUtf8());

  QObject::connect(this->m_networkManager, SIGNAL(finished(QNetworkReply *)),
      this, SLOT(finished(QNetworkReply *)));

  this->m_networkManager->get(request);
}

void ListFoldersRequest::finished(QNetworkReply *reply)
{
  QByteArray bytes = reply->readAll();
  if (reply->error()) {
    emit error(handleGirderError(reply, bytes), reply);
  }
  else {
    cJSON *jsonResponse = cJSON_Parse(bytes.constData());

    if (!jsonResponse || jsonResponse->type != cJSON_Array) {
      emit error(QString("Invalid response to listFolders."));
      cJSON_Delete(jsonResponse);
      return;
    }

    QMap<QString, QString> folderMap;
    for (cJSON* jsonFolder = jsonResponse->child; jsonFolder; jsonFolder = jsonFolder->next) {

      cJSON *idItem = cJSON_GetObjectItem(jsonFolder, "_id");
      if (!idItem || idItem->type != cJSON_String) {
        emit error(QString("Unable to extract file id."));
        break;
      }
      QString id(idItem->valuestring);

      cJSON *nameItem = cJSON_GetObjectItem(jsonFolder, "name");
      if (!nameItem || nameItem->type != cJSON_String) {
        emit error(QString("Unable to extract file id."));
        break;
      }
      QString name(nameItem->valuestring);

      folderMap[id] = name;
    }

    emit folders(folderMap);

    cJSON_Delete(jsonResponse);
  }
}


DownloadFolderRequest::DownloadFolderRequest(QNetworkCookieJar *cookieJar,
    const QString &girderUrl, const QString &girderToken,
    const QString &downloadPath, const QString &folderId, QObject *parent) :
    GirderRequest(girderUrl, girderToken, parent),
    m_folderId(folderId), m_downloadPath(downloadPath), m_itemsToDownload(NULL),
    m_foldersToDownload(NULL), m_cookieJar(cookieJar)
{
  QDir(this->m_downloadPath).mkpath(".");
}

DownloadFolderRequest::~DownloadFolderRequest()
{
  delete this->m_itemsToDownload;
  delete this->m_foldersToDownload;
}

void DownloadFolderRequest::send()
{
  ListItemsRequest *itemsRequest = new ListItemsRequest(this->m_girderUrl,
      this->m_girderToken, this->m_folderId, this);

  connect(itemsRequest, SIGNAL(items(const QList<QString>)),
      this, SLOT(items(const QList<QString>)));
  connect(itemsRequest, SIGNAL(error(const QString, QNetworkReply*)), this,
      SIGNAL(error(const QString, QNetworkReply*)));

  itemsRequest->send();

  ListFoldersRequest *foldersRequest = new ListFoldersRequest(this->m_girderUrl,
      this->m_girderToken, this->m_folderId, this);

  connect(foldersRequest, SIGNAL(folders(const QMap<QString, QString>)),
      this, SLOT(folders(const QMap<QString, QString>)));
  connect(foldersRequest, SIGNAL(error(const QString, QNetworkReply*)), this,
      SIGNAL(error(const QString, QNetworkReply*)));

  foldersRequest->send();


}

void DownloadFolderRequest::items(const QList<QString> &itemIds)
{
  ListItemsRequest *request \
    = qobject_cast<ListItemsRequest*>(this->sender());

  this->m_itemsToDownload = new QList<QString>(itemIds);

  foreach(QString itemId, itemIds) {
    DownloadItemRequest *request = new DownloadItemRequest(this->m_cookieJar,
        this->m_girderUrl, this->m_girderToken, this->m_downloadPath,
        itemId, this);

    connect(request, SIGNAL(complete()),
        this, SLOT(downloadItemFinished()));
    connect(request, SIGNAL(error(const QString, QNetworkReply*)), this,
        SIGNAL(error(const QString, QNetworkReply*)));
    connect(request, SIGNAL(info(const QString)), this,
            SIGNAL(info(const QString)));
    request->send();
  }
}

void DownloadFolderRequest::downloadItemFinished()
{
  DownloadItemRequest *request
    = qobject_cast<DownloadItemRequest*>(this->sender());

  this->m_itemsToDownload->removeOne(request->itemId());

  if (this->isComplete()) {
    emit complete();
  }

  request->deleteLater();
}

void DownloadFolderRequest::folders(const QMap<QString, QString> &folders)
{
  ListFoldersRequest *request \
    = qobject_cast<ListFoldersRequest*>(this->sender());

  this->m_foldersToDownload = new QMap<QString, QString>(folders);

  QMapIterator<QString, QString> i(folders);
  while (i.hasNext()) {
     i.next();
     QString id  = i.key();
     QString name = i.value();
     QString path = QDir(this->m_downloadPath).filePath(name);
     DownloadFolderRequest *request = new DownloadFolderRequest(this->m_cookieJar,
         this->m_girderUrl, this->m_girderToken, path, id, this);

     connect(request, SIGNAL(complete()),
         this, SLOT(downloadFolderFinished()));
     connect(request, SIGNAL(error(const QString, QNetworkReply*)), this,
         SIGNAL(error(const QString, QNetworkReply*)));
     connect(request, SIGNAL(info(const QString)), this,
             SIGNAL(info(const QString)));

     request->send();
   }
}

void DownloadFolderRequest::downloadFolderFinished()
{
  DownloadFolderRequest *request
    = qobject_cast<DownloadFolderRequest*>(this->sender());

  this->m_foldersToDownload->remove(request->folderId());

  if (this->isComplete()) {
    emit complete();
  }

  request->deleteLater();
}


bool DownloadFolderRequest::isComplete()
{
  return (this->m_itemsToDownload  && this->m_itemsToDownload->isEmpty() &&
      this->m_foldersToDownload && this->m_foldersToDownload->isEmpty());
}


DownloadItemRequest::DownloadItemRequest(QNetworkCookieJar *cookieJar,
    const QString &girderUrl, const QString &girderToken,
    const QString &path, const QString &itemId,
    QObject *parent) :
    GirderRequest(girderUrl, girderToken, parent),
    m_itemId(itemId), m_downloadPath(path), m_cookieJar(cookieJar)
{

}

DownloadItemRequest::~DownloadItemRequest()
{

}

void DownloadItemRequest::send()
{
  ListFilesRequest *request = new ListFilesRequest(this->m_girderUrl,
      this->m_girderToken, this->m_itemId, this);

  connect(request, SIGNAL(files(const QMap<QString, QString>)),
      this, SLOT(files(const QMap<QString, QString>)));
  connect(request, SIGNAL(error(const QString, QNetworkReply*)), this,
      SIGNAL(error(const QString, QNetworkReply*)));
  connect(request, SIGNAL(info(const QString)), this,
          SIGNAL(info(const QString)));

  request->send();
}

void DownloadItemRequest::files(const QMap<QString, QString> &files)
{
  ListItemsRequest *request
    = qobject_cast<ListItemsRequest*>(this->sender());

  this->m_filesToDownload = files;

  QMapIterator<QString, QString> i(files);
  while (i.hasNext()) {
    i.next();
    QString id  = i.key();
    QString name = i.value();
    DownloadFileRequest *request = new DownloadFileRequest(this->m_cookieJar,
        this->m_girderUrl, this->m_girderToken, this->m_downloadPath,
        name, id, this);

    connect(request, SIGNAL(complete()),
        this, SLOT(fileDownloadFinish()));
    connect(request, SIGNAL(error(const QString, QNetworkReply*)), this,
        SIGNAL(error(const QString, QNetworkReply*)));
    connect(request, SIGNAL(info(const QString)), this,
            SIGNAL(info(const QString)));

    request->send();
  }
}

void DownloadItemRequest::fileDownloadFinish()
{
  DownloadFileRequest *request
    = qobject_cast<DownloadFileRequest*>(this->sender());

  this->m_filesToDownload.remove(request->fileId());

  if (this->m_filesToDownload.isEmpty()) {
    emit complete();
    this->deleteLater();
  }

  request->deleteLater();
}

DownloadFileRequest::DownloadFileRequest(QNetworkCookieJar *cookieJar,
    const QString &girderUrl, const QString &girderToken, const QString &path,
    const QString &fileName, const QString &fileId, QObject *parent) :
    GirderRequest(girderUrl, girderToken, parent),
    m_fileName(fileName), m_fileId(fileId), m_downloadPath(path),
    m_cookieJar(cookieJar), m_retryCount(0)
{
  this->m_networkManager->setCookieJar(this->m_cookieJar);
  this->m_cookieJar->setParent(NULL);
}

DownloadFileRequest::~DownloadFileRequest()
{

}

void DownloadFileRequest::send()
{
  QString girderAuthUrl = QString("%1/file/%2/download")
      .arg(this->m_girderUrl).arg(this->m_fileId);

  QNetworkRequest request(girderAuthUrl);
  request.setRawHeader(QByteArray("Girder-Token"), this->m_girderToken.toUtf8());

  QObject::connect(this->m_networkManager, SIGNAL(finished(QNetworkReply *)),
      this, SLOT(finished(QNetworkReply *)));

  this->m_networkManager->get(request);
}

void DownloadFileRequest::finished(QNetworkReply *reply)
{
  if (reply->error()) {
    QByteArray bytes = reply->readAll();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).value<int>();

    if (statusCode == 400 && this->m_retryCount < 5) {
      this->send();
      this->m_retryCount++;
    }
    else {
      emit error(handleGirderError(reply, bytes), reply);
    }
  }
  else {
    // We need todo the redirect ourselves!
    QUrl redirectUrl = reply->attribute(
        QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if(!redirectUrl.isEmpty()) {
      QNetworkRequest request;
      request.setUrl(redirectUrl);
      this->m_networkManager->get(request);
      return;
    }

    emit info(QString("Downloading %1 ...").arg(this->fileName()));
    QDir downloadDir(this->m_downloadPath);
    QFile file(downloadDir.filePath(this->fileName()));
    file.open(QIODevice::WriteOnly);

    qint64 count = 0;
    char bytes[1024];

    while((count = reply->read(bytes, 1024)) != -1) {
      file.write(bytes, count);
    }

    file.close();
  }

  emit complete();
}

}
