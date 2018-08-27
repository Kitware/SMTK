//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "girderrequest.h"
#include "utils.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPair>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkCookieJar>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <memory>

namespace cumulus
{

GirderRequest::GirderRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
  const QString& girderToken, QObject* parent)
  : QObject(parent)
  , m_girderUrl(girderUrl)
  , m_girderToken(girderToken)
  , m_networkManager(networkManager)
{
}

GirderRequest::~GirderRequest()
{
}

// We will use this for our unique_ptrs
struct QObjectLaterDeleter
{
  void operator()(QObject* obj) { obj->deleteLater(); }
};
template <typename T>
using unique_ptr_delete_later = std::unique_ptr<T, QObjectLaterDeleter>;

ListItemsRequest::ListItemsRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
  const QString& girderToken, const QString folderId, QObject* parent)
  : GirderRequest(networkManager, girderUrl, girderToken, parent)
  , m_folderId(folderId)
{
}

ListItemsRequest::~ListItemsRequest()
{
}

void ListItemsRequest::send()
{
  QUrlQuery urlQuery;
  urlQuery.addQueryItem("folderId", m_folderId);
  urlQuery.addQueryItem("limit", "0");

  QUrl url(QString("%1/item").arg(m_girderUrl));
  url.setQuery(urlQuery); // reconstructs the query string from the QUrlQuery

  QNetworkRequest request(url);
  request.setRawHeader(QByteArray("Girder-Token"), m_girderToken.toUtf8());

  auto reply = m_networkManager->get(request);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void ListItemsRequest::finished()
{
  unique_ptr_delete_later<QNetworkReply> reply(qobject_cast<QNetworkReply*>(this->sender()));
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply.get(), bytes), reply.get());
  }
  else
  {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes.constData());

    if (!jsonResponse.isArray())
    {
      emit error(QString("Invalid response to listItems."));
      return;
    }

    const QJsonArray& array = jsonResponse.array();
    QMap<QString, QString> itemMap;
    for (const auto& item : array)
    {
      if (!item.isObject())
      {
        emit error(QString("Invalid entry in QJsonArray"));
        break;
      }

      const QJsonObject& object = item.toObject();
      if (!object.contains("_id"))
      {
        emit error(QString("Unable to extract id."));
        break;
      }
      QString id = object.value("_id").toString();

      if (!object.contains("name"))
      {
        emit error(QString("Unable to extract name."));
        break;
      }
      QString name = object.value("name").toString();

      itemMap[id] = name;
    }

    emit items(itemMap);
  }
}

ListFilesRequest::ListFilesRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
  const QString& girderToken, const QString itemId, QObject* parent)
  : GirderRequest(networkManager, girderUrl, girderToken, parent)
  , m_itemId(itemId)
{
}

ListFilesRequest::~ListFilesRequest()
{
}

void ListFilesRequest::send()
{
  QUrl url = QUrl(QString("%1/item/%2/files").arg(m_girderUrl).arg(m_itemId));
  QUrlQuery urlQuery;
  urlQuery.addQueryItem("limit", "0");
  url.setQuery(urlQuery);

  QNetworkRequest request(url);
  request.setRawHeader(QByteArray("Girder-Token"), m_girderToken.toUtf8());

  auto reply = m_networkManager->get(request);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void ListFilesRequest::finished()
{
  unique_ptr_delete_later<QNetworkReply> reply(qobject_cast<QNetworkReply*>(this->sender()));
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply.get(), bytes), reply.get());
  }
  else
  {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes.constData());

    if (!jsonResponse.isArray())
    {
      emit error(QString("Invalid response to listFiles."));
      return;
    }

    const QJsonArray& array = jsonResponse.array();
    QMap<QString, QString> fileMap;
    for (const auto& item : array)
    {
      if (!item.isObject())
      {
        emit error(QString("Invalid entry in QJsonArray"));
        break;
      }

      const QJsonObject& object = item.toObject();
      if (!object.contains("_id"))
      {
        emit error(QString("Unable to extract id."));
        break;
      }
      QString id = object.value("_id").toString();

      if (!object.contains("name"))
      {
        emit error(QString("Unable to extract name."));
        break;
      }
      QString name = object.value("name").toString();

      fileMap[id] = name;
    }

    emit files(fileMap);
  }
}

