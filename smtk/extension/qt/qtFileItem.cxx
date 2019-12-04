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

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSignalMapper>
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
using std::sregex_token_iterator;
using std::regex_replace;
using std::regex_search;
using std::regex_match;
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::sregex_token_iterator;
using boost::regex_replace;
using boost::regex_search;
using boost::regex_match;
#endif

using namespace smtk::attribute;
using namespace smtk::extension;

class qtFileItemInternals
{
public:
  qtFileItemInternals()
    : FileBrowser(NULL)
  {
  }
  ~qtFileItemInternals() {}

  bool IsDirectory;
  QFileDialog* FileBrowser;
  QPointer<QComboBox> fileCombo;

  QPointer<QComboBox> fileExtCombo;
  QVector<QString> fileExtensions;

  QPointer<QGridLayout> EntryLayout;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;

  // for discrete items that with potential child widget
  // <Enum-Combo, child-layout >
  QMap<QWidget*, QPointer<QLayout> > ChildrenMap;

  // for extensible items
  QMap<QToolButton*, QPair<QPointer<QLayout>, QPointer<QWidget> > > ExtensibleMap;
  QPointer<QSignalMapper> SignalMapper;
  QList<QToolButton*> MinusButtonIndices;
  QPointer<QToolButton> AddItemButton;
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
  this->Internals = new qtFileItemInternals;
  this->Internals->IsDirectory =
    (m_itemInfo.item()->type() == smtk::attribute::Item::DirectoryType);
  this->Internals->SignalMapper = new QSignalMapper();
  m_isLeafItem = true;
  this->Internals->VectorItemOrient = Qt::Horizontal;
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
  std::string name = fileTypeDescription.substr(0, fileTypeDescription.find_last_of("("));

  // Trim leading and trailing whitespace, remove multiple spaces.
  name = regex_replace(name, regex("^ +| +$|( ) +"), "$1");

  return QString::fromStdString(name);
}

QString extractFileTypeExtension(const std::string& fileTypeDescription)
{
  std::size_t begin =
    fileTypeDescription.find_first_of("*", fileTypeDescription.find_first_of("(")) + 1;
  std::size_t end = fileTypeDescription.find_first_of(" \n\r\t)", begin);
  std::string acceptableSuffix = fileTypeDescription.substr(begin, end - begin);

  return QString::fromStdString(acceptableSuffix);
}
}

qtFileItem::~qtFileItem()
{
  delete this->Internals;
}

void qtFileItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
}

bool qtFileItem::isDirectory()
{
  return this->Internals->IsDirectory;
}

// Although you *can* disable this feature, it is not recommended.
// Behavior is not defined if this method is called after
// the ancestor qtUIManager::initializeUI() method is called.
void qtFileItem::enableFileBrowser(bool state)
{
  if (!state)
  {
    this->Internals->FileBrowser->setParent(NULL);
    delete this->Internals->FileBrowser;
    this->Internals->FileBrowser = NULL;
  }
  else if (NULL == this->Internals->FileBrowser)
  {
    this->Internals->FileBrowser = new QFileDialog(m_widget);
    this->Internals->FileBrowser->setObjectName("Select File Dialog");
    this->Internals->FileBrowser->setDirectory(QDir::currentPath());
  }
}

