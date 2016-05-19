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

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"

#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGridLayout>
#include <QComboBox>

using namespace smtk::extension;

//----------------------------------------------------------------------------
class qtFileItemInternals
{
public:
  bool IsDirectory;
  QFileDialog *FileBrowser;
  QPointer<QFrame> EntryFrame;
  QPointer<QLabel> theLabel;
  QPointer<QComboBox> fileCombo;
  Qt::Orientation VectorItemOrient;
};

//----------------------------------------------------------------------------
qtFileItem::qtFileItem(smtk::attribute::FileItemPtr dataObj, QWidget* p,
                       qtBaseView* bview, Qt::Orientation enVectorItemOrient)
   : qtItem(dataObj, p, bview)
{
  this->Internals = new qtFileItemInternals;
  this->Internals->IsDirectory = false;
  this->Internals->FileBrowser = NULL;
  this->Internals->VectorItemOrient = enVectorItemOrient;

  this->IsLeafItem = true;
  this->createWidget();
  if (bview)
    {
    bview->uiManager()->onFileItemCreated(this);
    }
}
//----------------------------------------------------------------------------
qtFileItem::qtFileItem(smtk::attribute::DirectoryItemPtr dataObj, QWidget* p,
                       qtBaseView* bview, Qt::Orientation enVectorItemOrient)
   : qtItem(dataObj, p, bview)
{
  this->Internals = new qtFileItemInternals;
  this->Internals->IsDirectory = true;
  this->Internals->FileBrowser = NULL;
  this->Internals->VectorItemOrient = enVectorItemOrient;

  this->IsLeafItem = true;
  this->createWidget();
  if (bview)
    {
    bview->uiManager()->onFileItemCreated(this);
    }
}

//----------------------------------------------------------------------------
qtFileItem::~qtFileItem()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtFileItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
}

//----------------------------------------------------------------------------
void qtFileItem::createWidget()
{
  if(!this->getObject())
    {
    return;
    }
  this->clearChildItems();
  this->Widget = new QFrame(this->parentWidget());
  QGridLayout* layout = new QGridLayout(this->Widget);
  layout->setMargin(0);
  layout->setSpacing(0);
  layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  this->updateItemData();
}
//----------------------------------------------------------------------------
bool qtFileItem::isDirectory()
{
  return this->Internals->IsDirectory;
}

//----------------------------------------------------------------------------
// Although you *can* disable this feature, it is not recommended.
// Behavior is not defined if this method is called after
// the ancestor qtUIManager::initializeUI() method is called.
void qtFileItem::enableFileBrowser(bool state)
{
  if (!state)
    {
    delete this->Internals->FileBrowser;
    this->Internals->FileBrowser = NULL;
    }
  else if (NULL == this->Internals->FileBrowser)
    {
    this->Internals->FileBrowser = new QFileDialog(this->Widget);
    this->Internals->FileBrowser->setObjectName("Select File Dialog");
    this->Internals->FileBrowser->setDirectory(QDir::currentPath());
    }
}

//----------------------------------------------------------------------------
void qtFileItem::updateItemData()
{
  smtk::attribute::FileItemPtr fItem =dynamic_pointer_cast<attribute::FileItem>(this->getObject());
  smtk::attribute::DirectoryItemPtr dItem =dynamic_pointer_cast<attribute::DirectoryItem>(this->getObject());
  if(!fItem && !dItem)
    {
    return;
    }
  std::size_t i, n = fItem ? fItem->numberOfValues() : dItem->numberOfValues();
  if (!n)
    {
    return;
    }

  if(this->Internals->EntryFrame)
    {
    this->Widget->layout()->removeWidget(this->Internals->EntryFrame);
    delete this->Internals->EntryFrame;
    }

  this->Internals->EntryFrame = new QFrame(this->parentWidget());
  this->Internals->EntryFrame->setObjectName("FileBrowsingFrame");
  //this->Internals->EntryFrame->setStyleSheet("QFrame { background-color: pink; }");

  // Setup layout
  QBoxLayout* entryLayout;
  if(this->Internals->VectorItemOrient == Qt::Vertical)
    {
    entryLayout = new QVBoxLayout(this->Internals->EntryFrame);
    }
  else
    {
    entryLayout = new QHBoxLayout(this->Internals->EntryFrame);
    }

  int spacing = entryLayout->spacing() / 2;  // reduce spacing
  entryLayout->setSpacing(spacing);
  entryLayout->setMargin(0);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if(this->getObject()->isOptional())
    {
    QCheckBox* optionalCheck = new QCheckBox(this->parentWidget());
    optionalCheck->setChecked(this->getObject()->isEnabled());
    optionalCheck->setText(" ");
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = optionalCheck->iconSize().width() + 3; // 3 is for layout spacing
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)),
      this, SLOT(setOutputOptional(int)));
    this->Internals->EntryFrame->setEnabled(this->getObject()->isEnabled());
    labelLayout->addWidget(optionalCheck);
    }

  // Add label
  smtk::attribute::ItemPtr item = dynamic_pointer_cast<attribute::Item>(this->getObject());
  QString labelText;
  if(!item->label().empty())
    {
    labelText = item->label().c_str();
    }
  else
    {
    labelText = item->name().c_str();
    }
  QLabel* label = new QLabel(labelText, this->Widget);
  //label->setStyleSheet("QLabel { background-color: lightblue; }");
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  label->setFixedWidth(this->baseView()->fixedLabelWidth() - padding);
  label->setWordWrap(true);
  label->setSizePolicy(sizeFixedPolicy);
  this->Internals->theLabel = label;
  labelLayout->addWidget(label);

  // Add in BriefDescription as tooltip if available
  smtk::attribute::ConstItemDefinitionPtr itemDef = item->definition();
  const std::string strBriefDescription = itemDef->briefDescription();
  if(!strBriefDescription.empty())
    {
    label->setToolTip(strBriefDescription.c_str());
    }

  // If advanced level, update font
  if(itemDef->advanceLevel())
    {
    label->setFont(this->baseView()->uiManager()->advancedFont());
    }
