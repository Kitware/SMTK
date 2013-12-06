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
#include "smtk/Qt/qtAttribute.h"

#include "smtk/Qt/qtUIManager.h"
#include "smtk/Qt/qtComboItem.h"
#include "smtk/Qt/qtGroupItem.h"
#include "smtk/Qt/qtInputsItem.h"
#include "smtk/Qt/qtFileItem.h"
#include "smtk/Qt/qtAttributeRefItem.h"
#include "smtk/Qt/qtVoidItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include <QPointer>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtAttributeInternals
{
public:
  qtAttributeInternals(smtk::attribute::AttributePtr dataObject, QWidget* p)
  {
  this->ParentWidget = p;
  this->DataObject = dataObject;
  }
  ~qtAttributeInternals()
  {
  }
 smtk::attribute::WeakAttributePtr DataObject;
 QPointer<QWidget> ParentWidget;
 QList<smtk::attribute::qtItem*> Items;
};

//----------------------------------------------------------------------------
qtAttribute::qtAttribute(smtk::attribute::AttributePtr dataObject, QWidget* p)
{ 
  this->Internals  = new qtAttributeInternals(dataObject, p); 
  this->Widget = NULL;
  //this->Internals->DataConnect = NULL;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtAttribute::~qtAttribute()
{
  this->clearItems();

  if (this->Internals)
    {
    if(this->Internals->ParentWidget && this->Widget
      && this->Internals->ParentWidget->layout())
      {
      this->Internals->ParentWidget->layout()->removeWidget(this->Widget);
      }
    delete this->Internals;
    }
}

//----------------------------------------------------------------------------
void qtAttribute::createWidget()
{
  if(!this->getObject() || !this->getObject()->numberOfItems())
    {
    return;
    }
  this->clearItems();

  this->Widget = new QFrame(this->parentWidget());
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);

//  QLabel* label = new QLabel(this->getObject()->name().c_str(),
//    this->parentWidget());
//  layout->addWidget(label);
  this->updateItemsData();
}

//----------------------------------------------------------------------------
void qtAttribute::addItem(qtItem* child)
{
  if(!this->Internals->Items.contains(child))
    {
    this->Internals->Items.append(child);
    }
}

//----------------------------------------------------------------------------
QList<qtItem*>& qtAttribute::items() const
{
  return this->Internals->Items;
} 

//----------------------------------------------------------------------------
void qtAttribute::clearItems()
{
  for(int i=0; i < this->Internals->Items.count(); i++)
    {
    delete this->Internals->Items.value(i);
    }
  this->Internals->Items.clear();
}

//----------------------------------------------------------------------------
void qtAttribute::updateItemsData()
{
  this->clearItems();
  QLayout* layout = this->Widget->layout();
  qtItem* qItem = NULL;
  smtk::attribute::AttributePtr att = this->getObject();
  std::size_t i, n = att->numberOfItems();
  for (i = 0; i < n; i++)
    {
    qItem = this->createItem(att->item(i), this->Widget);
    if(qItem)
      {
      layout->addWidget(qItem->widget());
      this->addItem(qItem);
      }
    }
}

//----------------------------------------------------------------------------
smtk::attribute::AttributePtr qtAttribute::getObject()
{
  return this->Internals->DataObject.lock();
}

//----------------------------------------------------------------------------
QWidget* qtAttribute::parentWidget()
{
  return this->Internals->ParentWidget;
}

//----------------------------------------------------------------------------
qtItem* qtAttribute::createItem(smtk::attribute::ItemPtr item, QWidget* pW)
{
  if(!qtUIManager::instance()->passAdvancedCheck(
      item->definition()->advanceLevel()) ||
    !qtUIManager::instance()->passItemCategoryCheck(
      item->definition()))
    {
    return NULL;
    }

  qtItem* aItem = NULL;
  switch (item->type())
    {
    case smtk::attribute::Item::ATTRIBUTE_REF: // This is always inside valueItem ???
      aItem = qtAttribute::createAttributeRefItem(smtk::dynamic_pointer_cast<RefItem>(item), pW);
      break;
    case smtk::attribute::Item::DOUBLE:
    case smtk::attribute::Item::INT:
    case smtk::attribute::Item::STRING:
      aItem = qtAttribute::createValueItem(smtk::dynamic_pointer_cast<ValueItem>(item), pW);
      break;
    case smtk::attribute::Item::DIRECTORY:
      aItem = qtAttribute::createDirectoryItem(smtk::dynamic_pointer_cast<DirectoryItem>(item), pW);
      break;
    case smtk::attribute::Item::FILE:
      aItem = qtAttribute::createFileItem(smtk::dynamic_pointer_cast<FileItem>(item), pW);
      break;
    case smtk::attribute::Item::GROUP:
      aItem = qtAttribute::createGroupItem(smtk::dynamic_pointer_cast<GroupItem>(item), pW);
      break;
    case smtk::attribute::Item::VOID:
      aItem = new qtVoidItem(smtk::dynamic_pointer_cast<VoidItem>(item), pW);
      break;
    default:
      //this->m_errorStatus << "Error: Unsupported Item Type: " <<
      // smtk::attribute::Item::type2String(item->type()) << "\n";
      break;
    }
  return aItem;
}

//----------------------------------------------------------------------------
qtItem* qtAttribute::createAttributeRefItem(
  smtk::attribute::RefItemPtr item, QWidget* pW)
{
  qtItem* returnItem = new qtAttributeRefItem(dynamic_pointer_cast<Item>(item), pW);
  return returnItem;
}
//----------------------------------------------------------------------------
qtItem* qtAttribute::createDirectoryItem(
  smtk::attribute::DirectoryItemPtr item, QWidget* pW)
{
  qtFileItem* returnItem = new qtFileItem(dynamic_pointer_cast<Item>(item), pW, true);
  qtUIManager::instance()->onFileItemCreated(returnItem);
  return returnItem;
}
//----------------------------------------------------------------------------
qtItem* qtAttribute::createFileItem(
  smtk::attribute::FileItemPtr item, QWidget* pW, bool dirOnly)
{
  qtFileItem* returnItem = new qtFileItem(
    dynamic_pointer_cast<Item>(item), pW, dirOnly);
  qtUIManager::instance()->onFileItemCreated(returnItem);
  return returnItem;
}
//----------------------------------------------------------------------------
qtItem* qtAttribute::createGroupItem(smtk::attribute::GroupItemPtr item, QWidget* pW)
{
  qtItem* returnItem = new qtGroupItem(dynamic_pointer_cast<Item>(item), pW);
  return returnItem;
}

//----------------------------------------------------------------------------
qtItem* qtAttribute::createValueItem(
  smtk::attribute::ValueItemPtr item, QWidget* pW)
{
    // create the input item for editable type values
  qtItem* returnItem = new qtInputsItem(dynamic_pointer_cast<Item>(item), pW);
  return returnItem;
}
