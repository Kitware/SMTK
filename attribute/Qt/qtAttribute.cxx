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
#include "qtAttribute.h"

#include "qtUIManager.h"
#include "qtComboItem.h"
#include "qtGroupItem.h"
#include "qtInputsItem.h"
#include "qtFileItem.h"
#include "qtAttributeRefItem.h"

#include "attribute/Attribute.h"
#include "attribute/Definition.h"
#include "attribute/DoubleItem.h"
#include "attribute/DoubleItemDefinition.h"
#include "attribute/DirectoryItem.h"
#include "attribute/DirectoryItemDefinition.h"
#include "attribute/FileItem.h"
#include "attribute/FileItemDefinition.h"
#include "attribute/GroupItem.h"
#include "attribute/GroupItemDefinition.h"
#include "attribute/IntItem.h"
#include "attribute/IntItemDefinition.h"
#include "attribute/ValueItem.h"
#include "attribute/ValueItemDefinition.h"

#include <QPointer>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

using namespace slctk::attribute;

//----------------------------------------------------------------------------
class qtAttributeInternals
{
public:
  qtAttributeInternals(slctk::AttributePtr dataObject, QWidget* p)
  {
  this->ParentWidget = p;
  this->DataObject = dataObject;
  }
  ~qtAttributeInternals()
  {
  }
 slctk::WeakAttributePtr DataObject;
 QPointer<QWidget> ParentWidget;
 QList<slctk::attribute::qtItem*> Items;
};

//----------------------------------------------------------------------------
qtAttribute::qtAttribute(slctk::AttributePtr dataObject, QWidget* p)
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

  QLabel* label = new QLabel(this->getObject()->name().c_str(),
    this->parentWidget());
  layout->addWidget(label);
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
  slctk::AttributePtr att = this->getObject();
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
slctk::AttributePtr qtAttribute::getObject()
{
  return this->Internals->DataObject.lock();
}

//----------------------------------------------------------------------------
QWidget* qtAttribute::parentWidget()
{
  return this->Internals->ParentWidget;
}

//----------------------------------------------------------------------------
qtItem* qtAttribute::createItem(slctk::AttributeItemPtr item, QWidget* pW)
{
  qtItem* aItem = NULL;
  switch (item->type())
    {
    case slctk::attribute::Item::ATTRIBUTE_REF: // This is always inside valueItem ???
      // aItem = qtAttribute::createAttributeRefItem(slctk::dynamicCastPointer<AttributeRefItem>(item), pW);
      break;
    case slctk::attribute::Item::DOUBLE:
    case slctk::attribute::Item::INT:
    case slctk::attribute::Item::STRING:
      aItem = qtAttribute::createValueItem(slctk::dynamicCastPointer<ValueItem>(item), pW);
      break;
    case slctk::attribute::Item::DIRECTORY:
      aItem = qtAttribute::createDirectoryItem(slctk::dynamicCastPointer<DirectoryItem>(item), pW);
      break;
    case slctk::attribute::Item::FILE:
      aItem = qtAttribute::createFileItem(slctk::dynamicCastPointer<FileItem>(item), pW);
      break;
    case slctk::attribute::Item::GROUP:
      aItem = qtAttribute::createGroupItem(slctk::dynamicCastPointer<GroupItem>(item), pW);
      break;
    case slctk::attribute::Item::VOID:
      // Nothing to do!
      break;
    default:
      //this->m_errorStatus << "Error: Unsupported Item Type: " <<
      // slctk::attribute::Item::type2String(item->type()) << "\n";
      break;
    }
  return aItem;
}

//----------------------------------------------------------------------------
qtItem* qtAttribute::createAttributeRefItem(
  slctk::AttributeRefItemPtr item, QWidget* pW)
{
  qtItem* returnItem = new qtAttributeRefItem(dynamicCastPointer<Item>(item), pW);
  return returnItem;
}
//----------------------------------------------------------------------------
qtItem* qtAttribute::createDirectoryItem(
  slctk::DirectoryItemPtr item, QWidget* pW)
{
  qtItem* returnItem = new qtFileItem(dynamicCastPointer<Item>(item), pW, true);
  return returnItem;
}

//----------------------------------------------------------------------------
qtItem* qtAttribute::createFileItem(
  slctk::FileItemPtr item, QWidget* pW)
{
  qtItem* returnItem = new qtFileItem(dynamicCastPointer<Item>(item), pW);
  return returnItem;
}
//----------------------------------------------------------------------------
qtItem* qtAttribute::createGroupItem(slctk::GroupItemPtr item, QWidget* pW)
{
  qtItem* returnItem = new qtGroupItem(dynamicCastPointer<Item>(item), pW);
  return returnItem;
}

//----------------------------------------------------------------------------
qtItem* qtAttribute::createValueItem(
  slctk::ValueItemPtr item, QWidget* pW)
{
  qtItem* returnItem = NULL;
  if (item->allowsExpressions())
    {
    // create the expression item for expression type values
    returnItem = qtAttribute::createAttributeRefItem(
      slctk::dynamicCastPointer<AttributeRefItem>(item), pW);
    }
  else if (!item->isDiscrete())
    {
    // create the input item for editable type values
    returnItem = new qtInputsItem(dynamicCastPointer<Item>(item), pW);
    }
  else if(item->isDiscrete())
    {
    // create the combo item for discrete values
    returnItem = new qtComboItem(dynamicCastPointer<Item>(item), pW);
    }
  return returnItem;
}
