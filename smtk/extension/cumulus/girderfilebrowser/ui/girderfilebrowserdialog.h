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

#ifndef girderfilebrowser_girderfilebrowserdialog_h
#define girderfilebrowser_girderfilebrowserdialog_h

// Are we building a stand alone girder file browser?
#ifdef GIRDERFILEBROWSER_BUILD_STANDALONE
// Empty definition to prevent compilation error
#define SMTKCUMULUSEXT_EXPORT
#else
#include "smtk/extension/cumulus/Exports.h"
#endif

#include <QDialog>
#include <QMap>
#include <QString>

#include <memory>

class QIcon;
class QModelIndex;
class QNetworkAccessManager;
class QResizeEvent;
class QStandardItemModel;

namespace Ui
{
class GirderFileBrowserDialog;
}

namespace cumulus
{

class GirderFileBrowserFetcher;

class SMTKCUMULUSEXT_EXPORT GirderFileBrowserDialog : public QDialog
{
  Q_OBJECT

public:
  // If customRootFolder is set, that will be the root folder instead of the
  // standard one.
  // customRootFolder should have the following keys: "name", "id", and
  // "type". "type" will typically be "folder" or "user". The keys should
  // all be valid. If an invalid customRootFolder is provided, the behavior
  // is undefined.
  explicit GirderFileBrowserDialog(QNetworkAccessManager* networkAccessManager,
    const QMap<QString, QString>& customRootFolder = QMap<QString, QString>(),
    QWidget* parent = nullptr);

  virtual ~GirderFileBrowserDialog() override;

  void setApiUrl(const QString& url);
  void setGirderToken(const QString& token);

  // Set the choosable types. Must be valid types. Some of the valid
  // types are: 'user', 'collection', 'folder', 'item', 'file'
  void setChoosableTypes(const QStringList& choosableTypes);

signals:
  // If a row is chosen, this will be emitted
  // objectInfo should contain "name", "id", and "type".
  void objectChosen(const QMap<QString, QString>& objectInfo);

  // The following signals are used internally only:
  void changeFolder(const QMap<QString, QString>& parentInfo);
  void goHome();

public slots:
  // Call this when the api url and girder token are set, and browsing
  // is ready to start.
  void begin();

  // A convenience function for authentication success
  void setApiUrlAndGirderToken(const QString& url, const QString& token);

  // All visible rows must match this expression
  void changeVisibleRows(const QString& expression);

  // Current item modes are "Treat Items as Files", "Treat Items as Folders",
  // and "Treat Items as Folders with File Bumping".
  void setItemMode(const QString& text);

protected:
  void resizeEvent(QResizeEvent* event) override;

private slots:
  void rowActivated(const QModelIndex&);
  void goUpDirectory();
  void chooseObject();

  void finishChangingFolder(const QMap<QString, QString>& newParentInfo,
    const QList<QMap<QString, QString> >& folders, const QList<QMap<QString, QString> >& files,
    const QList<QMap<QString, QString> >& rootPath);

  void errorReceived(const QString& message);

private:
  void updateRootPathWidget();
  void updateVisibleRows();

  // Convenience functions...
  QString currentParentName() const { return m_currentParentInfo.value("name"); }
  QString currentParentId() const { return m_currentParentInfo.value("id"); }
  QString currentParentType() const { return m_currentParentInfo.value("type"); }

  // Members
  QNetworkAccessManager* m_networkManager;
  std::unique_ptr<Ui::GirderFileBrowserDialog> m_ui;
  std::unique_ptr<QStandardItemModel> m_itemModel;
  std::unique_ptr<GirderFileBrowserFetcher> m_girderFileBrowserFetcher;

  // Have we started yet?
  bool m_hasStarted = false;

  QMap<QString, QString> m_currentParentInfo;

  // What is the root info?
  QMap<QString, QString> m_rootFolder;

  // Only show these types
  QStringList m_choosableTypes;

  // How much should we offset the top buttons by?
  int m_rootPathOffset = 0;

  // Only show rows whose names match with this expression
  QString m_rowsMatchExpression;

  // Cache info for each row so we can use it when the user double clicks
  // on the row.
  QList<QMap<QString, QString> > m_cachedRowInfo;
  QList<QMap<QString, QString> > m_currentRootPathInfo;

  // Our icons that we use
  std::unique_ptr<QIcon> m_folderIcon;
  std::unique_ptr<QIcon> m_fileIcon;
};

inline void GirderFileBrowserDialog::begin()
{
  m_hasStarted = true;
  emit changeFolder(m_rootFolder);
}

} // end of namespace

#endif
