//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtFileItem.h"

#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtOverlay.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QSizePolicy>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/FileSystemItem.h"
#include "smtk/attribute/FileSystemItemDefinition.h"

#include <cassert>

//#include "pqApplicationCore.h"

// We use either STL regex or Boost regex, depending on support. These flags
// correspond to the equivalent logic used to determine the inclusion of Boost's
// regex library.
#if defined(SMTK_CLANG) ||                                                                         \
  (defined(SMTK_GCC) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)) ||                 \
  defined(SMTK_MSVC)
#include <regex>
using std::regex;
using std::regex_replace;
using std::sregex_token_iterator;
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_match;
using boost::regex_replace;
using boost::regex_search;
using boost::sregex_token_iterator;
#endif

using namespace smtk::attribute;
using namespace smtk::extension;

class qtFileItemInternals
{
public:
  qtFileItemInternals() = default;
  ~qtFileItemInternals() = default;

  bool IsDirectory{ false };
  bool m_showRecentFiles{ false };
  bool m_showExtensions{ false };
  bool m_useFileDirectory{ true };
  bool m_hasDefaultDirectory{ false };
  std::string m_defaultDirectory;
  std::string m_nameFilter;
  QFileDialog* FileBrowser{ nullptr };

  QPointer<QVBoxLayout> EntryLayout;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;
  QVector<QString> fileExtensions;

  // for extensible items
  QList<QToolButton*> MinusButtonIndices;
  QList<QPointer<QWidget>> m_editors;
  QList<QPointer<QFrame>> m_editFrames;
  QPointer<QToolButton> AddItemButton;
  QPointer<QFrame> Contents;
  QPointer<QCheckBox> OptionalCheck;
};

qtItem* qtFileItem::createItemWidget(const qtAttributeItemInfo& info)
{
  // So we support this type of item?
  if (info.itemAs<smtk::attribute::FileSystemItem>() == nullptr)
  {
    return nullptr;
  }
  return new qtFileItem(info);
}

qtFileItem::qtFileItem(const qtAttributeItemInfo& info)
  : qtItem(info)
{
  m_internals = new qtFileItemInternals;
  m_internals->IsDirectory = (m_itemInfo.item()->type() == smtk::attribute::Item::DirectoryType);
  m_isLeafItem = true;
  m_internals->VectorItemOrient = Qt::Vertical;
  auto atts = m_itemInfo.component().attributes();
  std::string val;

  // Lets examine the Item's View configuration:
  m_itemInfo.component().attributeAsBool("ShowRecentFiles", m_internals->m_showRecentFiles);
  m_itemInfo.component().attributeAsBool("ShowFileExtensions", m_internals->m_showExtensions);
  m_itemInfo.component().attributeAsBool("UseFileDirectory", m_internals->m_useFileDirectory);
  // Are we suppose to use a property on the resource as a default directory location?
  std::string propname;
  if (m_itemInfo.component().attribute("DefaultDirectoryProperty", propname))
  {
    auto attribute = m_itemInfo.item()->attribute();
    if (attribute)
    {
      auto attResource = attribute->resource();
      if (attResource)
      {
        auto vals = attResource->properties().get<std::vector<std::string>>()[propname];
        if (!vals.empty())
        {
          m_internals->m_hasDefaultDirectory = true;
          m_internals->m_defaultDirectory = vals.at(0);
        }
      }
    }
  }
  this->createWidget();
  if (m_itemInfo.uiManager())
  {
    m_itemInfo.uiManager()->onFileItemCreated(this);
  }
}

namespace
{

QString extractFileTypeName(const std::string& fileTypeDescription)
{
  // Trim the extensions to get just the file type name
  std::string name = fileTypeDescription.substr(0, fileTypeDescription.find_last_of('('));

  // Trim leading and trailing whitespace, remove multiple spaces.
  name = regex_replace(name, regex("^ +| +$|( ) +"), "$1");

  return QString::fromStdString(name);
}

QString extractFileTypeExtension(const std::string& fileTypeDescription)
{
  std::size_t begin =
    fileTypeDescription.find_first_of('*', fileTypeDescription.find_first_of('(')) + 1;
  std::size_t end = fileTypeDescription.find_first_of(" \n\r\t)", begin);
  std::string acceptableSuffix = fileTypeDescription.substr(begin, end - begin);

  return QString::fromStdString(acceptableSuffix);
}
} // namespace

qtFileItem::~qtFileItem()
{
  delete m_internals;
}