QWidget* qtFileItem::createFileBrowseWidget(int elementIdx,
  const smtk::attribute::FileSystemItem& item,
  const smtk::attribute::FileSystemItemDefinition& itemDef)
{
  QWidget* fileTextWidget = NULL;
  QComboBox* fileCombo = NULL;
  QLineEdit* lineEdit = NULL;
  QFrame* frame = new QFrame(m_itemInfo.parentWidget());
  //frame->setStyleSheet("QFrame { background-color: yellow; }");
  QString defaultText;
  if (item.type() == smtk::attribute::Item::FileType)
  {
    auto fDef = dynamic_cast<const attribute::FileItemDefinition&>(itemDef);
    if (fDef.hasDefault())
      defaultText = fDef.defaultValue().c_str();
    // For open Files, we use a combobox to show the recent file list
    if (fDef.shouldExist() && !item.isExtensible())
    {
      fileCombo = new QComboBox(frame);
      fileCombo->setEditable(true);
      fileTextWidget = fileCombo;
      this->Internals->fileCombo = fileCombo;
    }

    if (!fDef.shouldExist())
    {
      std::string filters = fDef.getFileFilters();
      regex re(";;");
      sregex_token_iterator it(filters.begin(), filters.end(), re, -1), last;
      if (it != last && std::next(it) != last)
      {
        this->Internals->fileExtCombo = new QComboBox(frame);
        this->Internals->fileExtCombo->setEditable(false);
        for (; it != last; ++it)
        {
          this->Internals->fileExtCombo->addItem(extractFileTypeName(it->str()));
          this->Internals->fileExtensions.push_back(extractFileTypeExtension(it->str()));
        }
        this->Internals->fileExtCombo->setMinimumContentsLength(8);
        this->Internals->fileExtCombo->setSizeAdjustPolicy(
          QComboBox::AdjustToMinimumContentsLength);
      }
    }
  }

  if (fileTextWidget == NULL)
  {
    lineEdit = new QLineEdit(frame);
    fileTextWidget = lineEdit;
  }

  // As a file input, if the name is too long lets favor
  // the file name over the path
  if (lineEdit)
  {
    lineEdit->setAlignment(Qt::AlignRight);
    lineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lineEdit->setMinimumWidth(5);
  }
  else if (fileCombo)
  {
    fileCombo->lineEdit()->setAlignment(Qt::AlignRight);
    fileCombo->setMinimumContentsLength(10);

    // http://doc.qt.io/qt-5/qcombobox.html#sizeAdjustPolicy-prop
    // Recommends using QComboBox::AdjustToContents, but that does not seem to
    // work on Linux.
    fileCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
  }

  frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  fileTextWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QPushButton* fileBrowserButton = new QPushButton("Browse", frame);
  fileBrowserButton->setMinimumHeight(fileTextWidget->height());
  fileBrowserButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QHBoxLayout* layout = new QHBoxLayout(frame);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(fileTextWidget);
  if (this->Internals->fileExtCombo)
  {
    layout->addWidget(this->Internals->fileExtCombo);
  }
  layout->addWidget(fileBrowserButton);
  layout->setAlignment(Qt::AlignCenter);

  QString valText = item.valueAsString(elementIdx).c_str();

  QVariant vdata;
  vdata.setValue(static_cast<void*>(fileTextWidget));
  this->setProperty("DataItem", vdata);
  QVariant vdata1(elementIdx);
  fileTextWidget->setProperty("ElementIndex", vdata1);

  this->updateFileComboList(valText);

  if (fileCombo)
    fileCombo->setCurrentIndex(fileCombo->findText(valText));
  else if (lineEdit)
    lineEdit->setText(valText);

  // Lets color the widget properly
  QFile qfile(item.value(elementIdx).c_str());
  if ((itemDef.isValueValid(item.value(elementIdx))) && (!itemDef.shouldExist() || qfile.exists()))
  {
    if (item.isUsingDefault(elementIdx))
    {
      m_itemInfo.uiManager()->setWidgetColorToDefault(fileTextWidget);
    }
    else
    {
      m_itemInfo.uiManager()->setWidgetColorToNormal(fileTextWidget);
    }
  }
  else
  {
    m_itemInfo.uiManager()->setWidgetColorToInvalid(fileTextWidget);
  }

  QObject::connect(
    fileBrowserButton, SIGNAL(clicked()), this->Internals->SignalMapper, SLOT(map()));
  this->Internals->SignalMapper->setMapping(fileBrowserButton, fileBrowserButton);

  // We use a QSignalMapper here to connect our signal, which comes from one of
  // potentially several lineEdits, fileCombos or fileBrowserButtons, to our
  // slot, which is the method setActiveField(QWidget*). This way,
  // setActiveField can tag the appropriate field to be used within
  // onInputValueChanged().
  //
  // TODO: This may be handled more elegantly using lambda expressions as
  // slots.

  if (lineEdit)
  {
    QObject::connect(
      lineEdit, SIGNAL(textChanged(const QString&)), this->Internals->SignalMapper, SLOT(map()));
    this->Internals->SignalMapper->setMapping(lineEdit, lineEdit);
    QObject::connect(this->Internals->SignalMapper, SIGNAL(mapped(QWidget*)), this,
      SLOT(setActiveField(QWidget*)));
    QObject::connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(onInputValueChanged()));
    this->Internals->SignalMapper->setMapping(fileBrowserButton, lineEdit);

    if (this->Internals->fileExtCombo)
    {
      QObject::connect(this->Internals->fileExtCombo, SIGNAL(currentTextChanged(const QString&)),
        this->Internals->SignalMapper, SLOT(map()));
      QObject::connect(this->Internals->fileExtCombo, SIGNAL(currentIndexChanged(int)),
        this->Internals->SignalMapper, SLOT(map()));
      this->Internals->SignalMapper->setMapping(this->Internals->fileExtCombo, lineEdit);
      QObject::connect(this->Internals->SignalMapper, SIGNAL(mapped(QWidget*)), this,
        SLOT(setActiveField(QWidget*)));

      QObject::connect(this->Internals->fileExtCombo, SIGNAL(currentIndexChanged(int)), this,
        SLOT(onInputValueChanged()));
    }
  }
  else if (fileCombo)
  {
    QObject::connect(fileCombo, SIGNAL(editTextChanged(const QString&)),
      this->Internals->SignalMapper, SLOT(map()));
    QObject::connect(
      fileCombo, SIGNAL(currentIndexChanged(int)), this->Internals->SignalMapper, SLOT(map()));
    this->Internals->SignalMapper->setMapping(fileCombo, fileCombo);
    QObject::connect(this->Internals->SignalMapper, SIGNAL(mapped(QWidget*)), this,
      SLOT(setActiveField(QWidget*)));

    QObject::connect(
      fileCombo->lineEdit(), SIGNAL(editingFinished()), this, SLOT(onInputValueChanged()));
    QObject::connect(
      fileCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onInputValueChanged()));
    this->Internals->SignalMapper->setMapping(fileBrowserButton, fileCombo);
  }

  QObject::connect(
    this->Internals->SignalMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveField(QWidget*)));
  QObject::connect(fileBrowserButton, SIGNAL(clicked()), this, SLOT(onLaunchFileBrowser()));

  return frame;
}

