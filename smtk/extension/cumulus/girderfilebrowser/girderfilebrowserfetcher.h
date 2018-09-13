//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME girderfilebrowserdialog.h
// .SECTION Description
// .SECTION See Also

#ifndef girderfilebrowser_girderfilebrowserfetcher_h
#define girderfilebrowser_girderfilebrowserfetcher_h

#include <QMap>
#include <QObject>
#include <QPair>
#include <QString>

#include <map>
#include <memory>

class QNetworkAccessManager;

namespace cumulus
{

class GirderRequest;

class GirderFileBrowserFetcher : public QObject
{
  Q_OBJECT

public:
  explicit GirderFileBrowserFetcher(
    QNetworkAccessManager* networkAccessManager, QObject* parent = nullptr);

  virtual ~GirderFileBrowserFetcher() override;

  void setApiUrl(const QString& url) { m_apiUrl = url; }
  void setGirderToken(const QString& token) { m_girderToken = token; }

  // Our different modes for treating items. Default is "treatItemsAsFiles".
  enum class ItemMode
  {
    treatItemsAsFiles,
    treatItemsAsFolders,
    treatItemsAsFoldersWithFileBumping
  };

  void setItemMode(ItemMode mode) { m_itemMode = mode; }
  ItemMode itemMode() const { return m_itemMode; }

  // Convenience functions
  bool treatItemsAsFiles() const;
  bool treatItemsAsFolders() const;

  // Set the root folder. Do not set this unless using a custom root folder.
  void setCustomRootInfo(const QMap<QString, QString>& rootInfo) { m_customRootInfo = rootInfo; }

signals:
  // Emitted when getFolderInformation() is complete
  void folderInformation(const QMap<QString, QString>& parentInfo,
    const QList<QMap<QString, QString> >& folders, const QList<QMap<QString, QString> >& files,
    const QList<QMap<QString, QString> >& rootPath);

  // Emitted when there is an error
  void error(const QString& message);

public slots:
  // Emits folderInformation() when it is completed.
  // This map should contain "name", "id", and "type" entries.
  void getFolderInformation(const QMap<QString, QString>& parentInfo);

  // Get the information about the home folder
  void getHomeFolderInformation();

  // Convenience function for signals
  void setApiUrlAndGirderToken(const QString& apiUrl, const QString& girderToken);

  void errorReceived(const QString& message);

private slots:
  void finishGettingContainingItems(const QMap<QString, QString>& items);
  // Only called if m_itemMode is ItemMode::treatItemsAsFoldersWithFileBumping
  void finishGettingFilesForContainingItems(
    const QMap<QString, QString>& files, const QString& itemId);

private:
  // The generic cases
  void getContainingFolders();
  void getContainingItems();
  void getContainingFiles();
  void getRootPath();

  // Checks to see if all steps are complete and there are no errors
  void finishGettingFolderInformationIfReady();
  void finishGettingFolderInformation();

  // The special cases in the top two level directories
  void getRootFolderInformation();
  void getUsersFolderInformation();
  void getCollectionsFolderInformation();

  // A function to prepend /root, /root/Users, or /root/Collections to the root path
  void prependNeededRootPathItems();

  // A general update function called by getUsersFolderInformation() and
  // getCollectionsFolderInformation()
  void finishGettingSecondLevelFolderInformation(
    const QString& type, const QMap<QString, QString>& map);

  // Remove all current requests
  void clearAllRequests();
  // Also restore the previous state. This should be done for an
  // interruption or an error.
  void clearAllRequestsAndRestorePreviousState();

  // Cached previous info in case of an error or interruption
  void clearAllCachedPreviousInfo();
  void restoreAllCachedPreviousInfo();

  // Convenience functions...
  QString currentParentName() const { return m_currentParentInfo.value("name"); }
  QString currentParentId() const { return m_currentParentInfo.value("id"); }
  QString currentParentType() const { return m_currentParentInfo.value("type"); }

  // Are any folder requests pending?
  bool folderRequestPending() const;

  // Members
  QNetworkAccessManager* m_networkManager;

  QString m_apiUrl;
  QString m_girderToken;
  ItemMode m_itemMode = ItemMode::treatItemsAsFiles;

  // These maps are < id => name >
  QMap<QString, QString> m_currentFolders;
  QMap<QString, QString> m_currentItems;
  // Each of these QMaps represents a girder object. These maps
  // should all have 3 keys: "name", "id", and "type"
  QList<QMap<QString, QString> > m_currentRootPath;

  // Only used if m_itemMode is not ItemMode::treatItemsAsFiles
  QMap<QString, QString> m_currentFiles;

  // Information about the current parent
  QMap<QString, QString> m_currentParentInfo;

  // Information about the previous parent, folder, and items are
  // cached to speed up root path functions.
  QMap<QString, QString> m_previousParentInfo;
  QMap<QString, QString> m_previousFolders;
  QMap<QString, QString> m_previousItems;

  // Cache these in case there is an error or interruption
  // The bool in the pair indicates whether a cache is available.
  // The second item in the pair is the available data.
  QPair<bool, QMap<QString, QString> > m_cachedPreviousParentInfo;
  QPair<bool, QMap<QString, QString> > m_cachedPreviousFolders;
  QPair<bool, QMap<QString, QString> > m_cachedPreviousItems;
  QPair<bool, QList<QMap<QString, QString> > > m_cachedRootPath;

  // Our requests.
  // These will be deleted automatically when a new request is made.
  // We must use a std::map here because QMap has errors with unique_ptr
  // as value.
  std::map<QString, std::unique_ptr<GirderRequest> > m_girderRequests;

  // When the item mode is "treatItemsAsFoldersWithFileBumping", we look inside
  // every item to see if it only contains one file. This variable holds all of
  // these requests and it is used to check if the requests are completed.
  std::vector<std::unique_ptr<GirderRequest> > m_itemContentsRequests;

  // Are there any updates pending?
  QMap<QString, bool> m_folderRequestPending;

  // This should only be set if we have a custom root folder
  QMap<QString, QString> m_customRootInfo;
};

inline bool GirderFileBrowserFetcher::treatItemsAsFiles() const
{
  return m_itemMode == ItemMode::treatItemsAsFiles;
}

inline bool GirderFileBrowserFetcher::treatItemsAsFolders() const
{
  return m_itemMode == ItemMode::treatItemsAsFolders ||
    m_itemMode == ItemMode::treatItemsAsFoldersWithFileBumping;
}

inline void GirderFileBrowserFetcher::setApiUrlAndGirderToken(
  const QString& url, const QString& token)
{
  setApiUrl(url);
  setGirderToken(token);
}

inline void GirderFileBrowserFetcher::finishGettingFolderInformationIfReady()
{
  if (!folderRequestPending())
    finishGettingFolderInformation();
}

} // end of namespace

#endif
