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

#include "qtAttributeSection.h"

#include "qtUIManager.h"
#include "qtTableWidget.h"
#include "qtAttribute.h"
#include "qtAssociationWidget.h"
#include "qtItem.h"

#include "attribute/AttributeSection.h"
#include "attribute/Attribute.h"
#include "attribute/Definition.h"
#include "attribute/ItemDefinition.h"
#include "attribute/Manager.h"
#include "attribute/ValueItem.h"
#include "attribute/ValueItemDefinition.h"

#include <QGridLayout>
#include <QComboBox>
#include <QTableWidgetItem>
#include <QVariant>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QKeyEvent>
#include <QModelIndex>
#include <QModelIndexList>
#include <QMessageBox>
#include <QSplitter>
#include <QPointer>

#include <set>
using namespace slctk::attribute;

//----------------------------------------------------------------------------
class qtAttributeSectionInternals
{
public:
  QListWidget* ListBox;
  qtTableWidget* AttributeTable;

  QPushButton* AddButton;
  QPushButton* DeleteButton;
  QPushButton* CopyButton;

  QFrame* FiltersFrame;
  QFrame* ButtonsFrame;
  QFrame* TopFrame; // top
  QFrame* BottomFrame; // bottom

  QComboBox* ViewByCombo;
  QComboBox* ShowCategoryCombo;
  QPointer<qtAssociationWidget> AssociationWidget;
};

//----------------------------------------------------------------------------
qtAttributeSection::qtAttributeSection(
  slctk::SectionPtr dataObj, QWidget* p) : qtSection(dataObj, p)
{
  this->Internals = new qtAttributeSectionInternals;
  this->createWidget( );

}

