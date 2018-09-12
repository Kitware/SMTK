//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME girderrequest.h
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_extension_cumulus_girderrequest_h
#define __smtk_extension_cumulus_girderrequest_h

// Are we building a stand alone girder file browser?
#ifdef GIRDERFILEBROWSER_BUILD_STANDALONE
// Empty definition to prevent compilation error
#define SMTKCUMULUSEXT_EXPORT
#else
#include "smtk/extension/cumulus/Exports.h"
#endif

#include <QList>
#include <QMap>
#include <QNetworkReply>
#include <QObject>
#include <QPair>

class QNetworkAccessManager;
class QNetworkCookieJar;
class QNetworkReply;

namespace cumulus
{

class SMTKCUMULUSEXT_EXPORT GirderRequest : public QObject
{
  Q_OBJECT

public:
  GirderRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, QObject* parent = 0);
  ~GirderRequest();

  void virtual send() = 0;

signals:
  void complete();
  void error(const QString& msg, QNetworkReply* networkReply = NULL);
  void info(const QString& msg);

protected:
  QString m_girderUrl;
  QString m_girderToken;
  QNetworkAccessManager* m_networkManager;
};

class ListItemsRequest : public GirderRequest
{
  Q_OBJECT

public:
  ListItemsRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, const QString folderId, QObject* parent = 0);
  ~ListItemsRequest();

  void send();

signals:
  void items(const QMap<QString, QString>& itemMap);

private slots:
  void finished();
  QString folderId() const { return m_folderId; };

private:
  QString m_folderId;
};

class ListFoldersRequest : public GirderRequest
{
  Q_OBJECT

public:
  ListFoldersRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, const QString parentId, const QString parentType = "folder",
    QObject* parent = 0);
  ~ListFoldersRequest();

  void send();

signals:
  void folders(const QMap<QString, QString>& folders);

private slots:
  void finished();

private:
  QString m_parentId;
  QString m_parentType;
};

class ListFilesRequest : public GirderRequest
{
  Q_OBJECT

public:
  ListFilesRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, const QString itemId, QObject* parent = 0);
  ~ListFilesRequest();

  void send();
  QString itemId() const { return m_itemId; };
  QString path() const { return m_path; };

signals:
  void files(const QMap<QString, QString>& files);

private slots:
  void finished();

private:
  QString m_itemId;
  QString m_path;
};

class DownloadFolderRequest : public GirderRequest
{
  Q_OBJECT

public:
  DownloadFolderRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, const QString& downloadPath, const QString& folderId,
    QObject* parent = 0);
  ~DownloadFolderRequest();

  void send();
  QString folderId() const { return m_folderId; };
  QString downloadPath() const { return m_downloadPath; };

private slots:
  void items(const QList<QString>& itemIds);
  void folders(const QMap<QString, QString>& folders);
  void downloadItemFinished();
  void downloadFolderFinished();

private:
  QString m_folderId;
  QString m_downloadPath;
  QList<QString>* m_itemsToDownload;
  QMap<QString, QString>* m_foldersToDownload;

  bool isComplete();
};

class DownloadFileRequest : public GirderRequest
{
  Q_OBJECT

public:
  DownloadFileRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, const QString& path, const QString& fileName, const QString& fileId,
    QObject* parent = 0);
  ~DownloadFileRequest();

  void send();
  QString fileName() const { return m_fileName; };
  QString fileId() const { return m_fileId; };
  QString downloadPath() const { return m_downloadPath; };

private slots:
  void finished();

private:
  QString m_fileName;
  QString m_fileId;
  QString m_downloadPath;
  int m_retryCount;
};

class DownloadItemRequest : public GirderRequest
{
  Q_OBJECT

public:
  DownloadItemRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, const QString& path, const QString& itemId, QObject* parent = 0);
  ~DownloadItemRequest();

  void send();
  QString itemId() const { return m_itemId; };
  QString downloadPath() const { return m_downloadPath; };

private slots:
  void files(const QMap<QString, QString>& fileIds);
  void fileDownloadFinish();

private:
  QString m_itemId;
  QString m_downloadPath;
  // <fileId => fileName>
  QMap<QString, QString> m_filesToDownload;
};

class GetFolderParentRequest : public GirderRequest
{
  Q_OBJECT

public:
  GetFolderParentRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, const QString& folderId, QObject* parent = 0);
  ~GetFolderParentRequest();

  void send();

signals:
  // This map should contain "id", "type", and "name"
  void parent(const QMap<QString, QString>& parentInfo);

private slots:
  void finished();

private:
  QString m_folderId;
};

class GetRootPathRequest : public GirderRequest
{
  Q_OBJECT

public:
  GetRootPathRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, const QString& parentId, const QString& parentType = "folder",
    QObject* parent = 0);
  ~GetRootPathRequest();

  void send();

signals:
  // A hierarchy of folders to the root folder. The first item in the
  // QList should be the user. The rest are folders.
  // The keys "type", "id", and "name" should be present for each entry.
  void rootPath(const QList<QMap<QString, QString> >& rootPathList);

private slots:
  void finished();

private:
  QString m_parentId;
  QString m_parentType;
};

class GetUsersRequest : public GirderRequest
{
  Q_OBJECT

public:
  GetUsersRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, QObject* parent = 0);
  ~GetUsersRequest();

  void send();

signals:

  // <userId => loginName>
  void users(const QMap<QString, QString>& usersMap);

private slots:
  void finished();
};

class GetCollectionsRequest : public GirderRequest
{
  Q_OBJECT

public:
  GetCollectionsRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, QObject* parent = 0);
  ~GetCollectionsRequest();

  void send();

signals:

  // <collectionId => collectionName>
  void collections(const QMap<QString, QString>& collectionsMap);

private slots:
  void finished();
};

class GetMyUserRequest : public GirderRequest
{
  Q_OBJECT

public:
  GetMyUserRequest(QNetworkAccessManager* networkManager, const QString& girderUrl,
    const QString& girderToken, QObject* parent = 0);
  ~GetMyUserRequest();

  void send();

signals:
  // Should contain some information about my user. Includes these keys:
  // - id
  // - login
  void myUser(const QMap<QString, QString>& userInfo);

private slots:
  void finished();
};
}

#endif