void qtFileItem::onInputValueChanged()
{
  QLineEdit* editBox = NULL;
  if (this->Internals->fileCombo)
  {
    editBox = this->Internals->fileCombo->lineEdit();
  }
  if (!editBox)
  {
    editBox = static_cast<QLineEdit*>(this->property("DataItem").value<void*>());
  }

  if (!editBox)
  {
    return;
  }

  auto item = m_itemInfo.itemAs<attribute::FileSystemItem>();
  int elementIdx = editBox->property("ElementIndex").toInt();
  std::string value = editBox->text().toStdString();

  if (!value.empty())
  {
    auto fSystemItemDef = item->definitionAs<attribute::FileSystemItemDefinition>();
    auto fItemDef = item->definitionAs<attribute::FileItemDefinition>();
    if (fItemDef && this->Internals->fileExtCombo)
    {
      int filterId = fItemDef->filterId(value);
      if (filterId != -1)
      {
        // the value has a suffix that matches our definition, so we set the
        // file type combobox to reflect the update.
        assert(this->Internals->fileExtCombo != nullptr);
        this->Internals->fileExtCombo->setCurrentIndex(filterId);
      }
    }
    // Lets see if we need to append a suffix
    if ((!fSystemItemDef->isValueValid(value)) && this->Internals->fileExtCombo)
    {
      value += this->Internals->fileExtensions.at(this->Internals->fileExtCombo->currentIndex())
                 .toStdString();

      // If the value is still not valid color the widget accordingly and just return
      if (!fSystemItemDef->isValueValid(value))
      {
        m_itemInfo.uiManager()->setWidgetColorToInvalid(editBox);
        return; // the value is not valid - nothing to set
      }
    }
    // Lets see if the value is different from the current one
    if (item->value(elementIdx) == value)
    {
      return; // value hasn't changed
    }

    item->setValue(elementIdx, value);
    emit this->modified();

    if (!this->isDirectory())
    {
      this->updateFileComboList(editBox->text());
    }
    auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
    if (iview)
    {
      iview->valueChanged(item);
    }

    QFile theFile(item->value(elementIdx).c_str());
    if ((fSystemItemDef->isValueValid(item->value(elementIdx))) &&
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
    emit(modified());
  }
}

bool qtFileItem::onLaunchFileBrowser()
{
  // If we are not using local file browser, just emit signal and return
  if (!this->Internals->FileBrowser)
  {
    return emit launchFileBrowser();
  }

  // If local file browser instantiated, get data from it
  auto item = m_itemInfo.itemAs<attribute::FileSystemItem>();

  QString filters;
  QFileDialog::FileMode mode = QFileDialog::AnyFile;
  if (this->Internals->IsDirectory)
  {
    mode = QFileDialog::Directory;
    auto itemDef = item->definitionAs<attribute::FileSystemItemDefinition>();
    this->Internals->FileBrowser->setOption(QFileDialog::ShowDirsOnly, itemDef->shouldExist());
  }
  else
  {
    auto fItemDef = item->definitionAs<attribute::FileItemDefinition>();
    filters = fItemDef->getFileFilters().c_str();
    mode = fItemDef->shouldExist() ? QFileDialog::ExistingFile : QFileDialog::AnyFile;
  }
  this->Internals->FileBrowser->setFileMode(mode);
  QStringList name_filters = filters.split(";;");
  this->Internals->FileBrowser->setNameFilters(name_filters);

  this->Internals->FileBrowser->setWindowModality(Qt::WindowModal);
  if (this->Internals->FileBrowser->exec() == QDialog::Accepted)
  {
    QStringList files = this->Internals->FileBrowser->selectedFiles();
    this->setInputValue(files[0]);
    return true;
  }
  return false;
}

void qtFileItem::updateFileComboList(const QString& newFile)
{
  if (this->Internals->fileCombo)
  {
    this->Internals->fileCombo->blockSignals(true);
    QString currentFile = this->Internals->fileCombo->currentText();
    this->Internals->fileCombo->clear();
    auto fItem = m_itemInfo.itemAs<attribute::FileItem>();
    fItem->addRecentValue(newFile.toStdString());
    std::vector<std::string>::const_iterator it;
    for (it = fItem->recentValues().begin(); it != fItem->recentValues().end(); ++it)
    {
      this->Internals->fileCombo->addItem((*it).c_str());
    }
    this->Internals->fileCombo->setCurrentIndex(this->Internals->fileCombo->findText(currentFile));
    this->Internals->fileCombo->blockSignals(false);
  }
}

void qtFileItem::setInputValue(const QString& val)
{
  QLineEdit* lineEdit = NULL;
  if (this->Internals->fileCombo)
  {
    lineEdit = this->Internals->fileCombo->lineEdit();
  }
  else
  {
    lineEdit = static_cast<QLineEdit*>(this->property("DataItem").value<void*>());
  }
  if (!lineEdit)
  {
    return;
  }

  QString value = val;

  // For files, check if the extension in val is valid. If it is not, append a
  // valid extension to it.
  if (!value.isEmpty() && !this->Internals->IsDirectory)
  {
    smtk::attribute::ItemPtr item = m_itemInfo.item();

    auto fItemDef = item->definitionAs<attribute::FileItemDefinition>();
    if (fItemDef->isValueValid(val.toStdString()) == false)
    {
      QFileInfo fi(val);

      std::string filters = fItemDef->getFileFilters();
      std::size_t begin = filters.find_first_of("*", filters.find_first_of("(")) + 1;
      std::size_t end = filters.find_first_of(" \n\r\t", begin);
      QString acceptableSuffix(filters.substr(begin, end - begin).c_str());

      value = fi.absolutePath() + fi.baseName() + acceptableSuffix;
    }
  }

  // this itself will not trigger onInputValueChanged
  lineEdit->setText(value);
  this->onInputValueChanged();
}

void qtFileItem::createWidget()
{
  smtk::attribute::ItemPtr item = m_itemInfo.item();
  auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
  if (iview && !iview->displayItem(item))
  {
    return;
  }

  this->clearChildWidgets();
  this->updateItemData();
}

void qtFileItem::updateItemData()
{
  this->updateUI();
  this->qtItem::updateItemData();
}

void qtFileItem::addInputEditor(int i, const smtk::attribute::FileSystemItem& item,
  const smtk::attribute::FileSystemItemDefinition& itemDef)
{
  int n = static_cast<int>(item.numberOfValues());
  if (!n)
  {
    return;
  }
  QBoxLayout* childLayout = NULL;
  childLayout = new QVBoxLayout;
  childLayout->setContentsMargins(12, 3, 3, 0);
  childLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QWidget* editBox = this->createFileBrowseWidget(i, item, itemDef);
  if (!editBox)
  {
    return;
  }

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QBoxLayout* editorLayout = new QHBoxLayout;
  editorLayout->setMargin(0);
  editorLayout->setSpacing(3);
  if (item.isExtensible())
  {
    QToolButton* minusButton = new QToolButton(m_widget);
    QString iconName(":/icons/attribute/minus.png");
    minusButton->setFixedSize(QSize(12, 12));
    minusButton->setIcon(QIcon(iconName));
    minusButton->setSizePolicy(sizeFixedPolicy);
    minusButton->setToolTip("Remove value");
    editorLayout->addWidget(minusButton);
    connect(minusButton, SIGNAL(clicked()), this, SLOT(onRemoveValue()));
    QPair<QPointer<QLayout>, QPointer<QWidget> > pair;
    pair.first = editorLayout;
    pair.second = editBox;
    this->Internals->ExtensibleMap[minusButton] = pair;
    this->Internals->MinusButtonIndices.push_back(minusButton);
  }

  if (n != 1 && itemDef.hasValueLabels())
  {
    std::string componentLabel = itemDef.valueLabel(i);
    if (!componentLabel.empty())
    {
      // acbauer -- this should probably be improved to look nicer
      QString labelText = componentLabel.c_str();
      QLabel* label = new QLabel(labelText, editBox);
      label->setSizePolicy(sizeFixedPolicy);
      editorLayout->addWidget(label);
    }
  }
  editorLayout->addWidget(editBox);

  // always going vertical for discrete and extensible items
  if (this->Internals->VectorItemOrient == Qt::Vertical || item.isExtensible())
  {
    int row = 2 * i;
    // The "Add New Value" button is in first row, so take that into account
    row = item.isExtensible() ? row + 1 : row;
    this->Internals->EntryLayout->addLayout(editorLayout, row, 1);

    // there could be conditional children, so we need another layout
    // so that the combobox will stay TOP-left when there are multiple
    // combo boxes.
    if (childLayout)
    {
      this->Internals->EntryLayout->addLayout(childLayout, row + 1, 0, 1, 2);
    }
  }
  else // going horizontal
  {
    this->Internals->EntryLayout->addLayout(editorLayout, 0, i + 1);
  }

  this->Internals->ChildrenMap[editBox] = childLayout;
  this->updateExtensibleState();
}

void qtFileItem::loadInputValues(const smtk::attribute::FileSystemItem& item,
  const smtk::attribute::FileSystemItemDefinition& itemDef)
{
  int n = static_cast<int>(item.numberOfValues());
  if (!n && !item.isExtensible())
  {
    return;
  }

  if (item.isExtensible())
  {
    if (!this->Internals->AddItemButton)
    {
      QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      this->Internals->AddItemButton = new QToolButton(m_widget);
      QString iconName(":/icons/attribute/plus.png");
      this->Internals->AddItemButton->setText("Add New Value");
      this->Internals->AddItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

      //      this->Internals->AddItemButton->setFixedSize(QSize(12, 12));
      this->Internals->AddItemButton->setIcon(QIcon(iconName));
      this->Internals->AddItemButton->setSizePolicy(sizeFixedPolicy);
      connect(this->Internals->AddItemButton, SIGNAL(clicked()), this, SLOT(onAddNewValue()));
      this->Internals->EntryLayout->addWidget(this->Internals->AddItemButton, 0, 1);
    }
  }

  for (int i = 0; i < n; i++)
  {
    this->addInputEditor(i, item, itemDef);
  }
}

void qtFileItem::updateUI()
{
  auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  if (iview && !iview->displayItem(item))
  {
    return;
  }

  if (m_widget)
  {
    delete m_widget;
  }

  m_widget = new QFrame(m_itemInfo.parentWidget());
  if (this->isReadOnly())
  {
    m_widget->setEnabled(false);
  }
  this->Internals->EntryLayout = new QGridLayout(m_widget);
  this->Internals->EntryLayout->setMargin(0);
  this->Internals->EntryLayout->setSpacing(0);
  this->Internals->EntryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if (item->isOptional())
  {
    QCheckBox* optionalCheck = new QCheckBox(m_itemInfo.parentWidget());
    optionalCheck->setChecked(item->isEnabled());
    optionalCheck->setText(" ");
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = optionalCheck->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)), this, SLOT(setOutputOptional(int)));
    labelLayout->addWidget(optionalCheck);
  }
  auto itemDef = item->definitionAs<attribute::FileSystemItemDefinition>();

  QString labelText;
  if (!item->label().empty())
  {
    labelText = item->label().c_str();
  }
  else
  {
    labelText = item->name().c_str();
  }
  QLabel* label = new QLabel(labelText, m_widget);
  label->setSizePolicy(sizeFixedPolicy);
  if (iview)
  {
    label->setFixedWidth(iview->fixedLabelWidth() - padding);
  }
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  //  qtOverlayFilter *filter = new qtOverlayFilter(this);
  //  label->installEventFilter(filter);

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
  this->Internals->theLabel = label;

  this->loadInputValues(*item, *itemDef);

  // we need this layout so that for items with conditionan children,
  // the label will line up at Top-left against the chilren's widgets.
  //  QVBoxLayout* vTLlayout = new QVBoxLayout;
  //  vTLlayout->setMargin(0);
  //  vTLlayout->setSpacing(0);
  //  vTLlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  //  vTLlayout->addLayout(labelLayout);
  this->Internals->EntryLayout->addLayout(labelLayout, 0, 0);
  //  layout->addWidget(this->Internals->EntryFrame, 0, 1);
  if (m_itemInfo.parentWidget() && m_itemInfo.parentWidget()->layout())
  {
    m_itemInfo.parentWidget()->layout()->addWidget(m_widget);
  }
  if (item->isOptional())
  {
    this->setOutputOptional(item->isEnabled() ? 1 : 0);
  }
}

