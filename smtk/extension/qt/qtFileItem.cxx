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
  qtFileItemInternals() = default;
  ~qtFileItemInternals() = default;

  bool IsDirectory;
  QFileDialog* FileBrowser{ nullptr };
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
  m_internals = new qtFileItemInternals;
  m_internals->IsDirectory = (m_itemInfo.item()->type() == smtk::attribute::Item::DirectoryType);
  m_isLeafItem = true;
  m_internals->VectorItemOrient = Qt::Horizontal;
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
}

qtFileItem::~qtFileItem()
{
  delete m_internals;
}

void qtFileItem::setLabelVisible(bool visible)
{
  m_internals->theLabel->setVisible(visible);
}

bool qtFileItem::isDirectory()
{
  return m_internals->IsDirectory;
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
    m_internals->FileBrowser->setObjectName("Select File Dialog");
    m_internals->FileBrowser->setDirectory(QDir::currentPath());
  }
}

QWidget* qtFileItem::createFileBrowseWidget(int elementIdx,
  const smtk::attribute::FileSystemItem& item,
  const smtk::attribute::FileSystemItemDefinition& itemDef)
{
  QWidget* fileTextWidget = nullptr;
  QComboBox* fileCombo = nullptr;
  QLineEdit* lineEdit = nullptr;
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
      m_internals->fileCombo = fileCombo;
    }

    if (!fDef.shouldExist())
    {
      std::string filters = fDef.getFileFilters();
      regex re(";;");
      sregex_token_iterator it(filters.begin(), filters.end(), re, -1), last;
      if (it != last && std::next(it) != last)
      {
        m_internals->fileExtCombo = new QComboBox(frame);
        m_internals->fileExtCombo->setEditable(false);
        for (; it != last; ++it)
        {
          m_internals->fileExtCombo->addItem(extractFileTypeName(it->str()));
          m_internals->fileExtensions.push_back(extractFileTypeExtension(it->str()));
        }
        m_internals->fileExtCombo->setMinimumContentsLength(8);
        m_internals->fileExtCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
      }
    }
  }

  if (fileTextWidget == nullptr)
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
  if (m_internals->fileExtCombo)
  {
    layout->addWidget(m_internals->fileExtCombo);
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

  if (lineEdit)
  {
    QObject::connect(lineEdit, &QLineEdit::textChanged, [=]() { setActiveField(lineEdit); });

    QObject::connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(onInputValueChanged()));

    if (m_internals->fileExtCombo)
    {
      QObject::connect(m_internals->fileExtCombo, &QComboBox::currentTextChanged,
        [=]() { setActiveField(lineEdit); });
      QObject::connect(m_internals->fileExtCombo,
        (void (QComboBox::*)(int)) & QComboBox::currentIndexChanged,
        [=]() { setActiveField(lineEdit); });

      QObject::connect(m_internals->fileExtCombo, SIGNAL(currentIndexChanged(int)), this,
        SLOT(onInputValueChanged()));
    }

    QObject::connect(fileBrowserButton, &QPushButton::clicked, [=]() { setActiveField(lineEdit); });
  }
  else if (fileCombo)
  {
    QObject::connect(
      fileCombo, &QComboBox::currentTextChanged, [=]() { setActiveField(fileCombo); });
    QObject::connect(fileCombo, (void (QComboBox::*)(int)) & QComboBox::currentIndexChanged,
      [=]() { setActiveField(fileCombo); });

    QObject::connect(
      fileCombo->lineEdit(), SIGNAL(editingFinished()), this, SLOT(onInputValueChanged()));
    QObject::connect(
      fileCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onInputValueChanged()));

    QObject::connect(
      fileBrowserButton, &QPushButton::clicked, [=]() { setActiveField(fileCombo); });
  }

  QObject::connect(fileBrowserButton, SIGNAL(clicked()), this, SLOT(onLaunchFileBrowser()));

  return frame;
}