void qtFileItem::setLabelVisible(bool visible)
{
  m_internals->theLabel->setVisible(visible);
}

bool qtFileItem::isDirectory() const
{
  return m_internals->IsDirectory;
}

bool qtFileItem::showRecentFiles() const
{
  return m_internals->m_showRecentFiles;
}

bool qtFileItem::showExtensions() const
{
  return m_internals->m_showExtensions;
}

bool qtFileItem::useFileDirectory() const
{
  return m_internals->m_useFileDirectory;
}

bool qtFileItem::hasDefaultDirectory() const
{
  return m_internals->m_hasDefaultDirectory;
}

const std::string& qtFileItem::defaultDirectory() const
{
  return m_internals->m_defaultDirectory;
}

// Although you *can* disable this feature, it is not recommended.
// Behavior is not defined if this method is called after
// the ancestor qtUIManager::initializeUI() method is called.
void qtFileItem::enableFileBrowser(bool state)
{
  if (!state)
  {
    m_internals->FileBrowser->setParent(nullptr);
    delete m_internals->FileBrowser;
    m_internals->FileBrowser = nullptr;
  }
  else if (nullptr == m_internals->FileBrowser)
  {
    m_internals->FileBrowser = new QFileDialog(m_widget);
    m_internals->FileBrowser->setObjectName("selectFileDialog");
    m_internals->FileBrowser->setDirectory(QDir::currentPath());
  }
}