//----------------------------------------------------------------------------
qtAttributeSection::~qtAttributeSection()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtAttributeSection::createWidget( )
{
  if(!this->getObject())
    {
    return;
    }
  // Create a frame to contain all gui components for this object
  // Create a list box for the group entries
  // Create a table widget
  // Add link from the listbox selection to the table widget
  // A common add/delete/(copy/paste ??) widget

  QSplitter* frame = new QSplitter(this->parentWidget());
  //this panel looks better in a over / under layout, rather than left / right
  frame->setOrientation( Qt::Vertical );

  QFrame* TopFrame = new QFrame(frame);
  QFrame* BottomFrame = new QFrame(frame);

  this->Internals->TopFrame = TopFrame;
  this->Internals->BottomFrame = BottomFrame;

  QVBoxLayout* TopLayout = new QVBoxLayout(TopFrame);
  TopLayout->setMargin(0);
  QVBoxLayout* BottomLayout = new QVBoxLayout(BottomFrame);
  BottomLayout->setMargin(0);

  // create a filter-frame with ViewBy-combo
  this->Internals->FiltersFrame = new QFrame(frame);
  QGridLayout* filterLayout = new QGridLayout(this->Internals->FiltersFrame);
  filterLayout->setMargin(0);
  this->Internals->ViewByCombo = new QComboBox(this->Internals->FiltersFrame);
  this->Internals->ViewByCombo->addItem("Attribute");
  this->Internals->ViewByCombo->addItem("Property");
  QLabel* labelViewBy = new QLabel("View By: ", this->Internals->FiltersFrame);
  filterLayout->addWidget(labelViewBy, 0, 0);
  filterLayout->addWidget(this->Internals->ViewByCombo, 0, 1);

  this->Internals->ShowCategoryCombo = new QComboBox(this->Internals->FiltersFrame);

  const Manager* attMan = qtUIManager::instance()->attManager();
  std::set<std::string>::const_iterator it;
  const std::set<std::string> &cats = attMan->categories();
  for (it = cats.begin(); it != cats.end(); it++)
    {
    this->Internals->ShowCategoryCombo->addItem(it->c_str());
    }

  QLabel* labelShow = new QLabel("Show Category: ", this->Internals->FiltersFrame);
  filterLayout->addWidget(labelShow, 1, 0);
  filterLayout->addWidget(this->Internals->ShowCategoryCombo, 1, 1);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  // create a list box for all the entries
  this->Internals->ListBox = new QListWidget(frame);
  this->Internals->ListBox->setSelectionMode(QAbstractItemView::SingleSelection);

  // Buttons frame
  this->Internals->ButtonsFrame = new QFrame(frame);
  QHBoxLayout* buttonLayout = new QHBoxLayout(this->Internals->ButtonsFrame);
  buttonLayout->setMargin(0);
  this->Internals->AddButton = new QPushButton("New", this->Internals->ButtonsFrame);
  this->Internals->AddButton->setSizePolicy(sizeFixedPolicy);
  this->Internals->DeleteButton = new QPushButton("Delete", this->Internals->ButtonsFrame);
  this->Internals->DeleteButton->setSizePolicy(sizeFixedPolicy);
  this->Internals->CopyButton = new QPushButton("Copy", this->Internals->ButtonsFrame);
  this->Internals->CopyButton->setSizePolicy(sizeFixedPolicy);
  buttonLayout->addWidget(this->Internals->AddButton);
  buttonLayout->addWidget(this->Internals->CopyButton);
  buttonLayout->addWidget(this->Internals->DeleteButton);

  // Attribute table
  this->Internals->AttributeTable = new qtTableWidget(frame);
  QSizePolicy tableSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  this->Internals->AttributeTable->setSizePolicy(tableSizePolicy);

  TopLayout->addWidget(this->Internals->FiltersFrame);
  TopLayout->addWidget(this->Internals->ListBox);
  TopLayout->addWidget(this->Internals->ButtonsFrame);
  BottomLayout->addWidget(this->Internals->AttributeTable);

  frame->addWidget(TopFrame);
  frame->addWidget(BottomFrame);
  // the association widget
  this->Internals->AssociationWidget = new qtAssociationWidget(frame);
  BottomLayout->addWidget(this->Internals->AssociationWidget);

  // signals/slots
  QObject::connect(this->Internals->ViewByCombo,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onViewBy(int)));
  QObject::connect(this->Internals->ShowCategoryCombo,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onShowCategory(int)));

  QObject::connect(this->Internals->ListBox,
    SIGNAL(currentItemChanged (QListWidgetItem * , QListWidgetItem * )),
    this, SLOT(onListBoxSelectionChanged(QListWidgetItem * , QListWidgetItem * )));
  QObject::connect(this->Internals->ListBox,
    SIGNAL(itemChanged (QListWidgetItem *)),
    this, SLOT(onAttributeNameChanged(QListWidgetItem * )));

  QObject::connect(this->Internals->AddButton,
    SIGNAL(clicked()), this, SLOT(onCreateNew()));
  QObject::connect(this->Internals->CopyButton,
    SIGNAL(clicked()), this, SLOT(onCopySelected()));
  QObject::connect(this->Internals->DeleteButton,
    SIGNAL(clicked()), this, SLOT(onDeleteSelected()));

  QObject::connect(this->Internals->AttributeTable,
    SIGNAL(itemChanged (QTableWidgetItem *)),
    this, SLOT(onAttributeValueChanged(QTableWidgetItem * )));
  //QObject::connect(this->Internals->AttributeTable,
  //  SIGNAL(keyPressed (QKeyEvent *)),
  //  this, SLOT(onAttributeTableKeyPress(QKeyEvent * )));
  this->Internals->AttributeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->Internals->AttributeTable->setSelectionMode(QAbstractItemView::SingleSelection);

  this->Widget = frame;
  this->parentWidget()->layout()->addWidget(frame);

  this->onViewBy(VIEWBY_Attribute);
  this->Internals->ListBox->setCurrentRow(0);

}

//----------------------------------------------------------------------------
void qtAttributeSection::showAdvanced(int checked)
{

}