ListFoldersRequest::ListFoldersRequest(QNetworkAccessManager* networkManager,
  const QString& girderUrl, const QString& girderToken, const QString parentId,
  const QString parentType, QObject* parent)
  : GirderRequest(networkManager, girderUrl, girderToken, parent)
  , m_parentId(parentId)
  , m_parentType(parentType)
{
}

ListFoldersRequest::~ListFoldersRequest()
{
}

void ListFoldersRequest::send()
{
  QUrlQuery urlQuery;
  urlQuery.addQueryItem("parentId", m_parentId);
  urlQuery.addQueryItem("parentType", m_parentType);
  urlQuery.addQueryItem("limit", "0");

  QUrl url(QString("%1/folder").arg(m_girderUrl));
  url.setQuery(urlQuery); // reconstructs the query string from the QUrlQuery

  QNetworkRequest request(url);
  request.setRawHeader(QByteArray("Girder-Token"), m_girderToken.toUtf8());

  auto reply = m_networkManager->get(request);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void ListFoldersRequest::finished()
{
  unique_ptr_delete_later<QNetworkReply> reply(qobject_cast<QNetworkReply*>(this->sender()));
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply.get(), bytes), reply.get());
  }
  else
  {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes.constData());

    if (!jsonResponse.isArray())
    {
      emit error(QString("Invalid response to listFolders."));
      return;
    }

    const QJsonArray& array = jsonResponse.array();
    QMap<QString, QString> folderMap;
    for (const auto& item : array)
    {
      if (!item.isObject())
      {
        emit error(QString("Invalid entry in QJsonArray"));
        break;
      }

      const QJsonObject& object = item.toObject();
      if (!object.contains("_id"))
      {
        emit error(QString("Unable to extract id."));
        break;
      }
      QString id = object.value("_id").toString();

      if (!object.contains("name"))
      {
        emit error(QString("Unable to extract name."));
        break;
      }
      QString name = object.value("name").toString();

      folderMap[id] = name;
    }

    emit folders(folderMap);
  }
}

DownloadFolderRequest::DownloadFolderRequest(QNetworkAccessManager* networkManager,
  const QString& girderUrl, const QString& girderToken, const QString& downloadPath,
  const QString& folderId, QObject* parent)
  : GirderRequest(networkManager, girderUrl, girderToken, parent)
  , m_folderId(folderId)
  , m_downloadPath(downloadPath)
  , m_itemsToDownload(NULL)
  , m_foldersToDownload(NULL)
{
  QDir(m_downloadPath).mkpath(".");
}

DownloadFolderRequest::~DownloadFolderRequest()
{
  delete m_itemsToDownload;
  delete m_foldersToDownload;
}

void DownloadFolderRequest::send()
{
  ListItemsRequest* itemsRequest =
    new ListItemsRequest(m_networkManager, m_girderUrl, m_girderToken, m_folderId, this);

  connect(
    itemsRequest, SIGNAL(items(const QList<QString>)), this, SLOT(items(const QList<QString>)));
  connect(itemsRequest, SIGNAL(error(const QString, QNetworkReply*)), this,
    SIGNAL(error(const QString, QNetworkReply*)));

  itemsRequest->send();

  ListFoldersRequest* foldersRequest = new ListFoldersRequest(
    m_networkManager, m_girderUrl, m_girderToken, m_folderId, "folder", this);

  connect(foldersRequest, SIGNAL(folders(const QMap<QString, QString>)), this,
    SLOT(folders(const QMap<QString, QString>)));
  connect(foldersRequest, SIGNAL(error(const QString, QNetworkReply*)), this,
    SIGNAL(error(const QString, QNetworkReply*)));

  foldersRequest->send();
}