QWidget* qtFileItem::createFileBrowseWidget(
  int elementIdx,
  const smtk::attribute::FileSystemItem& item,
  const smtk::attribute::FileSystemItemDefinition& itemDef)
{
  QWidget* fileTextWidget = nullptr;
  QComboBox* fileCombo = nullptr;
  QLineEdit* lineEdit = nullptr;
  QComboBox* fileExtCombo = nullptr;
  QFrame* frame = new QFrame(m_internals->Contents);
  frame->setObjectName(QString("frame%1").arg(elementIdx));
  //frame->setStyleSheet("QFrame { background-color: yellow; }");
  QString defaultText;
  if (item.type() == smtk::attribute::Item::FileType)
  {
    auto fDef = dynamic_cast<const attribute::FileItemDefinition&>(itemDef);
    if (fDef.hasDefault())
      defaultText = fDef.defaultValue().c_str();
    // For open Files, if we are suppose to show recent values then we use a
    // combobox for the editor
    if (this->showRecentFiles() && fDef.shouldExist() && !item.isExtensible())
    {
      fileCombo = new QComboBox(frame);
      fileCombo->setObjectName(QString("fileCombo%1").arg(elementIdx));
      fileCombo->setEditable(true);
      fileTextWidget = fileCombo;
      lineEdit = fileCombo->lineEdit();
      //Lets add a property to the line edit  widget
      // so we can get back to the combo box its part of
      QVariant vdata;
      vdata.setValue(static_cast<void*>(fileCombo));
      lineEdit->setProperty("EditWidget", vdata);
    }

    // Note that we only need to process file extensions once
    // since all file extension combo boxes MUST have the same choices
    bool addExtensions = m_internals->fileExtensions.empty();
    // Should we be displaying file extensions?
    if (this->showExtensions() && !fDef.shouldExist())
    {
      std::string filters = fDef.getFileFilters();
      regex re(";;");
      sregex_token_iterator it(filters.begin(), filters.end(), re, -1), last;
      if (it != last && std::next(it) != last)
      {
        fileExtCombo = new QComboBox(frame);
        fileExtCombo->setObjectName(QString("fileExtCombo%1").arg(elementIdx));
        fileExtCombo->setEditable(false);
        for (; it != last; ++it)
        {
          fileExtCombo->addItem(extractFileTypeName(it->str()));
          if (addExtensions)
          {
            m_internals->fileExtensions.push_back(extractFileTypeExtension(it->str()));
          }
        }
        fileExtCombo->setMinimumContentsLength(8);
        fileExtCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
      }
    }
  }

  //If we are not using a comboox then create a line edit widget
  if (fileCombo == nullptr)
  {
    lineEdit = new QLineEdit(frame);
    lineEdit->setObjectName(QString("lineEdit%1").arg(elementIdx));
    fileTextWidget = lineEdit;
    // We need to be able to get the file ext combo from the line edit widget
    // and vice versa
    if (fileExtCombo)
    {
      QVariant vdata;
      vdata.setValue(static_cast<void*>(fileExtCombo));
      lineEdit->setProperty("ExtCombo", vdata);
      vdata.setValue(static_cast<void*>(lineEdit));
      fileExtCombo->setProperty("EditWidget", vdata);
    }
  }

  // Add the editor to the list so we can update them later
  m_internals->m_editors.push_back(fileTextWidget);

  // As a file input, if the name is too long lets favor
  // the file name over the path
  if (fileCombo)
  {
    fileCombo->lineEdit()->setAlignment(Qt::AlignRight);
    fileCombo->setMinimumContentsLength(10);

    // http://doc.qt.io/qt-5/qcombobox.html#sizeAdjustPolicy-prop
    // Recommends using QComboBox::AdjustToContents, but that does not seem to
    // work on Linux.
    fileCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
  }
  else
  {
    lineEdit->setAlignment(Qt::AlignRight);
    lineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lineEdit->setMinimumWidth(5);
  }
  frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  fileTextWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QPushButton* fileBrowserButton = new QPushButton("Browse", frame);
  fileBrowserButton->setObjectName(QString("fileBrowserButton%1").arg(elementIdx));
  fileBrowserButton->setMinimumHeight(fileTextWidget->height());
  fileBrowserButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QVariant vdata;
  vdata.setValue(static_cast<void*>(fileTextWidget));
  fileBrowserButton->setProperty("EditWidget", vdata);

  QHBoxLayout* layout = new QHBoxLayout(frame);
  layout->setObjectName(QString("layout%1").arg(elementIdx));
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(fileTextWidget);

  if (fileExtCombo)
  {
    layout->addWidget(fileExtCombo);
  }
  layout->addWidget(fileBrowserButton);
  layout->setAlignment(Qt::AlignCenter);

  std::string valText = item.valueAsString(elementIdx);
  if (fileCombo)
  {
    this->updateRecentValues(valText);
    this->updateFileComboLists();
    fileCombo->setCurrentIndex(fileCombo->findText(valText.c_str()));
  }
  else
  {
    lineEdit->setText(valText.c_str());
  }

  // Lets color the widget properly
  QFile qfile(item.value(elementIdx).c_str());
  if ((itemDef.isValueValid(item.value(elementIdx))) && (!itemDef.shouldExist() || qfile.exists()))
  {
    if (item.isUsingDefault(elementIdx))
    {
      m_itemInfo.uiManager()->setWidgetColorToDefault(lineEdit);
    }
    else
    {
      m_itemInfo.uiManager()->setWidgetColorToNormal(lineEdit);
    }
  }
  else
  {
    m_itemInfo.uiManager()->setWidgetColorToInvalid(lineEdit);
  }

  QObject::connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(onUpdateItemValue()));
  if (fileCombo)
  {
    QObject::connect(fileCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onUpdateItemValue()));
  }
  else if (fileExtCombo)
  {
    QObject::connect(
      fileExtCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onUpdateItemValue()));
  }

  QObject::connect(fileBrowserButton, SIGNAL(clicked()), this, SLOT(onLaunchFileBrowser()));

  // add custom context menu item for reset to default
  lineEdit->setContextMenuPolicy(Qt::CustomContextMenu);
  QPointer<qtFileItem> self(this);
  QObject::connect(
    lineEdit, &QLineEdit::customContextMenuRequested, this, [self, elementIdx](const QPoint& pt) {
      if (!self)
      {
        return;
      }
      self->showContextMenu(pt, elementIdx);
    });

  return frame;
}

void qtFileItem::showContextMenu(const QPoint& pt, int elementIdx)
{
  QLineEdit* const lineEdit = qobject_cast<QLineEdit*>(QObject::sender());
  auto item = this->m_itemInfo.itemAs<FileSystemItem>();
  if (!lineEdit || !item)
  {
    return;
  }
  QMenu* menu = lineEdit->createStandardContextMenu();
  auto* resetDefault = new QAction("Reset to Default");
  if (item->hasDefault())
  {
    QPointer<qtFileItem> self(this);
    QObject::connect(resetDefault, &QAction::triggered, this, [self, elementIdx, lineEdit]() {
      if (!self)
      {
        return;
      }
      auto item = self->m_itemInfo.itemAs<FileSystemItem>();
      if (item)
      {
        item->setToDefault(elementIdx);
        self->updateEditorValue(elementIdx);
      }
    });
  }
  else
  {
    resetDefault->setEnabled(false);
  }
  menu->addAction(resetDefault);
  menu->exec(lineEdit->mapToGlobal(pt));
  delete menu;
}

