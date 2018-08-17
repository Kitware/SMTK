//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "girderfilebrowserdialog.h"
#include "ui_girderfilebrowserdialog.h"

#include "girderfilebrowserfetcher.h"

#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardItemModel>

namespace cumulus
{

static const QStringList& ALL_OBJECT_TYPES = { "root", "Users", "Collections", "user", "collection",
  "folder", "item", "file" };

static bool isRootInfoValid(QMap<QString, QString>& rootInfo)
{
  bool infoIsValid = true;

  if (!rootInfo.contains("name"))
  {
    qDebug() << "Error in " << __FUNCTION__ << ": root folder must contain 'name'!";
    qDebug() << "Changing to default root folder...";
    infoIsValid = false;
  }
  if (!rootInfo.contains("id"))
  {
    qDebug() << "Error in " << __FUNCTION__ << ": root folder must contain 'id'!";
    qDebug() << "Changing to default root folder...";
    infoIsValid = false;
  }
  if (!rootInfo.contains("type"))
  {
    qDebug() << "Error in " << __FUNCTION__ << ": root folder must contain 'type'!";
    qDebug() << "Changing to default root folder...";
    infoIsValid = false;
  }

  return infoIsValid;
}

GirderFileBrowserDialog::GirderFileBrowserDialog(QNetworkAccessManager* networkManager,
  const QMap<QString, QString>& customRootFolder, QWidget* parent)
  : QDialog(parent)
  , m_networkManager(networkManager)
  , m_ui(new Ui::GirderFileBrowserDialog)
  , m_itemModel(new QStandardItemModel(this))
  , m_girderFileBrowserFetcher(new GirderFileBrowserFetcher(m_networkManager))
  , m_rootFolder(customRootFolder)
  , m_choosableTypes(ALL_OBJECT_TYPES)
  , m_folderIcon(new QIcon(":/icons/folder.png"))
  , m_fileIcon(new QIcon(":/icons/file.png"))
{
  m_ui->setupUi(this);

  m_ui->list_fileBrowser->setModel(m_itemModel.get());

  // Hide the item selection combobox by default
  m_ui->combo_itemMode->setVisible(false);

  // Increase the font size of the entries in the list by just a little
  // It is 11 by default.
  QFont font = m_ui->list_fileBrowser->font();
  font.setPointSize(12);
  m_ui->list_fileBrowser->setFont(font);

  m_ui->layout_rootPath->setAlignment(Qt::AlignLeft);

  // What to do with a row upon double-click or 'enter' key
  connect(m_ui->list_fileBrowser, &QAbstractItemView::activated, this,
    &GirderFileBrowserDialog::rowActivated);

  // The 'up' button
  connect(m_ui->push_goUpDir, &QPushButton::pressed, this, &GirderFileBrowserDialog::goUpDirectory);
  // The 'Home' button
  connect(m_ui->push_goHome, &QPushButton::pressed, this, &GirderFileBrowserDialog::goHome);
  // Temporary combo box. This will be removed in the final version of this program.
  connect(m_ui->combo_itemMode,
    static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this,
    &GirderFileBrowserDialog::setItemMode);

  // The 'choose' button
  connect(
    m_ui->push_chooseObject, &QPushButton::pressed, this, &GirderFileBrowserDialog::chooseObject);

  // Only enable the choose button if the type it is on is choosable
  connect(m_ui->list_fileBrowser->selectionModel(), &QItemSelectionModel::currentChanged, this,
    [this](const QModelIndex& current, const QModelIndex& previous) {
      m_ui->push_chooseObject->setEnabled(false);
      if (current.isValid())
      {
        int row = current.row();
        if (row < m_cachedRowInfo.size() && m_choosableTypes.contains(m_cachedRowInfo[row]["type"]))
        {
          m_ui->push_chooseObject->setEnabled(true);
        }
      }
    });

  // Change folder
  connect(
    this, &GirderFileBrowserDialog::changeFolder, [this]() { this->setCursor(Qt::WaitCursor); });
  connect(this, &GirderFileBrowserDialog::changeFolder, m_girderFileBrowserFetcher.get(),
    &GirderFileBrowserFetcher::getFolderInformation);
  // Finish changing folder
  connect(m_girderFileBrowserFetcher.get(), &GirderFileBrowserFetcher::folderInformation, this,
    &GirderFileBrowserDialog::finishChangingFolder);
  // An error occurred while changing folders
  connect(m_girderFileBrowserFetcher.get(), &GirderFileBrowserFetcher::error, this,
    &GirderFileBrowserDialog::errorReceived);

  bool usingCustomRootFolder = false;
  if (!m_rootFolder.isEmpty() && isRootInfoValid(m_rootFolder))
    usingCustomRootFolder = true;

  // What to do when 'goHome' is called.
  connect(this, &GirderFileBrowserDialog::goHome, [this]() { this->setCursor(Qt::WaitCursor); });
  if (!usingCustomRootFolder)
  {
    // If we are not using a custom root folder, "go home" will go to the user's
    // home directory.
    connect(this, &GirderFileBrowserDialog::goHome, m_girderFileBrowserFetcher.get(),
      &GirderFileBrowserFetcher::getHomeFolderInformation);
    // Reset the filter box when we go home
    connect(this, &GirderFileBrowserDialog::goHome, this, [this]() {
      m_ui->edit_matchesExpression->setText("");
      m_rowsMatchExpression = "";
    });
  }
  else
  {
    // If we are using a custom root folder, "go home" should just go to the
    // root folder (in case the home folder is outside the root path).
    connect(
      this, &GirderFileBrowserDialog::goHome, this, [this]() { emit changeFolder(m_rootFolder); });
  }

  // When the user types in the filter box, update the visible rows
  connect(m_ui->edit_matchesExpression, &QLineEdit::textEdited, this,
    &GirderFileBrowserDialog::changeVisibleRows);

  // Reset the filter text when we change folders
  connect(this, &GirderFileBrowserDialog::changeFolder, m_ui->edit_matchesExpression, [this]() {
    m_ui->edit_matchesExpression->setText("");
    m_rowsMatchExpression = "";
  });

  if (!usingCustomRootFolder)
  {
    // Start in root unless directed otherwise
    m_rootFolder["name"] = "root";
    m_rootFolder["id"] = "";
    m_rootFolder["type"] = "root";
  }
  else
  {
    // We will only set this if we are using a custom root folder
    m_girderFileBrowserFetcher->setCustomRootInfo(m_rootFolder);
  }
}

GirderFileBrowserDialog::~GirderFileBrowserDialog() = default;

// A convenience function for estimating button width
static int buttonWidth(QPushButton* button)
{
  return std::max(button->fontMetrics().width(button->text()), button->sizeHint().width());
}

void GirderFileBrowserDialog::updateRootPathWidget()
{
  QHBoxLayout* layout = m_ui->layout_rootPath;
  QWidget* parentWidget = layout->parentWidget();

  // Cache the old layout width and make sure we don't exceed it.
  int oldLayoutWidth = layout->geometry().width();

  // Clear the current items from the QLayout
  while (QLayoutItem* item = layout->takeAt(0))
  {
    layout->removeWidget(item->widget());
    item->widget()->deleteLater();
  }

  // The scroll left button
  QPushButton* scrollLeft = new QPushButton("<", parentWidget);
  scrollLeft->setAutoDefault(false);
  int scrollLeftWidth = scrollLeft->fontMetrics().width(scrollLeft->text()) * 2;
  scrollLeft->setFixedWidth(scrollLeftWidth);
  layout->addWidget(scrollLeft);
  connect(scrollLeft, &QPushButton::pressed, this, [this]() {
    ++m_rootPathOffset;
    updateRootPathWidget();
  });

  // The scroll right button
  QPushButton* scrollRight = new QPushButton(">", parentWidget);
  scrollRight->setAutoDefault(false);
  int scrollRightWidth = scrollRight->fontMetrics().width(scrollRight->text()) * 2;
  scrollRight->setFixedWidth(scrollRightWidth);
  scrollRight->setEnabled(m_rootPathOffset != 0);
  layout->addWidget(scrollRight);
  connect(scrollRight, &QPushButton::pressed, this, [this]() {
    --m_rootPathOffset;
    updateRootPathWidget();
  });

  // Sum up the total button width and make sure we don't exceed it
  int totalWidgetWidth = scrollLeftWidth;
  totalWidgetWidth += scrollRightWidth;

  // Add the widgets in backwards
  int currentOffset = 0;
  bool rootButtonAdded = false;
  if (m_rootPathOffset == 0)
  {
    // This button doesn't do anything... it is only here for consistency
    QPushButton* firstButton = new QPushButton(currentParentName() + "/", parentWidget);
    firstButton->setAutoDefault(false);
    layout->insertWidget(1, firstButton);

    totalWidgetWidth += buttonWidth(firstButton);

    if (m_currentParentInfo == m_rootFolder)
      rootButtonAdded = true;
  }
  else
  {
    currentOffset += 1;
  }

  for (auto it = m_currentRootPathInfo.rbegin(); it != m_currentRootPathInfo.rend(); ++it)
  {
    if (currentOffset < m_rootPathOffset)
    {
      ++currentOffset;
      continue;
    }

    const auto& rootPathItem = *it;

    auto callFunc = [this, rootPathItem]() { emit changeFolder(rootPathItem); };
    QString name = rootPathItem.value("name");

    QPushButton* button = new QPushButton(name + "/", parentWidget);
    button->setAutoDefault(false);
    connect(button, &QPushButton::pressed, this, callFunc);

    int newButtonWidth = buttonWidth(button);

    // We want to make sure at least one button is added
    if (newButtonWidth + totalWidgetWidth > oldLayoutWidth * 0.92 && layout->count() > 2)
    {
      delete button;
      break;
    }

    if (rootPathItem == m_rootFolder)
      rootButtonAdded = true;

    layout->insertWidget(1, button);
    totalWidgetWidth += newButtonWidth;
  }

  // Only enable the scroll left if there is room to offset in that direction
  scrollLeft->setEnabled(!rootButtonAdded);
}

void GirderFileBrowserDialog::resizeEvent(QResizeEvent* event)
{
  updateRootPathWidget();
  QWidget::resizeEvent(event);
}

void GirderFileBrowserDialog::rowActivated(const QModelIndex& index)
{
  int row = index.row();
  if (row < m_cachedRowInfo.size())
  {
    QString parentType = m_cachedRowInfo[row].value("type", "unknown");

    QStringList folderTypes{ "root", "Users", "Collections", "user", "collection", "folder" };

    // If we are to treat items as folders, add items to this list
    using ItemMode = GirderFileBrowserFetcher::ItemMode;
    if (m_girderFileBrowserFetcher->treatItemsAsFolders())
      folderTypes.append("item");

    if (folderTypes.contains(parentType))
      emit changeFolder(m_cachedRowInfo[row]);
  }
}

void GirderFileBrowserDialog::goUpDirectory()
{
  QString parentName, parentId, parentType;
  if (m_currentParentInfo == m_rootFolder)
  {
    // Do nothing
    return;
  }

  QMap<QString, QString> newParentInfo;
  newParentInfo["name"] = m_currentRootPathInfo.back().value("name");
  newParentInfo["id"] = m_currentRootPathInfo.back().value("id");
  newParentInfo["type"] = m_currentRootPathInfo.back().value("type");

  emit changeFolder(newParentInfo);
}

void GirderFileBrowserDialog::setItemMode(const QString& itemModeStr)
{
  QString modifiedItemModeStr = QString(itemModeStr).replace(" ", "");

  using ItemMode = GirderFileBrowserFetcher::ItemMode;
  ItemMode itemMode;
  if (modifiedItemModeStr.compare("TreatItemsAsFiles", Qt::CaseInsensitive) == 0)
  {
    itemMode = ItemMode::treatItemsAsFiles;
  }
  else if (modifiedItemModeStr.compare("TreatItemsAsFolders", Qt::CaseInsensitive) == 0)
  {
    itemMode = ItemMode::treatItemsAsFolders;
  }
  else if (modifiedItemModeStr.compare("TreatItemsAsFoldersWithFileBumping", Qt::CaseInsensitive) ==
    0)
  {
    itemMode = ItemMode::treatItemsAsFoldersWithFileBumping;
  }
  else
  {
    qDebug() << "Warning: ignoring unknown item mode:" << itemModeStr;
    return;
  }

  m_girderFileBrowserFetcher->setItemMode(itemMode);

  // Update the current folder since this may change how we interpret the contents
  if (m_hasStarted)
    emit changeFolder(m_currentParentInfo);
}

void GirderFileBrowserDialog::chooseObject()
{
  QModelIndexList list = m_ui->list_fileBrowser->selectionModel()->selectedIndexes();
  if (list.isEmpty())
    return;

  // We can only choose one object right now
  int row = list[0].row();

  QMap<QString, QString> selectedRowInfo = m_cachedRowInfo[row];

  // If this type is not choosable, just ignore it
  if (!m_choosableTypes.contains(selectedRowInfo["type"]))
    return;

  emit objectChosen(selectedRowInfo);
}

void GirderFileBrowserDialog::changeVisibleRows(const QString& expression)
{
  m_rowsMatchExpression = expression;
  updateVisibleRows();
}

void GirderFileBrowserDialog::updateVisibleRows()
{
  // First, make all rows visible
  for (size_t i = 0; i < m_cachedRowInfo.size(); ++i)
    m_ui->list_fileBrowser->setRowHidden(i, false);

  // First, hide any rows that do not match the type the user is choosing
  // We will always show the following types, even if they aren't choosable.
  QStringList showTypes = { "Users", "Collections", "user", "collection", "folder" };
  // Add the choosable types
  showTypes += m_choosableTypes;
  for (size_t i = 0; i < m_cachedRowInfo.size(); ++i)
  {
    if (!showTypes.contains(m_cachedRowInfo[i]["type"]))
    {
      m_ui->list_fileBrowser->setRowHidden(i, true);
    }
  }

  // Next, if there is a matching expression, hide all rows whose name does not match the expression
  if (m_rowsMatchExpression.isEmpty())
    return;

  QRegularExpression regExp(
    ".*" + m_rowsMatchExpression + ".*", QRegularExpression::CaseInsensitiveOption);

  for (size_t i = 0; i < m_cachedRowInfo.size(); ++i)
  {
    // If the row is already hidden, skip it
    if (m_ui->list_fileBrowser->isRowHidden(i))
      continue;

    if (!regExp.match(m_cachedRowInfo[i]["name"]).hasMatch())
    {
      m_ui->list_fileBrowser->setRowHidden(i, true);
    }
  }
}

void GirderFileBrowserDialog::finishChangingFolder(const QMap<QString, QString>& newParentInfo,
  const QList<QMap<QString, QString> >& folders, const QList<QMap<QString, QString> >& files,
  const QList<QMap<QString, QString> >& rootPath)
{
  // Reset the root path offset when we change folders
  m_rootPathOffset = 0;

  m_currentParentInfo = newParentInfo;
  m_currentRootPathInfo = rootPath;

  size_t numRows = folders.size() + files.size();
  m_itemModel->setRowCount(numRows);
  m_itemModel->setColumnCount(1);

  m_cachedRowInfo.clear();

  int currentRow = 0;

  // Folders
  for (int i = 0; i < folders.size(); ++i)
  {
    QString name = folders[i].value("name");

    m_itemModel->setItem(currentRow, 0, new QStandardItem(*m_folderIcon, name));
    m_cachedRowInfo.append(folders[i]);
    ++currentRow;
  }

  // Files
  for (int i = 0; i < files.size(); ++i)
  {
    QString name = files[i].value("name");

    m_itemModel->setItem(currentRow, 0, new QStandardItem(*m_fileIcon, name));
    m_cachedRowInfo.append(files[i]);
    ++currentRow;
  }

  updateVisibleRows();
  updateRootPathWidget();

  // Disable object choosing
  m_ui->push_chooseObject->setEnabled(false);
  setCursor(Qt::ArrowCursor);
}

void GirderFileBrowserDialog::errorReceived(const QString& message)
{
  setCursor(Qt::ArrowCursor);
  qDebug() << "An error occurred:\n" << message;
  QMessageBox::critical(this, "An Error Occurred:", message);
}

void GirderFileBrowserDialog::setApiUrl(const QString& url)
{
  m_girderFileBrowserFetcher->setApiUrl(url);
}

void GirderFileBrowserDialog::setGirderToken(const QString& token)
{
  m_girderFileBrowserFetcher->setGirderToken(token);
}

void GirderFileBrowserDialog::setApiUrlAndGirderToken(const QString& url, const QString& token)
{
  setApiUrl(url);
  setGirderToken(token);
}

void GirderFileBrowserDialog::setChoosableTypes(const QStringList& choosableTypes)
{
  // Double check and make sure they are valid types
  for (const auto& type : choosableTypes)
  {
    if (!ALL_OBJECT_TYPES.contains(type))
    {
      qDebug() << "Error in" << __FUNCTION__ << ": invalid type was set:" << type;
      qDebug() << "The list of valid types are as follows:" << ALL_OBJECT_TYPES;
      qDebug() << "Choosable types will not be changed.";
      return;
    }
  }

  m_choosableTypes = choosableTypes;
}

} // end namespace
