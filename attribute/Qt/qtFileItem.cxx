/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
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

#include "qtFileItem.h"

#include "qtUIManager.h"
#include "attribute/FileItem.h"
#include "attribute/FileItemDefinition.h"
#include "attribute/DirectoryItem.h"
#include "attribute/DirectoryItemDefinition.h"

#include <QLineEdit>
#include <QFrame>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPointer>

using namespace slctk::attribute;

//----------------------------------------------------------------------------
class qtFileItemInternals
{
public:
  bool IsDirectory;
  QPointer<QFrame> EntryFrame;
};

//----------------------------------------------------------------------------
qtFileItem::qtFileItem(
  slctk::AttributeItemPtr dataObj, QWidget* p, bool dirOnly)
   : qtItem(dataObj, p)
{
  this->Internals = new qtFileItemInternals;
  this->Internals->IsDirectory = dirOnly;
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
void qtFileItem::updateItemData()
{
  slctk::FileItemPtr fItem =dynamicCastPointer<FileItem>(this->getObject());
  slctk::DirectoryItemPtr dItem =dynamicCastPointer<DirectoryItem>(this->getObject());
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
  QVBoxLayout* entryLayout = new QVBoxLayout(this->Internals->EntryFrame);
  entryLayout->setMargin(0);

  for(i = 0; i < n; i++)
    {
    QWidget* fileframe = this->createFileBrowseWidget((int)i);
    entryLayout->addWidget(fileframe);
    }

  this->Widget->layout()->addWidget(this->Internals->EntryFrame);
}

//----------------------------------------------------------------------------
QWidget* qtFileItem::createFileBrowseWidget(int elementIdx)
{
  slctk::FileItemPtr fItem =dynamicCastPointer<FileItem>(this->getObject());
  slctk::DirectoryItemPtr dItem =dynamicCastPointer<DirectoryItem>(this->getObject());

  QFrame *frame = new QFrame(this->parentWidget());
  QLineEdit* lineEdit = new QLineEdit(frame);
  // As a file input, if the name is too long lets favor
  // the file name over the path
  lineEdit->setAlignment(Qt::AlignRight);
  frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QPushButton* fileBrowserButton = new QPushButton("Browse", frame);
  QHBoxLayout* layout = new QHBoxLayout(frame);
  layout->addWidget(lineEdit);
  layout->addWidget(fileBrowserButton);

  QString valText;
  if(fItem && fItem->isSet(elementIdx))
    {
    valText = fItem->valueAsString(elementIdx).c_str();
    }
  else if(dItem && dItem->isSet(elementIdx))
    {
    valText = dItem->valueAsString(elementIdx).c_str();
    }
  lineEdit->setText(valText);

  QVariant vdata;
  vdata.setValue((void*)lineEdit);
  this->setProperty("DataItem", vdata);

  QVariant vdata1(elementIdx);
  lineEdit->setProperty("ElementIndex", vdata1);
  QObject::connect(lineEdit, SIGNAL(editingFinished()),
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

  slctk::FileItemPtr fItem =dynamicCastPointer<FileItem>(this->getObject());
  slctk::DirectoryItemPtr dItem =dynamicCastPointer<DirectoryItem>(this->getObject());
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