void qtFileItem::onUpdateItemValue()
{
  QWidget* editor = qobject_cast<QWidget*>(sender());
  if (editor == nullptr)
  {
    return;
  }

  // Find the editor in our list
  int index = m_internals->m_editors.indexOf(editor);
  if (index == -1)
  {
    // Let's see if there is an edit widget associated with this widget
    editor = static_cast<QWidget*>(editor->property("EditWidget").value<void*>());
    if (editor != nullptr)
    {
      index = m_internals->m_editors.indexOf(editor);
    }
    // If the index is still -1 then we can not find the appropriate editor
    if (index == -1)
    {
      return;
    }
  }

  this->updateItemValue(index);
}

void qtFileItem::getEditor(int i, QComboBox** comboBox, QLineEdit** lineEdit)
{
  QWidget* editor = m_internals->m_editors.at(i);
  *comboBox = nullptr;
  *lineEdit = qobject_cast<QLineEdit*>(editor);
  if (*lineEdit != nullptr)
  {
    return;
  }
  *comboBox = qobject_cast<QComboBox*>(editor);
  if (*comboBox != nullptr)
  {
    *lineEdit = (*comboBox)->lineEdit();
  }
}

void qtFileItem::updateItemValue(int elementIdx)
{
  QComboBox* comboBox;
  QLineEdit* editBox;
  this->getEditor(elementIdx, &comboBox, &editBox);
  if (editBox == nullptr)
  {
    return; // Couldn't find an expected editor
  }
  QComboBox* fileExtCombo = static_cast<QComboBox*>(editBox->property("ExtCombo").value<void*>());

  auto item = m_itemInfo.itemAs<attribute::FileSystemItem>();

  std::string value = editBox->text().toStdString();

  if (!value.empty())
  {
    auto fSystemItemDef = item->definitionAs<attribute::FileSystemItemDefinition>();
    auto fItemDef = item->definitionAs<attribute::FileItemDefinition>();

    // Do we have to deal with file extensions?
    if (fItemDef && fileExtCombo)
    {
      int filterId = fItemDef->filterId(value);
      if (filterId != -1)
      {
        // the value has a suffix that matches our definition, so we set the
        // file type combobox to reflect the update.
        fileExtCombo->setCurrentIndex(filterId);
      }
      if ((!fSystemItemDef->isValueValid(value)))
      {
        value += m_internals->fileExtensions.at(fileExtCombo->currentIndex()).toStdString();
      }
    }
    // Lets see if the value is different from the current one
    if (item->value(elementIdx) == value)
    {
      return; // value hasn't changed
    }
    item->setValue(elementIdx, value);
    Q_EMIT this->modified();

    if (comboBox && !this->isDirectory() && this->updateRecentValues(editBox->text().toStdString()))
    {
      this->updateFileComboLists();
    }

    auto* iview = m_itemInfo.baseView();
    if (iview)
    {
      iview->valueChanged(item);
    }

    QFile theFile(item->value(elementIdx).c_str());
    if (
      (fSystemItemDef->isValueValid(item->value(elementIdx))) &&
      (!fSystemItemDef->shouldExist() || theFile.exists()))
    {
      if (item->isUsingDefault(elementIdx))
      {
        m_itemInfo.uiManager()->setWidgetColorToDefault(editBox);
      }
      else
      {
        m_itemInfo.uiManager()->setWidgetColorToNormal(editBox);
      }
    }
    else
    {
      m_itemInfo.uiManager()->setWidgetColorToInvalid(editBox);
    }
  }
  else
  {
    if (!item->isSet(elementIdx))
    {
      return; // the value is already not set
    }
    item->unset(elementIdx);
    m_itemInfo.uiManager()->setWidgetColorToInvalid(editBox);
    Q_EMIT(modified());
  }
}

