//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtAttribute.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtGroupItem.h"
#include "smtk/extension/qt/qtInputsItem.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtAttributeRefItem.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h"
#include "smtk/extension/qt/qtModelEntityItem.h"
#include "smtk/extension/qt/qtVoidItem.h"
#include "smtk/extension/qt/qtBaseView.h"

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
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
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
  int numShowItems = 0;
  smtk::attribute::AttributePtr att = this->getObject();
  std::size_t i, n = att->numberOfItems();
  if(this->Internals->View)
    {
    for (i = 0; i < n; i++)
      {
      if(this->Internals->View->displayItem(att->item(static_cast<int>(i))))
        {
        numShowItems++;
        }
      }
    }
  else // show everything
    {
    numShowItems = n;
    }
  if(numShowItems == 0)
    {
    return;
    }

  QFrame* attFrame = new QFrame(this->parentWidget());
  attFrame->setFrameShape(QFrame::Box);
  this->Widget = attFrame;

  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(3);
  this->Widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
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
void qtAttribute::showAdvanceLevelOverlay(bool show)
{
  for(int i=0; i < this->Internals->Items.count(); i++)
    {
    this->Internals->Items.value(i)->showAdvanceLevelOverlay(show);
    }
}

//----------------------------------------------------------------------------
void qtAttribute::updateItemsData()
{
  this->clearItems();
  QLayout* layout = this->Widget->layout();
  qtItem* qItem = NULL;
  smtk::attribute::AttributePtr att = this->getObject();
  // If there are model assocication for the attribute, create UI for it.
  // This will be the same widget used for ModelEntityItem.
  if(att->associations())
    {
    qItem = this->createItem(att->associations(), this->Widget,
      this->Internals->View);
    if(qItem && qItem->widget())
      {
      layout->addWidget(qItem->widget());
      this->addItem(qItem);
      }
    }
  // Now go through all child items and create ui components.
  std::size_t i, n = att->numberOfItems();
  for (i = 0; i < n; i++)
    {
    qItem = this->createItem(att->item(static_cast<int>(i)), this->Widget,
      this->Internals->View);
    if(qItem && qItem->widget())
      {
      layout->addWidget(qItem->widget());
      this->addItem(qItem);
      }
    }
}
//----------------------------------------------------------------------------
void qtAttribute::onRequestEntityAssociation()
{
  foreach(qtItem* item, this->Internals->Items)
    {
    if(qtModelEntityItem* mitem = qobject_cast<qtModelEntityItem*>(item))
      mitem->onRequestEntityAssociation();
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
  if(bview && (!bview->displayItem(item)))
    {
    return NULL;
    }

  qtItem* aItem = NULL;
  switch (item->type())
    {
    case smtk::attribute::Item::ATTRIBUTE_REF: // This is always inside valueItem ???
      aItem = qtAttribute::createAttributeRefItem(smtk::dynamic_pointer_cast<RefItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::DOUBLE:
    case smtk::attribute::Item::INT:
    case smtk::attribute::Item::STRING:
      aItem = qtAttribute::createValueItem(smtk::dynamic_pointer_cast<ValueItem>(item), pW, bview, enVectorItemOrient);
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
    case smtk::attribute::Item::MODEL_ENTITY:
      aItem = qtAttribute::createModelEntityItem(smtk::dynamic_pointer_cast<ModelEntityItem>(item), pW, bview, enVectorItemOrient);
      break;
    case smtk::attribute::Item::MESH_SELECTION:
      aItem = qtAttribute::createMeshSelectionItem(smtk::dynamic_pointer_cast<MeshSelectionItem>(item), pW, bview, enVectorItemOrient);
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
  smtk::attribute::FileItemPtr item, QWidget* pW, qtBaseView* view,
  Qt::Orientation enVectorItemOrient)
{
  qtFileItem* returnItem = new qtFileItem(
    dynamic_pointer_cast<Item>(item), pW, view, false, enVectorItemOrient);
  view->uiManager()->onFileItemCreated(returnItem);
  return returnItem;
}
//----------------------------------------------------------------------------
qtItem* qtAttribute::createModelEntityItem(
  smtk::attribute::ModelEntityItemPtr item, QWidget* pW, qtBaseView* view,
  Qt::Orientation enVectorItemOrient)
{
  qtModelEntityItem* returnItem = new qtModelEntityItem(item, pW, view, enVectorItemOrient);
  view->uiManager()->onModelEntityItemCreated(returnItem);
  return returnItem;
}
//----------------------------------------------------------------------------
qtItem* qtAttribute::createMeshSelectionItem(
  smtk::attribute::MeshSelectionItemPtr item, QWidget* pW, qtBaseView* view,
  Qt::Orientation enVectorItemOrient)
{
  qtMeshSelectionItem* returnItem = new qtMeshSelectionItem(item, pW, view, enVectorItemOrient);
  view->uiManager()->onMeshSelectionItemCreated(returnItem);
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
