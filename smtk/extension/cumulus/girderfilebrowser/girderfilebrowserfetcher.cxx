//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "girderfilebrowserfetcher.h"

#include "girderrequest.h"

#include <QNetworkAccessManager>

namespace cumulus
{

// Set the various names we will use for the requests
static const QString& GET_FOLDERS_REQUEST = "getFoldersRequest";
static const QString& GET_ITEMS_REQUEST = "getItemsRequest";
static const QString& GET_FILES_REQUEST = "getFilesRequest";
static const QString& GET_ROOT_PATH_REQUEST = "getRootPathRequest";
static const QString& GET_USERS_REQUEST = "getUsersRequest";
static const QString& GET_COLLECTIONS_REQUEST = "getCollectionsRequest";
static const QString& GET_MY_USER_REQUEST = "getMyUserRequest";

// The folder info for the special cases
static const QMap<QString, QString> ROOT_FOLDER_INFO = { { "name", "root" }, { "id", "" },
  { "type", "root" } };

static const QMap<QString, QString> USERS_FOLDER_INFO = { { "name", "Users" }, { "id", "" },
  { "type", "Users" } };

static const QMap<QString, QString> COLLECTIONS_FOLDER_INFO = { { "name", "Collections" },
  { "id", "" }, { "type", "Collections" } };

GirderFileBrowserFetcher::GirderFileBrowserFetcher(
  QNetworkAccessManager* networkManager, QObject* parent)
  : QObject(parent)
  , m_networkManager(networkManager)
{
  // These indicate if a folder request is currently pending
  m_folderRequestPending["folders"] = false;
  m_folderRequestPending["items"] = false;
  m_folderRequestPending["files"] = false;
  m_folderRequestPending["rootPath"] = false;

  // Any time a request is completed, delete the previous cache
  connect(
    this, &GirderFileBrowserFetcher::folderInformation, [this]() { clearAllCachedPreviousInfo(); });

  // This is done to set all the cache bools to false
  clearAllCachedPreviousInfo();
}

GirderFileBrowserFetcher::~GirderFileBrowserFetcher() = default;

void GirderFileBrowserFetcher::getFolderInformation(const QMap<QString, QString>& parentInfo)
{
  // Clear all requests to cancel any existing requests, and restore the
  // previous state if this is an interruption.
  clearAllRequestsAndRestorePreviousState();

  // Cache some info in case there is an interruption or error
  m_cachedPreviousParentInfo.first = true;
  m_cachedPreviousParentInfo.second = m_previousParentInfo;

  // Save this info to process the current request
  m_previousParentInfo = m_currentParentInfo;
  m_currentParentInfo = parentInfo;

  // The first two directory levels will be different from the rest.
  if (currentParentType() == "root")
  {
    getRootFolderInformation();
    return;
  }
  else if (currentParentType() == "Users")
  {
    getUsersFolderInformation();
    return;
  }
  else if (currentParentType() == "Collections")
  {
    getCollectionsFolderInformation();
    return;
  }

  // For the standard case
  getContainingFolders();
  getContainingItems();
  getContainingFiles();
  getRootPath();
}

void GirderFileBrowserFetcher::clearAllRequests()
{
  m_girderRequests.clear();
  m_itemContentsRequests.clear();

  // Indicate that no requests are pending
  for (const auto& key : m_folderRequestPending.keys())
    m_folderRequestPending[key] = false;
}

// Clear all requests and restore any previous cached info if an error
// or an interruption occurred.
void GirderFileBrowserFetcher::clearAllRequestsAndRestorePreviousState()
{
  clearAllRequests();
  restoreAllCachedPreviousInfo();
}

void GirderFileBrowserFetcher::clearAllCachedPreviousInfo()
{
  // This is a lazy clear...
  m_cachedPreviousParentInfo.first = false;
  m_cachedPreviousFolders.first = false;
  m_cachedPreviousItems.first = false;
  m_cachedRootPath.first = false;
}

// Restore all cached info if there was an error or interruption
void GirderFileBrowserFetcher::restoreAllCachedPreviousInfo()
{
  // The first element of each QPair indicates that cached information is
  // available.
  if (m_cachedPreviousParentInfo.first)
  {
    m_currentParentInfo = m_previousParentInfo;
    m_previousParentInfo = m_cachedPreviousParentInfo.second;
  }

  if (m_cachedPreviousFolders.first)
  {
    m_currentFolders = m_previousFolders;
    m_previousFolders = m_cachedPreviousFolders.second;
  }

  if (m_cachedPreviousItems.first)
  {
    m_currentItems = m_previousItems;
    m_previousItems = m_cachedPreviousItems.second;
  }

  if (m_cachedRootPath.first)
    m_currentRootPath = m_cachedRootPath.second;

  clearAllCachedPreviousInfo();
}

// A convenience function to do three things:
//   1. Call "send()" on the GirderRequest sender.
//   2. Create connections for finish condition.
//   3. Create connections for error condition.
// Sender must be a GirderRequest object, and Receiver must be
// a GirderFileBrowserFetcher object. Slot can be either a
// GirderFileBrowserFetcher function or a lambda.
template <typename Sender, typename Signal, typename Receiver, typename Slot>
static void sendAndConnect(Sender* sender, Signal signal, Receiver* receiver, Slot slot)
{
  sender->send();

  QObject::connect(sender, signal, receiver, slot);
  QObject::connect(
    sender, &GirderRequest::error, receiver, &GirderFileBrowserFetcher::errorReceived);
}

void GirderFileBrowserFetcher::getHomeFolderInformation()
{
  // Clear all requests to cancel any existing requests, and restore the
  // previous state if this is an interruption.
  clearAllRequestsAndRestorePreviousState();

  std::unique_ptr<GetMyUserRequest> getMyUserRequest(
    new GetMyUserRequest(m_networkManager, m_apiUrl, m_girderToken));

  sendAndConnect(getMyUserRequest.get(), &GetMyUserRequest::myUser, this,
    [this](const QMap<QString, QString>& myUserInfo) {
      QMap<QString, QString> myUserMap;
      myUserMap["name"] = myUserInfo.value("login");
      myUserMap["id"] = myUserInfo.value("id");
      myUserMap["type"] = "user";
      getFolderInformation(myUserMap);
    });

  m_girderRequests[GET_MY_USER_REQUEST] = std::move(getMyUserRequest);
}

bool GirderFileBrowserFetcher::folderRequestPending() const
{
  const QList<bool>& list = m_folderRequestPending.values();
  return std::any_of(list.cbegin(), list.cend(), [](bool b) { return b; });
}

void GirderFileBrowserFetcher::getRootFolderInformation()
{
  QList<QMap<QString, QString> > folders;
  folders.append(COLLECTIONS_FOLDER_INFO);
  folders.append(USERS_FOLDER_INFO);

  // We have no files for the first folder level
  QList<QMap<QString, QString> > files;

  // Root info is empty as well
  QList<QMap<QString, QString> > rootPath;

  emit folderInformation(m_currentParentInfo, folders, files, rootPath);
}

void GirderFileBrowserFetcher::getUsersFolderInformation()
{
  std::unique_ptr<GetUsersRequest> getUsersRequest(
    new GetUsersRequest(m_networkManager, m_apiUrl, m_girderToken));

  sendAndConnect(getUsersRequest.get(), &GetUsersRequest::users, this,
    [this](const QMap<QString, QString>& usersMap) {
      finishGettingSecondLevelFolderInformation("user", usersMap);
    });

  m_girderRequests[GET_USERS_REQUEST] = std::move(getUsersRequest);
}

void GirderFileBrowserFetcher::getCollectionsFolderInformation()
{
  std::unique_ptr<GetCollectionsRequest> getCollectionsRequest(
    new GetCollectionsRequest(m_networkManager, m_apiUrl, m_girderToken));

  sendAndConnect(getCollectionsRequest.get(), &GetCollectionsRequest::collections, this,
    [this](const QMap<QString, QString>& collectionsMap) {
      finishGettingSecondLevelFolderInformation("collection", collectionsMap);
    });

  m_girderRequests[GET_COLLECTIONS_REQUEST] = std::move(getCollectionsRequest);
}

// Type is probably either "user" or "collection"
void GirderFileBrowserFetcher::finishGettingSecondLevelFolderInformation(
  const QString& type, const QMap<QString, QString>& map)
{
  int numItems = map.keys().size();

  // Let's sort these by name. QMap sorts by key.
  QMap<QString, QString> sortedByName;
  for (const auto& id : map.keys())
    sortedByName[map.value(id)] = id;

  QList<QMap<QString, QString> > folders;
  for (const auto& name : sortedByName.keys())
  {
    QMap<QString, QString> folderInfo;
    folderInfo["type"] = type;
    folderInfo["id"] = sortedByName[name];
    folderInfo["name"] = name;
    folders.append(folderInfo);
  }

  // We have no files for the second directory level
  QList<QMap<QString, QString> > files;

  QList<QMap<QString, QString> > rootPath{ ROOT_FOLDER_INFO };

  emit folderInformation(m_currentParentInfo, folders, files, rootPath);
}

void GirderFileBrowserFetcher::finishGettingFolderInformation()
{
  QList<QMap<QString, QString> > folders;
  for (const auto& id : m_currentFolders.keys())
  {
    QString name = m_currentFolders.value(id);

    QMap<QString, QString> folderInfo;
    folderInfo["type"] = "folder";
    folderInfo["id"] = id;
    folderInfo["name"] = name;
    folders.append(folderInfo);
  }

  QList<QMap<QString, QString> > files;
  // Do we treat items as files?
  if (treatItemsAsFiles())
  {
    for (const auto& id : m_currentItems.keys())
    {
      QString name = m_currentItems.value(id);

      QMap<QString, QString> fileInfo;
      fileInfo["type"] = "item";
      fileInfo["id"] = id;
      fileInfo["name"] = name;
      files.append(fileInfo);
    }
  }
  // Or do we treat items as folders?
  else if (treatItemsAsFolders())
  {
    for (const auto& id : m_currentItems.keys())
    {
      QString name = m_currentItems.value(id);

      QMap<QString, QString> folderInfo;
      folderInfo["type"] = "item";
      folderInfo["id"] = id;
      folderInfo["name"] = name;
      folders.append(folderInfo);
    }

    for (const auto& id : m_currentFiles.keys())
    {
      QString name = m_currentFiles.value(id);

      QMap<QString, QString> fileInfo;
      fileInfo["type"] = "file";
      fileInfo["id"] = id;
      fileInfo["name"] = name;
      files.append(fileInfo);
    }
  }

  // Sort the folders and files by name
  auto sortFunc = [](const QMap<QString, QString>& a, const QMap<QString, QString>& b) {
    return a["name"] < b["name"];
  };

  std::sort(folders.begin(), folders.end(), sortFunc);
  std::sort(files.begin(), files.end(), sortFunc);

  emit folderInformation(m_currentParentInfo, folders, files, m_currentRootPath);
}

void GirderFileBrowserFetcher::getContainingFolders()
{
  // Cache some info in case there is an interruption or error
  m_cachedPreviousFolders.first = true;
  m_cachedPreviousFolders.second = m_previousFolders;

  // Save this info to process the current request
  m_previousFolders = m_currentFolders;
  m_currentFolders.clear();

  // Parent type must be user, collection, or folder, or there are no folders
  QStringList folderParentTypes{ "collection", "user", "folder" };
  if (!folderParentTypes.contains(currentParentType()))
    return;

  std::unique_ptr<ListFoldersRequest> getFoldersRequest(new ListFoldersRequest(
    m_networkManager, m_apiUrl, m_girderToken, currentParentId(), currentParentType()));

  sendAndConnect(getFoldersRequest.get(), &ListFoldersRequest::folders, this,
    [this](const QMap<QString, QString>& folders) {
      m_currentFolders = folders;
      m_folderRequestPending["folders"] = false;
      finishGettingFolderInformationIfReady();
    });

  m_girderRequests[GET_FOLDERS_REQUEST] = std::move(getFoldersRequest);
  m_folderRequestPending["folders"] = true;
}

void GirderFileBrowserFetcher::getContainingItems()
{
  // Cache some info in case there is an interruption or error
  m_cachedPreviousItems.first = true;
  m_cachedPreviousItems.second = m_previousItems;

  // Save this info for the current request
  m_previousItems = m_currentItems;
  m_currentItems.clear();

  // Parent type must be folder, or there are no items
  if (currentParentType() != "folder")
    return;

  std::unique_ptr<ListItemsRequest> getItemsRequest(
    new ListItemsRequest(m_networkManager, m_apiUrl, m_girderToken, currentParentId()));

  sendAndConnect(getItemsRequest.get(), &ListItemsRequest::items, this,
    &GirderFileBrowserFetcher::finishGettingContainingItems);

  m_girderRequests[GET_ITEMS_REQUEST] = std::move(getItemsRequest);
  m_folderRequestPending["items"] = true;
}

void GirderFileBrowserFetcher::finishGettingContainingItems(const QMap<QString, QString>& items)
{
  m_currentItems = items;

  // If we are to treat items as files or folders without file bumping, we are done
  if (treatItemsAsFiles() || m_itemMode == ItemMode::treatItemsAsFolders || items.empty())
  {
    m_folderRequestPending["items"] = false;
    finishGettingFolderInformationIfReady();
  }
  else if (m_itemMode == ItemMode::treatItemsAsFoldersWithFileBumping)
  {
    // Check the contents of every item. If it only contains a file,
    // treat that item as a file.
    // This results in a lot of api calls. Maybe it should be optimized somehow
    // in the future.
    for (const QString& itemId : m_currentItems.keys())
    {
      std::unique_ptr<ListFilesRequest> listFilesRequest(
        new ListFilesRequest(m_networkManager, m_apiUrl, m_girderToken, itemId));

      sendAndConnect(listFilesRequest.get(), &ListFilesRequest::files, this,
        [this, itemId](const QMap<QString, QString>& files) {
          finishGettingFilesForContainingItems(files, itemId);
        });

      m_itemContentsRequests.push_back(std::move(listFilesRequest));
    }
  }
}

// Only called if m_itemMode is ItemMode::treatItemsAsFoldersWithFileBumping
void GirderFileBrowserFetcher::finishGettingFilesForContainingItems(
  const QMap<QString, QString>& files, const QString& itemId)
{
  QObject* sender = QObject::sender();

  // Remove this object from the item contents requests
  auto it = std::find_if(m_itemContentsRequests.begin(), m_itemContentsRequests.end(),
    [sender](const std::unique_ptr<GirderRequest>& request) { return sender == request.get(); });

  if (it != m_itemContentsRequests.end())
    m_itemContentsRequests.erase(it);

  // If there is only one file that has the same name, remove the item and use the file instead.
  if (files.keys().size() == 1 && files.values()[0] == m_currentItems[itemId])
  {
    m_currentItems.remove(itemId);
    m_currentFiles[files.keys()[0]] = files.values()[0];
  }

  // If we have finished the last item, indicate that we are done.
  if (m_itemContentsRequests.empty())
  {
    m_folderRequestPending["items"] = false;
    finishGettingFolderInformationIfReady();
  }
}

void GirderFileBrowserFetcher::getContainingFiles()
{
  m_currentFiles.clear();

  // Parent type must be item, or there are no files
  if (currentParentType() != "item")
    return;

  std::unique_ptr<ListFilesRequest> listFilesRequest(
    new ListFilesRequest(m_networkManager, m_apiUrl, m_girderToken, currentParentId()));

  sendAndConnect(listFilesRequest.get(), &ListFilesRequest::files, this,
    [this](const QMap<QString, QString>& files) {
      m_currentFiles = files;
      m_folderRequestPending["files"] = false;
      finishGettingFolderInformationIfReady();
    });

  m_girderRequests[GET_FILES_REQUEST] = std::move(listFilesRequest);
  m_folderRequestPending["files"] = true;
}

void GirderFileBrowserFetcher::prependNeededRootPathItems()
{
  // This will add /root and /Users or /Collections if needed
  QList<QMap<QString, QString> > prependedRootPathItems;

  if (currentParentName() != "root")
  {
    prependedRootPathItems.append(ROOT_FOLDER_INFO);
  }

  bool needUsers = false;
  bool needCollections = false;

  // For the generic case
  if (!m_currentRootPath.isEmpty())
  {
    const auto& topPathItem = m_currentRootPath.front();
    QString topItemType = topPathItem.value("type");
    if (topItemType == "user")
      needUsers = true;
    else if (topItemType == "collection")
      needCollections = true;
  }

  if (currentParentType() == "user")
    needUsers = true;
  else if (currentParentType() == "collection")
    needCollections = true;

  if (needUsers)
    prependedRootPathItems.append(USERS_FOLDER_INFO);
  else if (needCollections)
    prependedRootPathItems.append(COLLECTIONS_FOLDER_INFO);

  m_currentRootPath = prependedRootPathItems + m_currentRootPath;
}

static void popFrontUntilEqual(
  QList<QMap<QString, QString> >& list, const QMap<QString, QString>& map)
{
  while (!list.isEmpty() && list.front() != map)
    list.pop_front();
}

void GirderFileBrowserFetcher::getRootPath()
{
  // Cache some info in case there is an interruption or error
  m_cachedRootPath.first = true;
  m_cachedRootPath.second = m_currentRootPath;

  // Skip the root path check if the previous parent was the same as the current one
  if (m_currentParentInfo == m_previousParentInfo)
    return;

  // Skip the root path check if the current parent is the actual root
  if (!m_customRootInfo.isEmpty() && m_currentParentInfo == m_customRootInfo)
  {
    m_currentRootPath.clear();
    return;
  }

  // To potentially skip an api call, check if the  parent is already
  // in the root path. If it is, re-assign the path.
  if (m_currentRootPath.contains(m_currentParentInfo))
  {
    while (m_currentRootPath.back() != m_currentParentInfo)
      m_currentRootPath.pop_back();
    m_currentRootPath.pop_back();
    return;
  }

  // Parent type must be folder or item, or this cannot be called.
  if (currentParentType() != "folder" && currentParentType() != "item")
  {
    m_currentRootPath.clear();
    prependNeededRootPathItems();
    return;
  }

  // To also potentially skip an api call, check if the current parent was in
  // the previous set of folders or items. If it was, then we just moved down
  // one directory. Skip the root path call and set it manually.
  if (currentParentType() == "folder" && m_previousFolders.keys().contains(currentParentId()))
  {
    m_currentRootPath.append(m_previousParentInfo);
    return;
  }

  if (currentParentType() == "item" && m_previousItems.keys().contains(currentParentId()))
  {
    m_currentRootPath.append(m_previousParentInfo);
    return;
  }

  m_currentRootPath.clear();

  std::unique_ptr<GetRootPathRequest> getRootPathRequest(new GetRootPathRequest(
    m_networkManager, m_apiUrl, m_girderToken, currentParentId(), currentParentType()));

  sendAndConnect(getRootPathRequest.get(), &GetRootPathRequest::rootPath, this,
    [this](const QList<QMap<QString, QString> >& rootPath) {
      m_currentRootPath = rootPath;
      prependNeededRootPathItems();
      // If there is a custom root, remove all items till we hit that one
      if (!m_customRootInfo.isEmpty())
        popFrontUntilEqual(m_currentRootPath, m_customRootInfo);
      m_folderRequestPending["rootPath"] = false;
      finishGettingFolderInformationIfReady();
    });

  m_girderRequests[GET_ROOT_PATH_REQUEST] = std::move(getRootPathRequest);
  m_folderRequestPending["rootPath"] = true;
}

void GirderFileBrowserFetcher::errorReceived(const QString& message)
{
  // First, clear the requests so no new error is produced from the
  // current set of updates, and restore the previous state.
  clearAllRequestsAndRestorePreviousState();

  QObject* sender = QObject::sender();
  QString completeMessage;
  if (sender == m_girderRequests[GET_FOLDERS_REQUEST].get())
  {
    completeMessage += "An error occurred while getting folders:\n";
  }
  else if (sender == m_girderRequests[GET_ITEMS_REQUEST].get())
  {
    completeMessage += "An error occurred while getting items:\n";
  }
  else if (sender == m_girderRequests[GET_FILES_REQUEST].get())
  {
    completeMessage += "An error occurred while getting files:\n";
  }
  else if (sender == m_girderRequests[GET_ROOT_PATH_REQUEST].get())
  {
    completeMessage += "An error occurred while updating the root path:\n";
  }
  else if (sender == m_girderRequests[GET_USERS_REQUEST].get())
  {
    completeMessage += "An error occurred while getting users:\n";
  }
  else if (sender == m_girderRequests[GET_COLLECTIONS_REQUEST].get())
  {
    completeMessage += "An error occurred while getting collections:\n";
  }
  else if (sender == m_girderRequests[GET_MY_USER_REQUEST].get())
  {
    completeMessage += "Failed to get information about current user:\n";
  }
  else if (!m_itemContentsRequests.empty() &&
    std::any_of(m_itemContentsRequests.cbegin(), m_itemContentsRequests.cend(),
      [sender](const std::unique_ptr<GirderRequest>& uPtr) { return uPtr.get() == sender; }))
  {
    m_itemContentsRequests.clear();
    completeMessage += "Failed to get one of the item's contents:\n";
  }
  completeMessage += message;
  emit error(message);
}

} // end namespace