void qtFileItem::updateEditorValue(int elementIdx)
{
  QComboBox* comboBox;
  QLineEdit* editBox;
  this->getEditor(elementIdx, &comboBox, &editBox);
  if (editBox == nullptr)
  {
    return; // Couldn't find an expected editor
  }

  auto item = m_itemInfo.itemAs<attribute::FileSystemItem>();

  if (item->isSet(elementIdx))
  {
    std::string value = item->value(elementIdx);
    auto fSystemItemDef = item->definitionAs<attribute::FileSystemItemDefinition>();
    auto fItemDef = item->definitionAs<attribute::FileItemDefinition>();
    editBox->setText(value.c_str());

    if (comboBox && !this->isDirectory() && this->updateRecentValues(value))
    {
      this->updateFileComboLists();
    }

    QFile theFile(value.c_str());
    if (
      (fSystemItemDef->isValueValid(value)) && (!fSystemItemDef->shouldExist() || theFile.exists()))
    {
      if (item->isUsingDefault(elementIdx))
      {
        m_itemInfo.uiManager()->setWidgetColorToDefault(editBox);
      }
      else
      {
        m_itemInfo.uiManager()->setWidgetColorToNormal(editBox);
      }
    }
    else
    {
      m_itemInfo.uiManager()->setWidgetColorToInvalid(editBox);
    }
  }
  else
  {
    editBox->setText("");
    m_itemInfo.uiManager()->setWidgetColorToInvalid(editBox);
  }
}

bool qtFileItem::onLaunchFileBrowser()
{
  // This method should only be called via a signal so lets get the sender
  QWidget* button = qobject_cast<QWidget*>(sender());
  if (button == nullptr)
  {
    return false;
  }

  // Lets get the editor for this
  QWidget* editor = static_cast<QWidget*>(button->property("EditWidget").value<void*>());
  if (editor == nullptr)
  {
    return false; // couldn't find the editor associated with the button
  }
  // valueIndex refers to both the editor being used to change an item's value
  // as well as the index to the value itself w/r to the item
  int valueIndex = m_internals->m_editors.indexOf(editor);
  if (valueIndex == -1)
  {
    return false; // can't find the editor in our list
  }

  // If we are not using local file browser, just emit signal and return
  if (!m_internals->FileBrowser)
  {
    return Q_EMIT launchFileBrowser();
  }

  // If local file browser instantiated, get data from it
  auto item = m_itemInfo.itemAs<attribute::FileSystemItem>();

  QString filters;
  QString title;
  QFileDialog::FileMode mode = QFileDialog::AnyFile;
  if (m_internals->IsDirectory)
  {
    mode = QFileDialog::Directory;
    auto itemDef = item->definitionAs<attribute::FileSystemItemDefinition>();
    m_internals->FileBrowser->setOption(QFileDialog::ShowDirsOnly, itemDef->shouldExist());
    title = "Find Directory";
  }
  else
  {
    auto fItemDef = item->definitionAs<attribute::FileItemDefinition>();
    filters = fItemDef->getFileFilters().c_str();
    mode = fItemDef->shouldExist() ? QFileDialog::ExistingFile : QFileDialog::AnyFile;
    title = "Select File";
  }
  m_internals->FileBrowser->setFileMode(mode);
  m_internals->FileBrowser->setWindowTitle(title);
  if (!filters.isEmpty())
  {
    QStringList name_filters = filters.split(";;");
    m_internals->FileBrowser->setNameFilters(name_filters);
  }
  // This is needed for Macs since mode alone is not enough to allow the user to
  // chose a non-existent file
  if (mode == QFileDialog::AnyFile)
  {
    m_internals->FileBrowser->setAcceptMode(QFileDialog::AcceptSave);
    m_internals->FileBrowser->setLabelText(QFileDialog::FileName, "File name:");
  }
  m_internals->FileBrowser->setLabelText(QFileDialog::Accept, "Accept");
  // The order of preference is:
  // - Are we using the current file's directory and does it exist?
  // - If not then do we have a default directory and does it exist?
  bool dirHasBeenSet = false;
  if (this->useFileDirectory() && item->isSet(valueIndex))
  {
    QFileInfo finfo(item->valueAsString(valueIndex).c_str());
    QDir fileDir = finfo.absoluteDir();
    if (fileDir.exists())
    {
      m_internals->FileBrowser->setDirectory(fileDir);
      dirHasBeenSet = true;
    }
  }
  if (this->hasDefaultDirectory() && !dirHasBeenSet)
  {
    QDir defDir(this->defaultDirectory().c_str());
    if (defDir.exists())
    {
      m_internals->FileBrowser->setDirectory(defDir);
    }
  }
  if (m_internals->FileBrowser->exec() != QDialog::Accepted)
  {
    return false;
  }

  QStringList files = m_internals->FileBrowser->selectedFiles();
  m_internals->m_nameFilter = m_internals->FileBrowser->selectedNameFilter().toStdString();
  this->setInputValue(valueIndex, files[0]);
  return true;
}