//-----------------------------------------------------------------------------
slctk::ValueItemPtr qtAttributeSection::getArrayDataFromItem(QListWidgetItem * item)
{
  return this->getAttributeArrayData(this->getAttributeFromItem(item));
}
//-----------------------------------------------------------------------------
slctk::AttributePtr qtAttributeSection::getAttributeFromItem(
  QListWidgetItem * item)
{
  Attribute* rawPtr = item ? 
    static_cast<Attribute*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->pointer() : slctk::AttributePtr();
}
//-----------------------------------------------------------------------------
slctk::ValueItemPtr qtAttributeSection::getSelectedArrayData()
{
  return this->getAttributeArrayData(this->getSelectedAttribute());
}
//-----------------------------------------------------------------------------
slctk::AttributePtr qtAttributeSection::getSelectedAttribute()
{
  return this->getAttributeFromItem(this->getSelectedItem());
}
//-----------------------------------------------------------------------------
QListWidgetItem *qtAttributeSection::getSelectedItem()
{
  return this->Internals->ListBox->selectedItems().count()>0 ?
    this->Internals->ListBox->selectedItems().value(0) : NULL;
}

//-----------------------------------------------------------------------------
slctk::ValueItemPtr qtAttributeSection::getAttributeArrayData(
  slctk::AttributePtr aAttribute)
{
  return aAttribute ? dynamicCastPointer<ValueItem>(aAttribute->item(0)) :
    slctk::ValueItemPtr();
}

//----------------------------------------------------------------------------
void qtAttributeSection::onListBoxSelectionChanged(
  QListWidgetItem * current, QListWidgetItem * previous)
{
  slctk::AttributePtr dataItem = this->getAttributeFromItem(current);
  this->Internals->AttributeTable->blockSignals(true);
  this->Internals->AttributeTable->clear();
  this->Internals->AttributeTable->setRowCount(0);
  this->Internals->AttributeTable->setColumnCount(0);
  if(dataItem && this->Internals->ViewByCombo->currentIndex() == VIEWBY_Attribute)
    {
    QString strMaterail = this->Internals->ShowCategoryCombo->currentText();
    this->updateTableWithAttribute(dataItem,strMaterail);
    }
  else if(!dataItem && this->Internals->ViewByCombo->currentIndex() == VIEWBY_PROPERTY)
    {
    QString temp = current->text();
    this->updateTableWithProperty(temp);
    }

  this->Internals->AttributeTable->resizeColumnsToContents();
  this->Internals->AttributeTable->resizeRowsToContents();
  this->Internals->AttributeTable->blockSignals(false);
}
//----------------------------------------------------------------------------
void qtAttributeSection::onAttributeNameChanged(QListWidgetItem* item)
{
  slctk::AttributePtr aAttribute = this->getAttributeFromItem(item);
  if(aAttribute)
    {
    Manager *attManager = aAttribute->definition()->manager();
    attManager->rename(aAttribute, item->text().toAscii().constData());
    //aAttribute->definition()->setLabel(item->text().toAscii().constData());

    // Lets see what attributes are being referenced
    std::vector<slctk::AttributeItemPtr> refs;
    std::size_t i;
    aAttribute->references(refs);
    for (i = 0; i < refs.size(); i++)
      {
      std::cout << "\tAtt:" << refs[i]->attribute()->name() << " Item:" << refs[i]->owningItem()->name() 
        << "\n";
      } 
    }
}