void qtFileItem::setOutputOptional(int state)
{
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  if (!item)
  {
    return;
  }
  bool enable = state ? true : false;
  if (item->isExtensible())
  {
    if (this->Internals->AddItemButton)
    {
      this->Internals->AddItemButton->setVisible(enable);
    }
    foreach (QToolButton* tButton, this->Internals->ExtensibleMap.keys())
    {
      tButton->setVisible(enable);
    }
  }

  foreach (QWidget* cwidget, this->Internals->ChildrenMap.keys())
  {
    QLayout* childLayout = this->Internals->ChildrenMap.value(cwidget);
    if (childLayout)
    {
      for (int i = 0; i < childLayout->count(); ++i)
        childLayout->itemAt(i)->widget()->setVisible(enable);
    }
    cwidget->setVisible(enable);
  }

  //  this->Internals->EntryFrame->setEnabled(enable);
  if (enable != item->isEnabled())
  {
    item->setIsEnabled(enable);
    emit this->modified();
    auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
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
    //    QBoxLayout* entryLayout = qobject_cast<QBoxLayout*>(
    //      this->Internals->EntryFrame->layout());
    this->addInputEditor(static_cast<int>(item->numberOfValues()) - 1, *item, *itemDef);
    emit this->modified();
  }
}

void qtFileItem::onRemoveValue()
{
  QToolButton* const minusButton = qobject_cast<QToolButton*>(QObject::sender());
  if (!minusButton || !this->Internals->ExtensibleMap.contains(minusButton))
  {
    return;
  }

  int gIdx = this->Internals->MinusButtonIndices.indexOf(
    minusButton); //minusButton->property("SubgroupIndex").toInt();
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  if (!item || gIdx < 0 || gIdx >= static_cast<int>(item->numberOfValues()))
  {
    return;
  }

  QWidget* childwidget = this->Internals->ExtensibleMap.value(minusButton).second;
  QLayout* childLayout = this->Internals->ChildrenMap.value(childwidget);
  if (childLayout)
  {
    QLayoutItem* child;
    while ((child = childLayout->takeAt(0)) != 0)
    {
      delete child;
    }
    delete childLayout;
  }
  delete childwidget;
  delete this->Internals->ExtensibleMap.value(minusButton).first;
  this->Internals->ExtensibleMap.remove(minusButton);
  this->Internals->MinusButtonIndices.removeOne(minusButton);
  delete minusButton;

  item->removeValue(gIdx);
  emit this->modified();

  this->updateExtensibleState();
}