//  entryLayout->addWidget(label);
  QGridLayout* thisLayout = qobject_cast<QGridLayout*>(this->Widget->layout());
  thisLayout->addLayout(labelLayout, 0 , 0);

  // Add file items
  for(i = 0; i < n; i++)
    {
    QWidget* fileframe = this->createFileBrowseWidget(static_cast<int>(i));
    entryLayout->addWidget(fileframe);
    }

//  this->Widget->layout()->addWidget(this->Internals->EntryFrame);
  thisLayout->addWidget(this->Internals->EntryFrame, 0 , 1);
  this->qtItem::updateItemData();
}

//----------------------------------------------------------------------------
QWidget* qtFileItem::createFileBrowseWidget(int elementIdx)
{
  smtk::attribute::FileItemPtr fItem =dynamic_pointer_cast<attribute::FileItem>(this->getObject());
  smtk::attribute::DirectoryItemPtr dItem =dynamic_pointer_cast<attribute::DirectoryItem>(this->getObject());

  QWidget* fileTextWidget = NULL;
  QComboBox* fileCombo = NULL;
  QLineEdit* lineEdit = NULL;
  QFrame *frame = new QFrame(this->parentWidget());
  //frame->setStyleSheet("QFrame { background-color: yellow; }");
  QString defaultText;
  if (fItem)
    {
    const smtk::attribute::FileItemDefinition *fDef =
      dynamic_cast<const attribute::FileItemDefinition*>(fItem->definition().get());
    if (fDef->hasDefault())
      defaultText = fDef->defaultValue().c_str();
    // For open Files, we use a combobox to show the recent file list
    if(fDef->shouldExist())
      {
      fileCombo = new QComboBox(frame);
      fileCombo->setEditable(true);
      fileTextWidget = fileCombo;
      this->Internals->fileCombo = fileCombo;
      }
    }

  if(fileTextWidget == NULL)
    {
    lineEdit = new QLineEdit(frame);
    fileTextWidget = lineEdit;
    }

  // As a file input, if the name is too long lets favor
  // the file name over the path
  if(lineEdit)
    lineEdit->setAlignment(Qt::AlignRight);
  else if(fileCombo)
    fileCombo->lineEdit()->setAlignment(Qt::AlignRight);

  frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  fileTextWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QPushButton* fileBrowserButton = new QPushButton("Browse", frame);
  fileBrowserButton->setMinimumHeight(fileTextWidget->height());
  fileBrowserButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QHBoxLayout* layout = new QHBoxLayout(frame);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(fileTextWidget);
  layout->addWidget(fileBrowserButton);
  layout->setAlignment(Qt::AlignCenter);

  QString valText;
  if(fItem && fItem->isSet(elementIdx))
    {
    valText = fItem->valueAsString(elementIdx).c_str();
    }
  else if(dItem && dItem->isSet(elementIdx))
    {
    valText = dItem->valueAsString(elementIdx).c_str();
    }
  else if (defaultText != "")
    {
    valText = defaultText;
    }

  QVariant vdata;
  vdata.setValue(static_cast<void*>(fileTextWidget));
  this->setProperty("DataItem", vdata);

  QVariant vdata1(elementIdx);
  fileTextWidget->setProperty("ElementIndex", vdata1);
  this->updateFileComboList();
 
  if(fileCombo)
    fileCombo->setCurrentIndex(fileCombo->findText(valText));
  else if(lineEdit)
    lineEdit->setText(valText);

  if(lineEdit)
    QObject::connect(lineEdit, SIGNAL(textChanged(const QString &)),
      this, SLOT(onInputValueChanged()));
  else if(fileCombo)
    {
    QObject::connect(fileCombo, SIGNAL(editTextChanged(const QString &)),
      this, SLOT(onInputValueChanged()));
    QObject::connect(fileCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(onInputValueChanged()));    
    }
  QObject::connect(fileBrowserButton, SIGNAL(clicked()),
    this, SLOT(onLaunchFileBrowser()));

  return frame;
}
//----------------------------------------------------------------------------
void qtFileItem::setInputValue(const QString& val)
{
  QLineEdit* lineEdit =  NULL;
  if(this->Internals->fileCombo)
    lineEdit = this->Internals->fileCombo->lineEdit();
  else
    lineEdit = static_cast<QLineEdit*>(
      this->property("DataItem").value<void *>());
  if(!lineEdit)
    {
    return;
    }
  // this itself will not trigger onInputValueChanged
  lineEdit->setText(val);
  this->onInputValueChanged();
}