//----------------------------------------------------------------------------
void qtAttributeSection::onAttributeValueChanged(QTableWidgetItem* item)
{
  ValueItem* linkedData = item ? 
    static_cast<ValueItem*>(item->data(Qt::UserRole).value<void *>()) : NULL;
    
  if(linkedData && linkedData->isOptional())
    {
    linkedData->setIsEnabled(item->checkState()==Qt::Checked);
    this->updateChildWidgetsEnableState(
      slctk::dynamicCastPointer<ValueItem>(linkedData->pointer()), item);
    }
  slctk::ValueItemPtr dataItem = this->getSelectedArrayData();
  if(!dataItem)
    {
    return;
    }
  //qtUIManager::instance()->updateArrayDataValue(dataItem, item);

}
//----------------------------------------------------------------------------
void qtAttributeSection::updateChildWidgetsEnableState(
  slctk::ValueItemPtr linkedData, QTableWidgetItem* item)
{
  if(!item || !linkedData || !linkedData->isOptional())
    {
    return;
    }
  QTableWidget* tableWidget = this->Internals->AttributeTable;
  bool bEnabled = linkedData->isOptional();
/*  
  if(linkedData->GetGUIObjectType()== vtkSBDataContainer::GENERICGROUP)
    {
    QWidget* cellWidget = tableWidget->cellWidget(item->row(), 0);
    if(cellWidget)
      {
      cellWidget->setEnabled(bEnabled);
      }
    }
  else
    {
    int startRow = item->row();
    for(int row=0; row<linkedData->GetNumberOfDataItems(); row++, startRow++)
      {
      QWidget* cellWidget = tableWidget->cellWidget(startRow, 1);
      if(cellWidget)
        {
        cellWidget->setEnabled(bEnabled);
        }
      }
    }
*/    
}

//----------------------------------------------------------------------------
void qtAttributeSection::onCreateNew()
{
  slctk::AttributeSectionPtr sec =
  slctk::dynamicCastPointer<AttributeSection>(this->getObject());
  if(!sec || !sec->definition(0))
    {
    return;
    }

  this->createNewAttribute(sec->definition(0));
}

//----------------------------------------------------------------------------
void qtAttributeSection::createNewAttribute(
  slctk::AttributeDefinitionPtr attDef)
{
  if(!attDef)
    {
    return;
    }

  Manager *attManager = attDef->manager();

  slctk::AttributePtr newAtt = attManager->createAttribute(attDef->type());
  QListWidgetItem* item = this->addAttributeListItem(newAtt);
  if(item)
    {
    this->Internals->ListBox->setCurrentItem(item);
    }
}
//----------------------------------------------------------------------------
void qtAttributeSection::onAttributeModified()
{
  this->onViewBy(VIEWBY_Attribute);
  int last = this->Internals->ListBox->count()-1;
  if(last >=0)
    {
    this->Internals->ListBox->setCurrentRow(last);
    }
}

//----------------------------------------------------------------------------
void qtAttributeSection::onCopySelected()
{
  slctk::AttributePtr selObject = this->getSelectedAttribute();
  if(selObject)
    {
    this->createNewAttribute(selObject->definition());

    //if(newAttribute)
    //  {
    //  QListWidgetItem* item = this->addAttributeListItem(newAttribute);
    //  if(item)
    //    {
    //    this->Internals->ListBox->setCurrentItem(item);
    //    }
    //  }
    }
}
//----------------------------------------------------------------------------
void qtAttributeSection::onDeleteSelected()
{
  QListWidgetItem* selItem = this->getSelectedItem();
  if(selItem)
    {
    //this->getAttributeContainer()->DestroyAttribute(
    //  this->getAttributeFromItem(selItem));
    slctk::AttributeSectionPtr sec =
      slctk::dynamicCastPointer<AttributeSection>(this->getObject());
    if(!sec || !sec->definition(0))
      {
      return;
      }

    AttributeDefinitionPtr attDef = sec->definition(0);
    Manager *attManager = attDef->manager();
    attManager->removeAttribute(this->getAttributeFromItem(selItem));

    this->Internals->ListBox->takeItem(this->Internals->ListBox->row(selItem));

//    this->Internals->ListBox->takeItem(this->Internals->ListBox->row(selItem));
    }
}
//----------------------------------------------------------------------------
void qtAttributeSection::addAttributePropertyItems(
  slctk::AttributeDefinitionPtr attDef, QString& group)
{
  if(!attDef)
    {
    return;
    }
  AttributeItemDefinitionPtr idef;

  // Now lets process its items
  std::size_t i, n = attDef->numberOfItemDefinitions();
  for (i = 0; i < n; i++)
    {
    idef = attDef->itemDefinition(i);
    if(idef && idef->isMemberOf(group.toStdString()) &&
      qtUIManager::instance()->passItemAdvancedCheck(
      attDef->advanceLevel(), idef->advanceLevel()))
      {
      // No User data, not editable
      QListWidgetItem* item = new QListWidgetItem(
        QString::fromUtf8(idef->name().c_str()),
        this->Internals->ListBox, slctk_USER_DATA_TYPE);
      if(idef->advanceLevel())
        {
        item->setFont(qtUIManager::instance()->instance()->advancedFont());
        }
      this->Internals->ListBox->addItem(item);
      }
    }
}

