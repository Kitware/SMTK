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
#include "smtk/Qt/qtDiscreteValueItem.h"
#include "smtk/Qt/qtGroupItem.h"
#include "smtk/Qt/qtInputsItem.h"
#include "smtk/Qt/qtFileItem.h"
#include "smtk/Qt/qtAttributeRefItem.h"
#include "smtk/Qt/qtVoidItem.h"
#include "smtk/Qt/qtBaseView.h"

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
  qtAttributeInternals(smtk::attribute::AttributePtr dataObject, QWidget* p,
    qtBaseView* viewW)
  {
  this->ParentWidget = p;
  this->DataObject = dataObject;
  this->View = viewW;
  }
  ~qtAttributeInternals()
  {
  }
 smtk::attribute::WeakAttributePtr DataObject;
 QPointer<QWidget> ParentWidget;
 QList<smtk::attribute::qtItem*> Items;
 QPointer<qtBaseView> View;
};

//----------------------------------------------------------------------------
qtAttribute::qtAttribute(smtk::attribute::AttributePtr dataObject, QWidget* p,
   qtBaseView* view)
{ 
  this->Internals  = new qtAttributeInternals(dataObject, p, view);
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
    qItem = this->createItem(att->item(static_cast<int>(i)), this->Widget,
      this->Internals->View);
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
qtItem* qtAttribute::createItem(smtk::attribute::ItemPtr item, QWidget* pW,
  qtBaseView* bview, Qt::Orientation enVectorItemOrient)
{
  if(!bview->uiManager()->passAdvancedCheck(
      item->definition()->advanceLevel()) ||
    !bview->uiManager()->passItemCategoryCheck(
      item->definition()))
    {
    return NULL;
    }

  qtItem* aItem = NULL;
  smtk::attribute::ValueItemPtr valItemPtr;
  switch (item->type())
    {
    case smtk::attribute::Item::ATTRIBUTE_REF: // This is always inside valueItem ???
      aItem = qtAttribute::createAttributeRefItem(smtk::dynamic_pointer_cast<RefItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::DOUBLE:
    case smtk::attribute::Item::INT:
    case smtk::attribute::Item::STRING:
      valItemPtr = smtk::dynamic_pointer_cast<ValueItem>(item);
      if(valItemPtr)
        {
      /*
        if (valItemPtr->allowsExpressions())
          {
          aItem = qtAttribute::createExpressionRefItem(valItemPtr,elementIdx,pWidget, bview) :
          }
        else
      */
        if(valItemPtr->isDiscrete())
          {
          aItem = qtAttribute::createDiscreteValueItem(valItemPtr,pW, bview, enVectorItemOrient);
          }
        else
          {
          aItem = qtAttribute::createValueItem(valItemPtr, pW, bview, enVectorItemOrient);
          }
        }
      break;
    case smtk::attribute::Item::DIRECTORY:
      aItem = qtAttribute::createDirectoryItem(smtk::dynamic_pointer_cast<DirectoryItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::FILE:
      aItem = qtAttribute::createFileItem(smtk::dynamic_pointer_cast<FileItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::GROUP:
      aItem = qtAttribute::createGroupItem(smtk::dynamic_pointer_cast<GroupItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::VOID:
      aItem = new qtVoidItem(smtk::dynamic_pointer_cast<VoidItem>(item), pW, bview);
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
  smtk::attribute::RefItemPtr item, QWidget* pW, qtBaseView* view,
  Qt::Orientation enVectorItemOrient)
{
  qtItem* returnItem = new qtAttributeRefItem(dynamic_pointer_cast<Item>(item), pW, view, enVectorItemOrient);
  return returnItem;
}
//----------------------------------------------------------------------------
qtItem* qtAttribute::createDirectoryItem(
  smtk::attribute::DirectoryItemPtr item, QWidget* pW, qtBaseView* view,
  Qt::Orientation enVectorItemOrient)
{
  qtFileItem* returnItem = new qtFileItem(dynamic_pointer_cast<Item>(item), pW, view, true, enVectorItemOrient);
  view->uiManager()->onFileItemCreated(returnItem);
  return returnItem;
}
//----------------------------------------------------------------------------
qtItem* qtAttribute::createFileItem(
  smtk::attribute::FileItemPtr item, QWidget* pW, qtBaseView* view, bool dirOnly,
  Qt::Orientation enVectorItemOrient)
{
  qtFileItem* returnItem = new qtFileItem(
    dynamic_pointer_cast<Item>(item), pW, view, dirOnly, enVectorItemOrient);
  view->uiManager()->onFileItemCreated(returnItem);
  return returnItem;
}
//----------------------------------------------------------------------------
qtItem* qtAttribute::createGroupItem(smtk::attribute::GroupItemPtr item,
 QWidget* pW, qtBaseView* view, Qt::Orientation enVectorItemOrient)
{
  qtItem* returnItem = new qtGroupItem(dynamic_pointer_cast<Item>(item), pW, view, enVectorItemOrient);
  return returnItem;
}

//----------------------------------------------------------------------------
qtItem* qtAttribute::createValueItem(
  smtk::attribute::ValueItemPtr item, QWidget* pW, qtBaseView* view,
  Qt::Orientation enVectorItemOrient)
{
    // create the input item for editable type values
  qtItem* returnItem = new qtInputsItem(dynamic_pointer_cast<Item>(item), pW, view, enVectorItemOrient);
  return returnItem;
}

//----------------------------------------------------------------------------
qtItem* qtAttribute::createDiscreteValueItem(
  smtk::attribute::ValueItemPtr item, QWidget* pW, qtBaseView* view,
  Qt::Orientation enVectorItemOrient)
{
    // create the input item for editable type values
  qtItem* returnItem = new qtDiscreteValueItem(dynamic_pointer_cast<Item>(item), pW, view, enVectorItemOrient);
  return returnItem;
}

/*
//----------------------------------------------------------------------------
qtItem* qtAttribute::createExpressionRefItem(
  smtk::attribute::ValueItemPtr item, QWidget* pW, qtBaseView* view,
  Qt::Orientation enVectorItemOrient)
{
    // create the input item for editable type values
  qtItem* returnItem = new qtAttributeRefItem(dynamic_pointer_cast<Item>(item), pW, view, enVectorItemOrient);
  return returnItem;
}
*/