void qtFileItem::updateFileComboLists()
{
  int i, n = m_internals->m_editors.size();
  QComboBox* cbox;
  QLineEdit* lineEdit;
  for (i = 0; i < n; i++)
  {
    this->getEditor(i, &cbox, &lineEdit);
    if (cbox == nullptr)
    {
      continue;
    }
    cbox->blockSignals(true);
    QString currentFile = cbox->currentText();
    cbox->clear();
    auto fItem = m_itemInfo.itemAs<attribute::FileItem>();
    std::vector<std::string>::const_iterator it;
    for (it = fItem->recentValues().begin(); it != fItem->recentValues().end(); ++it)
    {
      cbox->addItem((*it).c_str());
    }
    cbox->setCurrentIndex(cbox->findText(currentFile));
    cbox->blockSignals(false);
  }
}

void qtFileItem::setInputValue(int i, const QString& val)
{
  QLineEdit* lineEdit;
  QComboBox* comboBox;
  this->getEditor(i, &comboBox, &lineEdit);
  if (lineEdit == nullptr)
  {
    return; // Couldn't find an expected editor
  }

  QString value = val;

  // For files, check if the extension in val is valid. If it is not, append a
  // valid extension to it.
  if (!value.isEmpty() && !m_internals->IsDirectory)
  {
    smtk::attribute::ItemPtr item = m_itemInfo.item();

    auto fItemDef = item->definitionAs<attribute::FileItemDefinition>();
    if (!fItemDef->isValueValid(val.toStdString()))
    {
      QFileInfo fi(val);

      // grab the file-filter the user selected, if there is one.
      std::string filters =
        !m_internals->m_nameFilter.empty() ? m_internals->m_nameFilter : fItemDef->getFileFilters();
      std::size_t begin = filters.find_first_of('*', filters.find_first_of('(')) + 1;
      std::size_t end = filters.find_first_of(") \n\r\t", begin);
      QString acceptableSuffix(filters.substr(begin, end - begin).c_str());
      fi.setFile(QDir(fi.absolutePath()), fi.baseName() + acceptableSuffix);

      value = fi.filePath();
    }
  }

  // this itself will not trigger onInputValueChanged
  lineEdit->setText(value);
  this->updateItemValue(i);
}

void qtFileItem::createWidget()
{
  smtk::attribute::ItemPtr item = m_itemInfo.item();
  auto* iview = m_itemInfo.baseView();
  if (iview && !iview->displayItem(item))
  {
    return;
  }

  this->clearChildWidgets();
  this->updateUI();
}

void qtFileItem::updateItemData()
{
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  if (item->isOptional())
  {
    m_internals->OptionalCheck->setVisible(true);
    this->setOutputOptional(item->localEnabledState() ? 1 : 0);
  }
  else if (m_internals->OptionalCheck)
  {
    m_internals->OptionalCheck->setVisible(false);
    m_internals->Contents->setVisible(true);
  }

  int i, n = m_internals->m_editors.size();
  for (i = 0; i < n; i++)
  {
    this->updateEditorValue(i);
  }

  this->qtItem::updateItemData();
}