void DownloadFolderRequest::items(const QList<QString>& itemIds)
{
  m_itemsToDownload = new QList<QString>(itemIds);

  foreach (QString itemId, itemIds)
  {
    DownloadItemRequest* request = new DownloadItemRequest(
      m_networkManager, m_girderUrl, m_girderToken, m_downloadPath, itemId, this);

    connect(request, SIGNAL(complete()), this, SLOT(downloadItemFinished()));
    connect(request, SIGNAL(error(const QString, QNetworkReply*)), this,
      SIGNAL(error(const QString, QNetworkReply*)));
    connect(request, SIGNAL(info(const QString)), this, SIGNAL(info(const QString)));
    request->send();
  }
}

void DownloadFolderRequest::downloadItemFinished()
{
  DownloadItemRequest* request = qobject_cast<DownloadItemRequest*>(this->sender());

  m_itemsToDownload->removeOne(request->itemId());

  if (this->isComplete())
  {
    emit complete();
  }

  request->deleteLater();
}

void DownloadFolderRequest::folders(const QMap<QString, QString>& folders)
{
  m_foldersToDownload = new QMap<QString, QString>(folders);

  QMapIterator<QString, QString> i(folders);
  while (i.hasNext())
  {
    i.next();
    QString id = i.key();
    QString name = i.value();
    QString path = QDir(m_downloadPath).filePath(name);
    DownloadFolderRequest* request =
      new DownloadFolderRequest(m_networkManager, m_girderUrl, m_girderToken, path, id, this);

    connect(request, SIGNAL(complete()), this, SLOT(downloadFolderFinished()));
    connect(request, SIGNAL(error(const QString, QNetworkReply*)), this,
      SIGNAL(error(const QString, QNetworkReply*)));
    connect(request, SIGNAL(info(const QString)), this, SIGNAL(info(const QString)));

    request->send();
  }
}

void DownloadFolderRequest::downloadFolderFinished()
{
  DownloadFolderRequest* request = qobject_cast<DownloadFolderRequest*>(this->sender());

  m_foldersToDownload->remove(request->folderId());

  if (this->isComplete())
  {
    emit complete();
  }

  request->deleteLater();
}

bool DownloadFolderRequest::isComplete()
{
  return (m_itemsToDownload && m_itemsToDownload->isEmpty() && m_foldersToDownload &&
    m_foldersToDownload->isEmpty());
}

DownloadItemRequest::DownloadItemRequest(QNetworkAccessManager* networkManager,
  const QString& girderUrl, const QString& girderToken, const QString& path, const QString& itemId,
  QObject* parent)
  : GirderRequest(networkManager, girderUrl, girderToken, parent)
  , m_itemId(itemId)
  , m_downloadPath(path)
{
}

DownloadItemRequest::~DownloadItemRequest()
{
}

void DownloadItemRequest::send()
{
  ListFilesRequest* request =
    new ListFilesRequest(m_networkManager, m_girderUrl, m_girderToken, m_itemId, this);

  connect(request, SIGNAL(files(const QMap<QString, QString>)), this,
    SLOT(files(const QMap<QString, QString>)));
  connect(request, SIGNAL(error(const QString, QNetworkReply*)), this,
    SIGNAL(error(const QString, QNetworkReply*)));
  connect(request, SIGNAL(info(const QString)), this, SIGNAL(info(const QString)));

  request->send();
}

void DownloadItemRequest::files(const QMap<QString, QString>& files)
{
  m_filesToDownload = files;

  QMapIterator<QString, QString> i(files);
  while (i.hasNext())
  {
    i.next();
    QString id = i.key();
    QString name = i.value();
    DownloadFileRequest* request = new DownloadFileRequest(
      m_networkManager, m_girderUrl, m_girderToken, m_downloadPath, name, id, this);

    connect(request, SIGNAL(complete()), this, SLOT(fileDownloadFinish()));
    connect(request, SIGNAL(error(const QString, QNetworkReply*)), this,
      SIGNAL(error(const QString, QNetworkReply*)));
    connect(request, SIGNAL(info(const QString)), this, SIGNAL(info(const QString)));

    request->send();
  }
}

