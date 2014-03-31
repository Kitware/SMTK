/*=========================================================================

Copyright (c) 1998-2014 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "smtk/Qt/qtFileItem.h"

#include "smtk/Qt/qtBaseView.h"
#include "smtk/Qt/qtUIManager.h"
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

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtFileItemInternals
{
public:
  bool IsDirectory;
  QFileDialog *FileBrowser;
  QPointer<QFrame> EntryFrame;
};

//----------------------------------------------------------------------------
qtFileItem::qtFileItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p, qtBaseView* bview, bool dirOnly)
   : qtItem(dataObj, p, bview)
{
  this->Internals = new qtFileItemInternals;
  this->Internals->IsDirectory = dirOnly;
  this->Internals->FileBrowser = NULL;
  this->IsLeafItem = true;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtFileItem::~qtFileItem()
{
  delete this->Internals;
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
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  layout->setAlignment(Qt::AlignTop);
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
  smtk::attribute::FileItemPtr fItem =dynamic_pointer_cast<FileItem>(this->getObject());
  smtk::attribute::DirectoryItemPtr dItem =dynamic_pointer_cast<DirectoryItem>(this->getObject());
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
  QVBoxLayout* entryLayout = new QVBoxLayout(this->Internals->EntryFrame);
  int spacing = entryLayout->spacing() / 2;  // reduce spacing
  entryLayout->setSpacing(spacing);

  // Add label
  smtk::attribute::ItemPtr item = dynamic_pointer_cast<Item>(this->getObject());
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
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  label->setSizePolicy(sizeFixedPolicy);

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
  entryLayout->addWidget(label);

  // Add file items
  for(i = 0; i < n; i++)
    {
    QWidget* fileframe = this->createFileBrowseWidget(static_cast<int>(i));
    entryLayout->addWidget(fileframe);
    }

  this->Widget->layout()->addWidget(this->Internals->EntryFrame);
}

//----------------------------------------------------------------------------
QWidget* qtFileItem::createFileBrowseWidget(int elementIdx)
{
  smtk::attribute::FileItemPtr fItem =dynamic_pointer_cast<FileItem>(this->getObject());
  smtk::attribute::DirectoryItemPtr dItem =dynamic_pointer_cast<DirectoryItem>(this->getObject());

  QFrame *frame = new QFrame(this->parentWidget());
  //frame->setStyleSheet("QFrame { background-color: yellow; }");
  QLineEdit* lineEdit = new QLineEdit(frame);
  // As a file input, if the name is too long lets favor
  // the file name over the path
  lineEdit->setAlignment(Qt::AlignRight);
  frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QPushButton* fileBrowserButton = new QPushButton("Browse", frame);
  QHBoxLayout* layout = new QHBoxLayout(frame);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(lineEdit);
  layout->addWidget(fileBrowserButton);

  QString defaultText;
  if (fItem)
    {
    const smtk::attribute::FileItemDefinition *fDef =
    dynamic_cast<const FileItemDefinition*>(fItem->definition().get());
    if (fDef && fDef->hasDefault())
      {
      defaultText = fDef->defaultValue().c_str();
      }
    }

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
  lineEdit->setText(valText);

  QVariant vdata;
  vdata.setValue(static_cast<void*>(lineEdit));
  this->setProperty("DataItem", vdata);

  QVariant vdata1(elementIdx);
  lineEdit->setProperty("ElementIndex", vdata1);
  QObject::connect(lineEdit, SIGNAL(textChanged(const QString &)),
    this, SLOT(onInputValueChanged()));

  QObject::connect(fileBrowserButton, SIGNAL(clicked()),
    this, SLOT(onLaunchFileBrowser()));

  return frame;
}

//----------------------------------------------------------------------------
void qtFileItem::onInputValueChanged()
{
  QLineEdit* const editBox = qobject_cast<QLineEdit*>(
    QObject::sender());
  if(!editBox)
    {
    return;
    }

  smtk::attribute::FileItemPtr fItem =dynamic_pointer_cast<FileItem>(this->getObject());
  smtk::attribute::DirectoryItemPtr dItem =dynamic_pointer_cast<DirectoryItem>(this->getObject());
  int elementIdx = editBox->property("ElementIndex").toInt();

  if(!editBox->text().isEmpty())
    {
    if(fItem)
      {
      fItem->setValue(elementIdx, editBox->text().toStdString());
      }
    else if(dItem)
      {
      dItem->setValue(elementIdx, editBox->text().toStdString());
      }
    this->baseView()->valueChanged(this);
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

    QLineEdit* lineEdit =  static_cast<QLineEdit*>(
      this->property("DataItem").value<void *>());
    if(!lineEdit)
      {
      return;
      }
    lineEdit->setText(files[0]);
    }
}