void qtFileItem::addInputEditor(
  int i,
  const smtk::attribute::FileSystemItem& item,
  const smtk::attribute::FileSystemItemDefinition& itemDef)
{
  int n = static_cast<int>(item.numberOfValues());
  if (!n)
  {
    return;
  }
  QWidget* editBox = this->createFileBrowseWidget(i, item, itemDef);
  if (!editBox)
  {
    return;
  }

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QFrame* editFrame = new QFrame(m_internals->Contents);
  editFrame->setObjectName(QString("editFrame%1").arg(i));
  QBoxLayout* editorLayout = new QHBoxLayout(editFrame);
  editorLayout->setObjectName(QString("editorLayout%1").arg(i));
  editorLayout->setMargin(0);
  editorLayout->setSpacing(3);
  if (item.isExtensible() && (i >= static_cast<int>(itemDef.numberOfRequiredValues())))
  {
    QToolButton* minusButton = new QToolButton(m_internals->Contents);
    minusButton->setObjectName(QString("minusButton%1").arg(i));
    QString iconName(":/icons/attribute/minus.png");
    minusButton->setFixedSize(QSize(12, 12));
    minusButton->setIcon(QIcon(iconName));
    minusButton->setSizePolicy(sizeFixedPolicy);
    minusButton->setToolTip("Remove value");
    editorLayout->addWidget(minusButton);
    connect(minusButton, SIGNAL(clicked()), this, SLOT(onRemoveValue()));
    QPair<QPointer<QLayout>, QPointer<QWidget>> pair;
    pair.first = editorLayout;
    pair.second = editBox;
    m_internals->MinusButtonIndices.push_back(minusButton);
  }
  // Use the labels if we have then unless there is only one
  // value and the item is not extensible
  if ((n != 1 || item.isExtensible()) && itemDef.hasValueLabels())
  {
    std::string componentLabel = itemDef.valueLabel(i);
    if (!componentLabel.empty())
    {
      QString labelText = componentLabel.c_str();
      QLabel* label = new QLabel(labelText, editBox);
      label->setObjectName(QString("label%1").arg(i));
      label->setSizePolicy(sizeFixedPolicy);
      editorLayout->addWidget(label);
    }
  }
  editorLayout->addWidget(editBox);

  // always going vertical for discrete and extensible items
  if (m_internals->VectorItemOrient == Qt::Vertical || item.isExtensible())
  {
    int row = i;
    // The "Add New Value" button is in first row, so take that into account
    row = item.isExtensible() ? row + 1 : row;
    m_internals->EntryLayout->addWidget(editFrame);
  }
  else // going horizontal
  {
    m_internals->EntryLayout->addWidget(editFrame);
  }
  // Add the frame to the list so we can track it for possible removal
  // if the item is extensible
  m_internals->m_editFrames.push_back(editFrame);
}

void qtFileItem::loadInputValues(
  const smtk::attribute::FileSystemItem& item,
  const smtk::attribute::FileSystemItemDefinition& itemDef)
{
  int n = static_cast<int>(item.numberOfValues());
  if (!n && !item.isExtensible())
  {
    return;
  }

  if (item.isExtensible())
  {
    if (!m_internals->AddItemButton)
    {
      QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      m_internals->AddItemButton = new QToolButton(m_widget);
      m_internals->AddItemButton->setObjectName("AddItemButton");
      QString iconName(":/icons/attribute/plus.png");
      m_internals->AddItemButton->setText("Add New Value");
      m_internals->AddItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

      //      m_internals->AddItemButton->setFixedSize(QSize(12, 12));
      m_internals->AddItemButton->setIcon(QIcon(iconName));
      m_internals->AddItemButton->setSizePolicy(sizeFixedPolicy);
      connect(m_internals->AddItemButton, SIGNAL(clicked()), this, SLOT(onAddNewValue()));
      m_internals->EntryLayout->addWidget(m_internals->AddItemButton);
    }
  }

  for (int i = 0; i < n; i++)
  {
    this->addInputEditor(i, item, itemDef);
  }
}

void qtFileItem::updateUI()
{
  auto* iview = m_itemInfo.baseView();
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  auto itemDef = item->definitionAs<attribute::FileSystemItemDefinition>();
  if (iview && !iview->displayItem(item))
  {
    return;
  }

  if (m_widget)
  {
    delete m_widget;
  }

  m_widget = new QFrame(m_itemInfo.parentWidget());
  m_widget->setObjectName(item->name().c_str());
  if (this->isReadOnly())
  {
    m_widget->setEnabled(false);
  }
  QBoxLayout* top;
  if ((itemDef->numberOfRequiredValues() == 1) && !itemDef->isExtensible())
  {
    top = new QHBoxLayout(m_widget);
  }
  else
  {
    top = new QVBoxLayout(m_widget);
  }
  top->setObjectName("top");

  top->setMargin(0);
  top->setSpacing(0);
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setObjectName("labelLayout");
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  // Note that the definition could be optional but the item maybe forced
  // to be required.  We need to still create the check box in case
  // the item's force required state is changed
  if (itemDef->isOptional())
  {
    m_internals->OptionalCheck = new QCheckBox(m_itemInfo.parentWidget());
    m_internals->OptionalCheck->setObjectName("OptionalCheck");
    m_internals->OptionalCheck->setChecked(item->localEnabledState());
    m_internals->OptionalCheck->setText(" ");
    m_internals->OptionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = m_internals->OptionalCheck->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(
      m_internals->OptionalCheck, SIGNAL(stateChanged(int)), this, SLOT(setOutputOptional(int)));
    labelLayout->addWidget(m_internals->OptionalCheck);
    if (!item->isOptional())
    {
      m_internals->OptionalCheck->setVisible(false);
      m_internals->Contents->setVisible(true);
    }
  }

  QString labelText = item->label().c_str();
  QLabel* label = new QLabel(labelText, m_widget);
  label->setObjectName("label");
  label->setSizePolicy(sizeFixedPolicy);
  if (iview)
  {
    label->setFixedWidth(iview->fixedLabelWidth() - padding);
  }
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // add in BriefDescription as tooltip if available
  const std::string strBriefDescription = itemDef->briefDescription();
  if (!strBriefDescription.empty())
  {
    label->setToolTip(strBriefDescription.c_str());
  }

  if (itemDef->advanceLevel() && m_itemInfo.baseView())
  {
    label->setFont(m_itemInfo.uiManager()->advancedFont());
  }
  labelLayout->addWidget(label);
  m_internals->theLabel = label;

  if (label->text().trimmed().isEmpty())
  {
    this->setLabelVisible(false);
  }

  top->addLayout(labelLayout);

  // Create a frame to hold the item's contents
  m_internals->Contents = new QFrame(m_widget);
  m_internals->Contents->setObjectName("Contents");
  m_internals->EntryLayout = new QVBoxLayout(m_internals->Contents);
  m_internals->EntryLayout->setObjectName("EntryLayout");
  m_internals->EntryLayout->setMargin(0);
  m_internals->EntryLayout->setSpacing(0);
  m_internals->EntryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  top->addWidget(m_internals->Contents);

  this->loadInputValues(*item, *itemDef);

  if (m_itemInfo.parentWidget() && m_itemInfo.parentWidget()->layout())
  {
    m_itemInfo.parentWidget()->layout()->addWidget(m_widget);
  }
  if (item->isOptional())
  {
    this->setOutputOptional(item->localEnabledState() ? 1 : 0);
  }
}