void DownloadItemRequest::fileDownloadFinish()
{
  DownloadFileRequest* request = qobject_cast<DownloadFileRequest*>(this->sender());

  m_filesToDownload.remove(request->fileId());

  if (m_filesToDownload.isEmpty())
  {
    emit complete();
    this->deleteLater();
  }

  request->deleteLater();
}

DownloadFileRequest::DownloadFileRequest(QNetworkAccessManager* networkManager,
  const QString& girderUrl, const QString& girderToken, const QString& path,
  const QString& fileName, const QString& fileId, QObject* parent)
  : GirderRequest(networkManager, girderUrl, girderToken, parent)
  , m_fileName(fileName)
  , m_fileId(fileId)
  , m_downloadPath(path)

  , m_retryCount(0)
{
}

DownloadFileRequest::~DownloadFileRequest()
{
}

void DownloadFileRequest::send()
{
  QString girderAuthUrl = QString("%1/file/%2/download").arg(m_girderUrl).arg(m_fileId);

  QNetworkRequest request(girderAuthUrl);
  request.setRawHeader(QByteArray("Girder-Token"), m_girderToken.toUtf8());

  auto reply = m_networkManager->get(request);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void DownloadFileRequest::finished()
{
  auto reply = qobject_cast<QNetworkReply*>(this->sender());
  if (reply->error())
  {
    QByteArray bytes = reply->readAll();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).value<int>();

    if (statusCode == 400 && m_retryCount < 5)
    {
      this->send();
      m_retryCount++;
    }
    else
    {
      emit error(handleGirderError(reply, bytes), reply);
    }
  }
  else
  {
    // We need todo the redirect ourselves!
    QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!redirectUrl.isEmpty())
    {
      QNetworkRequest request;
      request.setUrl(redirectUrl);
      auto reply = m_networkManager->get(request);
      QObject::connect(reply, SIGNAL(finished()), this, SLOT(finished()));
      return;
    }

    emit info(QString("Downloading %1 ...").arg(this->fileName()));
    QDir downloadDir(m_downloadPath);
    QFile file(downloadDir.filePath(this->fileName()));
    file.open(QIODevice::WriteOnly);

    qint64 count = 0;
    char bytes[1024];

    while ((count = reply->read(bytes, sizeof(bytes))) > 0)
    {
      file.write(bytes, count);
    }

    file.close();
  }
  reply->deleteLater();
  emit complete();
}

GetFolderParentRequest::GetFolderParentRequest(QNetworkAccessManager* networkManager,
  const QString& girderUrl, const QString& girderToken, const QString& folderId, QObject* parent)
  : GirderRequest(networkManager, girderUrl, girderToken, parent)
  , m_folderId(folderId)
{
}

GetFolderParentRequest::~GetFolderParentRequest()
{
}

void GetFolderParentRequest::send()
{
  QUrl url(QString("%1/folder/%2").arg(m_girderUrl).arg(m_folderId));

  QNetworkRequest request(url);
  request.setRawHeader(QByteArray("Girder-Token"), m_girderToken.toUtf8());

  auto reply = m_networkManager->get(request);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void GetFolderParentRequest::finished()
{
  unique_ptr_delete_later<QNetworkReply> reply(qobject_cast<QNetworkReply*>(this->sender()));
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply.get(), bytes), reply.get());
  }
  else
  {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes.constData());

    // There should only be one response
    if (!jsonResponse.isObject())
    {
      emit error(QString("Invalid response to GetFolderParentRequest."));
      return;
    }

    const QJsonObject& jsonObject = jsonResponse.object();

    QMap<QString, QString> parentInfo;
    if (!jsonObject.contains("parentCollection"))
    {
      emit error("Unable to extract parent collection.");
      return;
    }
    parentInfo["type"] = jsonObject.value("parentCollection").toString();

    if (!jsonObject.contains("parentId"))
    {
      emit error("Unable to extract parent id.");
      return;
    }
    parentInfo["id"] = jsonObject.value("parentId").toString();

    emit parent(parentInfo);
  }
}