//----------------------------------------------------------------------------
QListWidgetItem* qtAttributeSection::addAttributeListItem(
  slctk::AttributePtr childData)
{
  QListWidgetItem* item = new QListWidgetItem(
      QString::fromUtf8(childData->name().c_str()),
      this->Internals->ListBox, slctk_USER_DATA_TYPE);
  QVariant vdata;
  vdata.setValue((void*)(childData.get()));
  item->setData(Qt::UserRole, vdata);
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  this->Internals->ListBox->addItem(item);
  return item;
}

//----------------------------------------------------------------------------
void qtAttributeSection::onViewBy(int viewBy)
{
  slctk::AttributeSectionPtr sec =
    slctk::dynamicCastPointer<AttributeSection>(this->getObject());
  if(!sec || !sec->definition(0))
    {
    return;
    }
  this->Internals->ListBox->blockSignals(true);
  this->Internals->ListBox->clear();

  AttributeDefinitionPtr attDef = sec->definition(0);

  std::vector<slctk::AttributePtr> result;
  Manager *attManager = attDef->manager();
  attManager->findAttributes(attDef, result);
  if(result.size())
    {
    if(viewBy == VIEWBY_Attribute)
      {
      this->Internals->ButtonsFrame->setEnabled(true);
      std::vector<slctk::AttributePtr>::iterator it;
      for (it=result.begin(); it!=result.end(); ++it)
        {
        QListWidgetItem* item = this->addAttributeListItem(*it);
        if((*it)->definition()->advanceLevel())
          {
          item->setFont(qtUIManager::instance()->instance()->advancedFont());
          }
        }
      }
    else if(viewBy == VIEWBY_PROPERTY)
      {
      this->Internals->ButtonsFrame->setEnabled(false);
      QString strCategory = this->Internals->ShowCategoryCombo->currentText();
      this->addAttributePropertyItems(attDef,strCategory);
      }
    this->Internals->ListBox->blockSignals(false);
    this->Internals->ListBox->setCurrentRow(0);
    }
  else
    {
    this->Internals->ListBox->blockSignals(false);
    }
}

//----------------------------------------------------------------------------
void qtAttributeSection::onShowCategory(int category)
{
  slctk::AttributeSectionPtr sec =
    slctk::dynamicCastPointer<AttributeSection>(this->getObject());
  if(!sec || !sec->definition(0))
    {
    return;
    }
  AttributeDefinitionPtr attDef = sec->definition(0);

  std::vector<slctk::AttributePtr> result;
  Manager *attManager = attDef->manager();
  attManager->findAttributes(attDef, result);

  if(result.size())
    {
    int viewBy = this->Internals->ViewByCombo->currentIndex();
    if(viewBy == VIEWBY_Attribute)
      {
      this->onListBoxSelectionChanged(
        this->Internals->ListBox->currentItem(), NULL);
      }
    else if(viewBy == VIEWBY_PROPERTY)
      {
      this->Internals->ListBox->blockSignals(true);
      this->Internals->ListBox->clear();
      this->Internals->ButtonsFrame->setEnabled(false);
      QString strCategory = this->Internals->ShowCategoryCombo->currentText();
      this->addAttributePropertyItems(attDef,strCategory);
      this->Internals->ListBox->blockSignals(false);
      this->Internals->ListBox->setCurrentRow(0);
      }
    }
}