void qtFileItem::setOutputOptional(int state)
{
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  if (!(item && m_widget))
  {
    return;
  }
  bool enable = state != 0;
  m_internals->Contents->setVisible(enable);
  //  m_internals->EntryFrame->setEnabled(enable);
  if (enable != item->localEnabledState())
  {
    item->setIsEnabled(enable);
    Q_EMIT this->modified();
    auto* iview = m_itemInfo.baseView();
    if (iview)
    {
      iview->valueChanged(item);
    }
  }
}

void qtFileItem::onAddNewValue()
{
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  if (!item)
  {
    return;
  }
  auto itemDef = item->definitionAs<attribute::FileSystemItemDefinition>();
  if (item->setNumberOfValues(item->numberOfValues() + 1))
  {
    this->addInputEditor(static_cast<int>(item->numberOfValues()) - 1, *item, *itemDef);
    Q_EMIT this->modified();
  }
}

void qtFileItem::onRemoveValue()
{
  QToolButton* const minusButton = qobject_cast<QToolButton*>(QObject::sender());
  if (!minusButton)
  {
    return;
  }

  // The minus button index  related the ith extensible value to be removed.
  // The actual value being deleted is offset by the number of required values.
  int bindex = m_internals->MinusButtonIndices.indexOf(
    minusButton); //minusButton->property("SubgroupIndex").toInt();
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  int eindex = bindex + static_cast<int>(item->numberOfRequiredValues());
  if (!item || eindex < 0 || eindex >= static_cast<int>(item->numberOfValues()))
  {
    return;
  }

  // We need to delete the corresponding edit box and  edit widget
  // We also need to update all of the exiting ElementIndex information on the
  // remaining edit widgets
  QWidget* editor = m_internals->m_editFrames.at(eindex);
  delete editor; // this should clean up all widgets associated with the value removed
  m_internals->m_editors.removeAt(eindex);
  m_internals->m_editFrames.removeAt(eindex);
  m_internals->MinusButtonIndices.removeAt(bindex);

  item->removeValue(eindex);
  Q_EMIT this->modified();
}

void qtFileItem::clearChildWidgets()
{
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  if (!item)
  {
    return;
  }

  delete m_internals->Contents;
  m_internals->m_editors.clear();
  m_internals->m_editFrames.clear();
  if (item->isExtensible())
  {
    m_internals->MinusButtonIndices.clear();
  }
}

bool qtFileItem::updateRecentValues(const std::string& val)
{
  auto item = m_itemInfo.itemAs<FileItem>();
  if (item == nullptr)
  {
    return false; // Nothing to be done
  }
  std::size_t n = item->recentValues().size();
  item->addRecentValue(val);
  // Did we actually change the number of recent values?
  return (item->recentValues().size() != n);
}