GetRootPathRequest::GetRootPathRequest(QNetworkAccessManager* networkManager,
  const QString& girderUrl, const QString& girderToken, const QString& parentId,
  const QString& parentType, QObject* parent)
  : GirderRequest(networkManager, girderUrl, girderToken, parent)
  , m_parentId(parentId)
  , m_parentType(parentType)
{
}

GetRootPathRequest::~GetRootPathRequest()
{
}

void GetRootPathRequest::send()
{
  QUrl url(QString("%1/%2/%3/rootpath").arg(m_girderUrl).arg(m_parentType).arg(m_parentId));

  QNetworkRequest request(url);
  request.setRawHeader(QByteArray("Girder-Token"), m_girderToken.toUtf8());

  auto reply = m_networkManager->get(request);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void GetRootPathRequest::finished()
{
  unique_ptr_delete_later<QNetworkReply> reply(qobject_cast<QNetworkReply*>(this->sender()));
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply.get(), bytes), reply.get());
  }
  else
  {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes.constData());

    if (!jsonResponse.isArray())
    {
      emit error(QString("Invalid response to GetRootPathRequest."));
      return;
    }

    const QJsonArray& array = jsonResponse.array();
    QList<QMap<QString, QString> > rootPathList;
    for (const auto& folder : array)
    {
      if (!folder.isObject())
      {
        emit error("Error: root path folder is not a json object.");
        break;
      }
      const QJsonObject& _tmpObject = folder.toObject();

      // For some reason, everything is under an "object" key...
      if (!_tmpObject.contains("object") && _tmpObject.value("object").isObject())
      {
        emit error("Object key is missing.");
        break;
      }

      const QJsonObject& object = _tmpObject.value("object").toObject();

      QMap<QString, QString> entryMap;
      if (!object.contains("_modelType"))
      {
        emit error("Unable to extract model type.");
        break;
      }
      entryMap["type"] = object.value("_modelType").toString();

      if (!object.contains("_id"))
      {
        emit error("Unable to extract id.");
        break;
      }
      entryMap["id"] = object.value("_id").toString();

      QString nameField;
      if (entryMap["type"] == "user")
        nameField = "login";
      else
        nameField = "name";

      if (!object.contains(nameField))
      {
        emit error("Unable to extract name.");
        break;
      }
      entryMap["name"] = object.value(nameField).toString();

      rootPathList.append(entryMap);
    }
    emit rootPath(rootPathList);
  }
}

GetUsersRequest::GetUsersRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
  const QString& girderToken, QObject* parent)
  : GirderRequest(networkManager, girderUrl, girderToken, parent)
{
}

GetUsersRequest::~GetUsersRequest() = default;

void GetUsersRequest::send()
{
  QUrlQuery urlQuery;
  urlQuery.addQueryItem("limit", "0");

  QUrl url(QString("%1/user").arg(m_girderUrl));
  url.setQuery(urlQuery); // reconstructs the query string from the QUrlQuery

  QNetworkRequest request(url);
  request.setRawHeader(QByteArray("Girder-Token"), m_girderToken.toUtf8());

  auto reply = m_networkManager->get(request);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void GetUsersRequest::finished()
{
  unique_ptr_delete_later<QNetworkReply> reply(qobject_cast<QNetworkReply*>(this->sender()));
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply.get(), bytes), reply.get());
  }
  else
  {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes.constData());

    if (!jsonResponse.isArray())
    {
      emit error("Invalid response to GetUsersRequest.");
      return;
    }

    const QJsonArray& array = jsonResponse.array();
    QMap<QString, QString> usersMap;
    for (const auto& item : array)
    {
      if (!item.isObject())
      {
        emit error(QString("Invalid entry in QJsonArray"));
        break;
      }

      const QJsonObject& object = item.toObject();
      if (!object.contains("_id"))
      {
        emit error(QString("Unable to extract id."));
        break;
      }
      QString id = object.value("_id").toString();

      if (!object.contains("login"))
      {
        emit error(QString("Unable to extract user login."));
        break;
      }
      QString login = object.value("login").toString();

      usersMap[id] = login;
    }

    emit users(usersMap);
  }
}