//----------------------------------------------------------------------------
void qtFileItem::onInputValueChanged()
{
  QLineEdit* editBox = NULL;
  if(this->Internals->fileCombo)
    editBox = this->Internals->fileCombo->lineEdit();
  if(!editBox)
    editBox = static_cast<QLineEdit*>(
      this->property("DataItem").value<void *>());

  if(!editBox)
    {
    return;
    }

  smtk::attribute::FileItemPtr fItem =dynamic_pointer_cast<attribute::FileItem>(this->getObject());
  smtk::attribute::DirectoryItemPtr dItem =dynamic_pointer_cast<attribute::DirectoryItem>(this->getObject());
  int elementIdx = editBox->property("ElementIndex").toInt();

  if((fItem && fItem->isSet(elementIdx) && fItem->value(elementIdx) == editBox->text().toStdString()) ||
     (dItem && dItem->isSet(elementIdx) && dItem->value(elementIdx) == editBox->text().toStdString()))
    {
    return;
    }
  if(!editBox->text().isEmpty())
    {
    if(fItem)
      {
      fItem->setValue(elementIdx, editBox->text().toStdString());
      this->updateFileComboList();
      }
    else if(dItem)
      {
      dItem->setValue(elementIdx, editBox->text().toStdString());
      }
    this->baseView()->valueChanged(this->getObject());
    }
  else if(fItem)
    {
    fItem->unset(elementIdx);
    }
  else if(dItem)
    {
    dItem->unset(elementIdx);
    }
}

//----------------------------------------------------------------------------
void qtFileItem::onLaunchFileBrowser()
{
  // If we are not using local file browser, just emit signal and return
  if (!this->Internals->FileBrowser)
    {
    emit this->launchFileBrowser();
    return;
    }

  // If local file browser instantiated, get data from it
  smtk::attribute::DirectoryItemPtr dItem;
  smtk::attribute::FileItemPtr fItem;
  smtk::attribute::ItemPtr item =
    smtk::dynamic_pointer_cast<smtk::attribute::Item>(this->getObject());

  QString filters;
  QFileDialog::FileMode mode = QFileDialog::AnyFile;
  if (this->Internals->IsDirectory)
    {
    dItem = smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItem>(item);
    const smtk::attribute::DirectoryItemDefinition *dItemDef =
      dynamic_cast<const smtk::attribute::DirectoryItemDefinition*>(dItem->definition().get());
    mode = QFileDialog::Directory;
    this->Internals->FileBrowser->setOption(QFileDialog::ShowDirsOnly,
      dItemDef->shouldExist());
    }
  else
    {
    fItem = smtk::dynamic_pointer_cast<smtk::attribute::FileItem>(item);
    const smtk::attribute::FileItemDefinition *fItemDef =
      dynamic_cast<const smtk::attribute::FileItemDefinition*>(fItem->definition().get());
    filters = fItemDef->getFileFilters().c_str();
    mode = fItemDef->shouldExist() ? QFileDialog::ExistingFile :
      QFileDialog::AnyFile;
    }
  this->Internals->FileBrowser->setFileMode(mode);
  this->Internals->FileBrowser->setFilter(filters);

  this->Internals->FileBrowser->setWindowModality(Qt::WindowModal);
  if (this->Internals->FileBrowser->exec() == QDialog::Accepted)
    {
    QStringList files = this->Internals->FileBrowser->selectedFiles();
    this->setInputValue(files[0]);
    }
}

//----------------------------------------------------------------------------
void qtFileItem::setOutputOptional(int state)
{
  bool enable = state ? true : false;
  this->Internals->EntryFrame->setEnabled(enable);
  if(enable != this->getObject()->isEnabled())
    {
    this->getObject()->setIsEnabled(enable);
    this->baseView()->valueChanged(this->getObject());
    }
}

//----------------------------------------------------------------------------
void qtFileItem::updateFileComboList()
{
  if (this->Internals->fileCombo)
    {
    this->Internals->fileCombo->blockSignals(true);
    QString currentFile = this->Internals->fileCombo->currentText();
    this->Internals->fileCombo->clear();
    smtk::attribute::FileItemPtr fItem =dynamic_pointer_cast<attribute::FileItem>(this->getObject());
    std::vector<std::string>::const_iterator it;
    for(it = fItem->recentValues().begin();
        it != fItem->recentValues().end(); ++it)
      {
      this->Internals->fileCombo->addItem((*it).c_str());
      }
    this->Internals->fileCombo->setCurrentIndex(
      this->Internals->fileCombo->findText(currentFile));
    this->Internals->fileCombo->blockSignals(false);
    }
}