//----------------------------------------------------------------------------
void qtAttributeSection::updateTableWithAttribute(
  slctk::AttributePtr att, QString& group)
{
  QTableWidget* widget = this->Internals->AttributeTable;
  widget->setColumnCount(3);

  widget->setHorizontalHeaderItem(0, new QTableWidgetItem("Property"));
  widget->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
  widget->setHorizontalHeaderItem(2, new QTableWidgetItem("Units"));
  QString strValue;
  int numRows = 0;
  Qt::ItemFlags nonEditableFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  std::size_t i, n = att->numberOfItems();
  for (i = 0; i < n; i++)
    {
    slctk::AttributeItemPtr attItem = att->item(i);
    const ItemDefinition* itemDef =
     dynamic_cast<const ItemDefinition*>(attItem->definition().get());
    if(!qtUIManager::instance()->passItemAdvancedCheck(
      att->definition()->advanceLevel(), itemDef->advanceLevel()))
      {
      continue;
      }
    if(itemDef->isMemberOf(group.toStdString()))
      {
      QTableWidgetItem* item = new QTableWidgetItem(attItem->name().c_str());
      if(itemDef->advanceLevel())
        {
        item->setFont(qtUIManager::instance()->instance()->advancedFont());
        }

      item->setFlags(nonEditableFlags);
      widget->setRowCount(++numRows);
      widget->setItem(numRows-1, 0, item);
      bool bEnabled = true;
      if(itemDef->isOptional())
        {
        Qt::CheckState checkState = itemDef->isEnabledByDefault() ?
          Qt::Checked : Qt::Unchecked;
        item->setCheckState(checkState);
        QVariant vdata;
        vdata.setValue((void*)attItem.get());
        item->setData(Qt::UserRole, vdata);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        bEnabled = (checkState==Qt::Checked);
        }
      //if(childData->GetGUIObjectType()== vtkSBDataContainer::GENERICGROUP)
      //  {
      //  SimBuilderItem* childItem = qtUIManager::instance()->instance()->
      //    createContainerItem(childData, NULL);
      //  if(childItem && childItem->widget())
      //    {
      //    this->addChildItem(childItem);
      //    childItem->widget()->setEnabled(bEnabled);
      //    widget->setCellWidget(numRows-1, 0, childItem->widget());
      //    }
      //  }
      //else
      //  {
        this->addTableValueItems(attItem, numRows, bEnabled);
      //  }
      }
    }
}
//----------------------------------------------------------------------------
void qtAttributeSection::updateTableWithProperty(QString& propertyName)
{
  slctk::AttributeSectionPtr sec =
    slctk::dynamicCastPointer<AttributeSection>(this->getObject());
  if(!sec || !sec->definition(0))
    {
    return;
    }

  AttributeDefinitionPtr attDef = sec->definition(0);

  std::vector<slctk::AttributePtr> result;
  Manager *attManager = attDef->manager();
  attManager->findAttributes(attDef, result);

  QTableWidget* widget = this->Internals->AttributeTable;
  widget->setColumnCount(3);

  widget->setHorizontalHeaderItem(0, new QTableWidgetItem("Attribute"));
  widget->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
  widget->setHorizontalHeaderItem(2, new QTableWidgetItem("Units"));

  int numRows = 0;
  Qt::ItemFlags nonEditableFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  std::vector<slctk::AttributePtr>::iterator it;
  for (it=result.begin(); it!=result.end(); ++it)
    {
    std::size_t i, n = (*it)->numberOfItems();
    for (i = 0; i < n; i++)// for each property
      {
      slctk::AttributeItemPtr attItem = (*it)->item(i);
      const ItemDefinition* itemDef =
      dynamic_cast<const ItemDefinition*>(attItem->definition().get());
      if(!qtUIManager::instance()->passItemAdvancedCheck(
        (*it)->definition()->advanceLevel(), itemDef->advanceLevel()))
        {
        continue;
        }    

      if( propertyName == QString(itemDef->name().c_str()))
        {
        widget->setRowCount(++numRows);
        QTableWidgetItem* item = new QTableWidgetItem(attItem->name().c_str());
        if(itemDef->advanceLevel())
          {
          item->setFont(qtUIManager::instance()->instance()->advancedFont());
          }

        item->setFlags(nonEditableFlags);
        widget->setItem(numRows-1, 0, item);
        bool bEnabled = true;
        if(itemDef->isOptional())
          {
          Qt::CheckState checkState = itemDef->isEnabledByDefault() ?
            Qt::Checked : Qt::Unchecked;
          item->setCheckState(checkState);
          QVariant vdata;
          vdata.setValue((void*)attItem.get());
          item->setData(Qt::UserRole, vdata);
          item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
          bEnabled = (checkState==Qt::Checked);
          }
        //if(childData->GetGUIObjectType()== vtkSBDataContainer::GENERICGROUP)
        //  {
        //  SimBuilderItem* childItem = qtUIManager::instance()->instance()->
        //    createContainerItem(childData, NULL);
        //  if(childItem && childItem->widget())
        //    {
        //    this->addChildItem(childItem);
        //    childItem->widget()->setEnabled(bEnabled);
        //    widget->setCellWidget(numRows-1, 1, childItem->widget());
        //    widget->setItem(numRows-1, 1, new QTableWidgetItem());
        //    }
        //  }
        //else
        //  {
          this->addTableValueItems(attItem, numRows, bEnabled);
        //  }
        break;
        }
      }
    }

}