GetCollectionsRequest::GetCollectionsRequest(QNetworkAccessManager* networkManager,
  const QString& girderUrl, const QString& girderToken, QObject* parent)
  : GirderRequest(networkManager, girderUrl, girderToken, parent)
{
}

GetCollectionsRequest::~GetCollectionsRequest() = default;

void GetCollectionsRequest::send()
{
  QUrlQuery urlQuery;
  urlQuery.addQueryItem("limit", "0");

  QUrl url(QString("%1/collection").arg(m_girderUrl));
  url.setQuery(urlQuery); // reconstructs the query string from the QUrlQuery

  QNetworkRequest request(url);
  request.setRawHeader(QByteArray("Girder-Token"), m_girderToken.toUtf8());

  auto reply = m_networkManager->get(request);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void GetCollectionsRequest::finished()
{
  unique_ptr_delete_later<QNetworkReply> reply(qobject_cast<QNetworkReply*>(this->sender()));
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply.get(), bytes), reply.get());
  }
  else
  {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes.constData());

    if (!jsonResponse.isArray())
    {
      emit error("Invalid response to GetCollectionsRequest.");
      return;
    }

    const QJsonArray& array = jsonResponse.array();
    QMap<QString, QString> collectionsMap;
    for (const auto& item : array)
    {
      if (!item.isObject())
      {
        emit error(QString("Invalid entry in QJsonArray"));
        break;
      }

      const QJsonObject& object = item.toObject();
      if (!object.contains("_id"))
      {
        emit error(QString("Unable to extract id."));
        break;
      }
      QString id = object.value("_id").toString();

      if (!object.contains("name"))
      {
        emit error(QString("Unable to extract name."));
        break;
      }
      QString name = object.value("name").toString();

      collectionsMap[id] = name;
    }
    emit collections(collectionsMap);
  }
}

GetMyUserRequest::GetMyUserRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
  const QString& girderToken, QObject* parent)
  : GirderRequest(networkManager, girderUrl, girderToken, parent)
{
}

GetMyUserRequest::~GetMyUserRequest() = default;

void GetMyUserRequest::send()
{
  QUrl url(QString("%1/user/me").arg(m_girderUrl));

  QNetworkRequest request(url);
  request.setRawHeader(QByteArray("Girder-Token"), m_girderToken.toUtf8());

  auto reply = m_networkManager->get(request);
  QObject::connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void GetMyUserRequest::finished()
{
  unique_ptr_delete_later<QNetworkReply> reply(qobject_cast<QNetworkReply*>(this->sender()));
  QByteArray bytes = reply->readAll();
  if (reply->error())
  {
    emit error(handleGirderError(reply.get(), bytes), reply.get());
  }
  else
  {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes.constData());

    // There should only be one response
    if (!jsonResponse.isObject())
    {
      emit error(QString("Invalid response to GetMyUserRequest."));
      return;
    }

    const QJsonObject& jsonObject = jsonResponse.object();

    QMap<QString, QString> myInfo;
    if (!jsonObject.contains("login"))
    {
      emit error("Unable to extract login.");
      return;
    }
    myInfo["login"] = jsonObject.value("login").toString();

    if (!jsonObject.contains("_id"))
    {
      emit error("Unable to extract id.");
      return;
    }
    myInfo["id"] = jsonObject.value("_id").toString();

    emit myUser(myInfo);
  }
}

} // end namespace