void qtFileItem::onInputValueChanged()
{
  QLineEdit* editBox = nullptr;
  if (m_internals->fileCombo)
  {
    editBox = m_internals->fileCombo->lineEdit();
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
    if (fItemDef && m_internals->fileExtCombo)
    {
      int filterId = fItemDef->filterId(value);
      if (filterId != -1)
      {
        // the value has a suffix that matches our definition, so we set the
        // file type combobox to reflect the update.
        assert(m_internals->fileExtCombo != nullptr);
        m_internals->fileExtCombo->setCurrentIndex(filterId);
      }
    }
    // Lets see if we need to append a suffix
    if ((!fSystemItemDef->isValueValid(value)) && m_internals->fileExtCombo)
    {
      value +=
        m_internals->fileExtensions.at(m_internals->fileExtCombo->currentIndex()).toStdString();

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
  if (!m_internals->FileBrowser)
  {
    return emit launchFileBrowser();
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
  QStringList name_filters = filters.split(";;");
  m_internals->FileBrowser->setNameFilters(name_filters);
  // This is needed for Macs since mode alone is not enough to allow the user to
  // chose a non-existent file
  if (mode == QFileDialog::AnyFile)
  {
    m_internals->FileBrowser->setAcceptMode(QFileDialog::AcceptSave);
    m_internals->FileBrowser->setLabelText(QFileDialog::FileName, "File name:");
  }
  m_internals->FileBrowser->setLabelText(QFileDialog::Accept, "Accept");
  if (m_internals->FileBrowser->exec() == QDialog::Accepted)
  {
    QStringList files = m_internals->FileBrowser->selectedFiles();
    this->setInputValue(files[0]);
    return true;
  }
  return false;
}

void qtFileItem::updateFileComboList(const QString& newFile)
{
  if (m_internals->fileCombo)
  {
    m_internals->fileCombo->blockSignals(true);
    QString currentFile = m_internals->fileCombo->currentText();
    m_internals->fileCombo->clear();
    auto fItem = m_itemInfo.itemAs<attribute::FileItem>();
    fItem->addRecentValue(newFile.toStdString());
    std::vector<std::string>::const_iterator it;
    for (it = fItem->recentValues().begin(); it != fItem->recentValues().end(); ++it)
    {
      m_internals->fileCombo->addItem((*it).c_str());
    }
    m_internals->fileCombo->setCurrentIndex(m_internals->fileCombo->findText(currentFile));
    m_internals->fileCombo->blockSignals(false);
  }
}

void qtFileItem::setInputValue(const QString& val)
{
  QLineEdit* lineEdit = nullptr;
  if (m_internals->fileCombo)
  {
    lineEdit = m_internals->fileCombo->lineEdit();
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
  if (!value.isEmpty() && !m_internals->IsDirectory)
  {
    smtk::attribute::ItemPtr item = m_itemInfo.item();

    auto fItemDef = item->definitionAs<attribute::FileItemDefinition>();
    if (!fItemDef->isValueValid(val.toStdString()))
    {
      QFileInfo fi(val);

      std::string filters = fItemDef->getFileFilters();
      std::size_t begin = filters.find_first_of('*', filters.find_first_of('(')) + 1;
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
  QBoxLayout* childLayout = nullptr;
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
    m_internals->ExtensibleMap[minusButton] = pair;
    m_internals->MinusButtonIndices.push_back(minusButton);
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
  if (m_internals->VectorItemOrient == Qt::Vertical || item.isExtensible())
  {
    int row = 2 * i;
    // The "Add New Value" button is in first row, so take that into account
    row = item.isExtensible() ? row + 1 : row;
    m_internals->EntryLayout->addLayout(editorLayout, row, 1);

    // there could be conditional children, so we need another layout
    // so that the combobox will stay TOP-left when there are multiple
    // combo boxes.
    if (childLayout)
    {
      m_internals->EntryLayout->addLayout(childLayout, row + 1, 0, 1, 2);
    }
  }
  else // going horizontal
  {
    m_internals->EntryLayout->addLayout(editorLayout, 0, i + 1);
  }

  m_internals->ChildrenMap[editBox] = childLayout;
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
    if (!m_internals->AddItemButton)
    {
      QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      m_internals->AddItemButton = new QToolButton(m_widget);
      QString iconName(":/icons/attribute/plus.png");
      m_internals->AddItemButton->setText("Add New Value");
      m_internals->AddItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

      //      m_internals->AddItemButton->setFixedSize(QSize(12, 12));
      m_internals->AddItemButton->setIcon(QIcon(iconName));
      m_internals->AddItemButton->setSizePolicy(sizeFixedPolicy);
      connect(m_internals->AddItemButton, SIGNAL(clicked()), this, SLOT(onAddNewValue()));
      m_internals->EntryLayout->addWidget(m_internals->AddItemButton, 0, 1);
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
  m_internals->EntryLayout = new QGridLayout(m_widget);
  m_internals->EntryLayout->setMargin(0);
  m_internals->EntryLayout->setSpacing(0);
  m_internals->EntryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if (item->isOptional())
  {
    QCheckBox* optionalCheck = new QCheckBox(m_itemInfo.parentWidget());
    optionalCheck->setChecked(item->localEnabledState());
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
  m_internals->theLabel = label;

  this->loadInputValues(*item, *itemDef);

  // we need this layout so that for items with conditionan children,
  // the label will line up at Top-left against the chilren's widgets.
  //  QVBoxLayout* vTLlayout = new QVBoxLayout;
  //  vTLlayout->setMargin(0);
  //  vTLlayout->setSpacing(0);
  //  vTLlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  //  vTLlayout->addLayout(labelLayout);
  m_internals->EntryLayout->addLayout(labelLayout, 0, 0);
  //  layout->addWidget(m_internals->EntryFrame, 0, 1);
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
  if (!item)
  {
    return;
  }
  bool enable = state != 0;
  if (item->isExtensible())
  {
    if (m_internals->AddItemButton)
    {
      m_internals->AddItemButton->setVisible(enable);
    }
    foreach (QToolButton* tButton, m_internals->ExtensibleMap.keys())
    {
      tButton->setVisible(enable);
    }
  }

  foreach (QWidget* cwidget, m_internals->ChildrenMap.keys())
  {
    QLayout* childLayout = m_internals->ChildrenMap.value(cwidget);
    if (childLayout)
    {
      for (int i = 0; i < childLayout->count(); ++i)
        childLayout->itemAt(i)->widget()->setVisible(enable);
    }
    cwidget->setVisible(enable);
  }

  //  m_internals->EntryFrame->setEnabled(enable);
  if (enable != item->localEnabledState())
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
    //      m_internals->EntryFrame->layout());
    this->addInputEditor(static_cast<int>(item->numberOfValues()) - 1, *item, *itemDef);
    emit this->modified();
  }
}

void qtFileItem::onRemoveValue()
{
  QToolButton* const minusButton = qobject_cast<QToolButton*>(QObject::sender());
  if (!minusButton || !m_internals->ExtensibleMap.contains(minusButton))
  {
    return;
  }

  int gIdx = m_internals->MinusButtonIndices.indexOf(
    minusButton); //minusButton->property("SubgroupIndex").toInt();
  auto item = m_itemInfo.itemAs<FileSystemItem>();
  if (!item || gIdx < 0 || gIdx >= static_cast<int>(item->numberOfValues()))
  {
    return;
  }

  QWidget* childwidget = m_internals->ExtensibleMap.value(minusButton).second;
  QLayout* childLayout = m_internals->ChildrenMap.value(childwidget);
  if (childLayout)
  {
    QLayoutItem* child;
    while ((child = childLayout->takeAt(0)) != nullptr)
    {
      delete child;
    }
    delete childLayout;
  }
  delete childwidget;
  delete m_internals->ExtensibleMap.value(minusButton).first;
  m_internals->ExtensibleMap.remove(minusButton);
  m_internals->MinusButtonIndices.removeOne(minusButton);
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
  m_internals->AddItemButton->setEnabled(!maxReached);

  bool minReached = (item->numberOfRequiredValues() > 0) &&
    (item->numberOfRequiredValues() == item->numberOfValues());
  foreach (QToolButton* tButton, m_internals->ExtensibleMap.keys())
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
    foreach (QToolButton* tButton, m_internals->ExtensibleMap.keys())
    {
      // will delete later from m_internals->ChildrenMap
      //      delete m_internals->ExtensibleMap.value(tButton).second;
      delete m_internals->ExtensibleMap.value(tButton).first;
      delete tButton;
    }
    m_internals->ExtensibleMap.clear();
    m_internals->MinusButtonIndices.clear();
  }

  foreach (QWidget* cwidget, m_internals->ChildrenMap.keys())
  {
    QLayout* childLayout = m_internals->ChildrenMap.value(cwidget);
    if (childLayout)
    {
      QLayoutItem* child;
      while ((child = childLayout->takeAt(0)) != nullptr)
      {
        delete child;
      }
      delete childLayout;
    }
    delete cwidget;
  }
  m_internals->ChildrenMap.clear();
}