//----------------------------------------------------------------------------
void qtAttributeSection::addTableValueItems(
  slctk::AttributeItemPtr attItem, int& numRows, bool bEnabled)
{
  QTableWidget* widget = this->Internals->AttributeTable;
  const ValueItemDefinition *vItemDef = 
    dynamic_cast<const ValueItemDefinition*>(attItem->definition().get());

  QString unitText = vItemDef && !vItemDef->units().empty() ? vItemDef->units().c_str() : "";
  QTableWidgetItem* unitItem = new QTableWidgetItem(unitText);
  unitItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  widget->setItem(numRows-1, 2, unitItem);

  int numAdded = 0; /// LIKELY NEED MORE WORK

    qtItem* qItem = qtAttribute::createItem(attItem, this->Internals->AttributeTable);
    QWidget* inputWidget = qItem ? qItem->widget() : NULL;
    if(inputWidget)
      {
      numRows = numAdded>0 ? numRows+1 : numRows;
      widget->setRowCount(numRows);
      inputWidget->setEnabled(bEnabled);
      widget->setCellWidget(numRows-1, 1, inputWidget);
      numAdded++;
      }
}

//----------------------------------------------------------------------------
int qtAttributeSection::currentViewBy()
{
  return this->Internals->ViewByCombo->currentIndex();
}
//----------------------------------------------------------------------------
int qtAttributeSection::currentCategory()
{
  return this->Internals->ShowCategoryCombo->currentIndex();
}
//----------------------------------------------------------------------------
void qtAttributeSection::showUI(int viewBy, int category)
{
  this->Internals->ViewByCombo->blockSignals(true);
  this->Internals->ViewByCombo->setCurrentIndex(viewBy);
  this->Internals->ViewByCombo->blockSignals(false);

  this->Internals->ShowCategoryCombo->blockSignals(true);
  this->Internals->ShowCategoryCombo->setCurrentIndex(category);
  this->Internals->ShowCategoryCombo->blockSignals(false);
  this->onShowCategory(category);
}