void qtFileItem::setActiveField(QWidget* activeField)
{
  QVariant vdata;
  vdata.setValue(static_cast<void*>(activeField));
  this->setProperty("DataItem", vdata);
}

void qtFileItem::updateExtensibleState()
{
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  if (!item || !item->isExtensible())
  {
    return;
  }
  bool maxReached =
    (item->maxNumberOfValues() > 0) && (item->maxNumberOfValues() == item->numberOfValues());
  this->Internals->AddItemButton->setEnabled(!maxReached);

  bool minReached = (item->numberOfRequiredValues() > 0) &&
    (item->numberOfRequiredValues() == item->numberOfValues());
  foreach (QToolButton* tButton, this->Internals->ExtensibleMap.keys())
  {
    tButton->setEnabled(!minReached);
  }
}

void qtFileItem::clearChildWidgets()
{
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  if (!item)
  {
    return;
  }

  if (item->isExtensible())
  {
    //clear mapping
    foreach (QToolButton* tButton, this->Internals->ExtensibleMap.keys())
    {
      // will delete later from this->Internals->ChildrenMap
      //      delete this->Internals->ExtensibleMap.value(tButton).second;
      delete this->Internals->ExtensibleMap.value(tButton).first;
      delete tButton;
    }
    this->Internals->ExtensibleMap.clear();
    this->Internals->MinusButtonIndices.clear();
  }

  foreach (QWidget* cwidget, this->Internals->ChildrenMap.keys())
  {
    QLayout* childLayout = this->Internals->ChildrenMap.value(cwidget);
    if (childLayout)
    {
      QLayoutItem* child;
      while ((child = childLayout->takeAt(0)) != 0)
      {
        delete child;
      }
      delete childLayout;
    }
    delete cwidget;
  }
  this->Internals->ChildrenMap.clear();
}
