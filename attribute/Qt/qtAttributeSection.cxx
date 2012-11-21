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
#include <QMap>

#include <set>
using namespace slctk::attribute;

//----------------------------------------------------------------------------
class qtAttributeSectionInternals
{
public:
  qtTableWidget* ListTable;
  qtTableWidget* ValuesTable;

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

  // <category, AttDefinitions>
  QMap<QString, QList<slctk::AttributeDefinitionPtr> > AttDefMap;

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
  this->Internals->AttDefMap.clear();
  for (it = cats.begin(); it != cats.end(); it++)
    {
    this->Internals->ShowCategoryCombo->addItem(it->c_str());
    QList<slctk::AttributeDefinitionPtr> attdeflist;
    this->Internals->AttDefMap[it->c_str()] = attdeflist;
    }

  QLabel* labelShow = new QLabel("Show Category: ", this->Internals->FiltersFrame);
  filterLayout->addWidget(labelShow, 1, 0);
  filterLayout->addWidget(this->Internals->ShowCategoryCombo, 1, 1);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QSizePolicy tableSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  // create a list box for all the entries
  this->Internals->ListTable = new qtTableWidget(frame);
  this->Internals->ListTable->setSelectionMode(QAbstractItemView::SingleSelection);
  this->Internals->ListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->Internals->ListTable->setSizePolicy(tableSizePolicy);

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
  this->Internals->ValuesTable = new qtTableWidget(frame);
  this->Internals->ValuesTable->setSizePolicy(tableSizePolicy);

  TopLayout->addWidget(this->Internals->FiltersFrame);
  TopLayout->addWidget(this->Internals->ListTable);
  TopLayout->addWidget(this->Internals->ButtonsFrame);
  BottomLayout->addWidget(this->Internals->ValuesTable);

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

  QObject::connect(this->Internals->ListTable,
    SIGNAL(itemSelectionChanged ()),
    this, SLOT(onListBoxSelectionChanged()));
  QObject::connect(this->Internals->ListTable,
    SIGNAL(itemChanged (QTableWidgetItem *)),
    this, SLOT(onAttributeNameChanged(QTableWidgetItem * )));

  QObject::connect(this->Internals->AddButton,
    SIGNAL(clicked()), this, SLOT(onCreateNew()));
  QObject::connect(this->Internals->CopyButton,
    SIGNAL(clicked()), this, SLOT(onCopySelected()));
  QObject::connect(this->Internals->DeleteButton,
    SIGNAL(clicked()), this, SLOT(onDeleteSelected()));

  QObject::connect(this->Internals->ValuesTable,
    SIGNAL(itemChanged (QTableWidgetItem *)),
    this, SLOT(onAttributeValueChanged(QTableWidgetItem * )));
  //QObject::connect(this->Internals->ValuesTable,
  //  SIGNAL(keyPressed (QKeyEvent *)),
  //  this, SLOT(onAttributeTableKeyPress(QKeyEvent * )));
  this->Internals->ValuesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->Internals->ValuesTable->setSelectionMode(QAbstractItemView::SingleSelection);

  this->Widget = frame;
  this->parentWidget()->layout()->addWidget(frame);

  this->getAllDefinitions();

  this->onViewBy(VIEWBY_Attribute);
}

//----------------------------------------------------------------------------
void qtAttributeSection::showAdvanced(int checked)
{

}

//-----------------------------------------------------------------------------
slctk::ValueItemPtr qtAttributeSection::getArrayDataFromItem(QTableWidgetItem * item)
{
  return this->getAttributeArrayData(this->getAttributeFromItem(item));
}
//-----------------------------------------------------------------------------
slctk::AttributePtr qtAttributeSection::getAttributeFromItem(
  QTableWidgetItem * item)
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
QTableWidgetItem *qtAttributeSection::getSelectedItem()
{
  return this->Internals->ListTable->selectedItems().count()>0 ?
    this->Internals->ListTable->selectedItems().value(0) : NULL;
}

//-----------------------------------------------------------------------------
slctk::ValueItemPtr qtAttributeSection::getAttributeArrayData(
  slctk::AttributePtr aAttribute)
{
  return aAttribute ? dynamicCastPointer<ValueItem>(aAttribute->item(0)) :
    slctk::ValueItemPtr();
}

//----------------------------------------------------------------------------
void qtAttributeSection::onListBoxSelectionChanged()
{
  this->Internals->ValuesTable->blockSignals(true);
  this->Internals->ValuesTable->clear();
  QTableWidgetItem* current = this->getSelectedItem();

  if(current)
    {
    slctk::AttributePtr dataItem = this->getAttributeFromItem(current);
    this->Internals->ValuesTable->setRowCount(0);
    this->Internals->ValuesTable->setColumnCount(0);
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
    }

  this->Internals->ValuesTable->resizeColumnsToContents();
  this->Internals->ValuesTable->resizeRowsToContents();
  this->Internals->ValuesTable->blockSignals(false);
}
//----------------------------------------------------------------------------
void qtAttributeSection::onAttributeNameChanged(QTableWidgetItem* item)
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
  QTableWidget* tableWidget = this->Internals->ValuesTable;
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
  if(slctk::AttributePtr selObject = this->getSelectedAttribute())
    {
    this->createNewAttribute(selObject->definition());
    }
  else
    {
    slctk::AttributeSectionPtr sec =
      slctk::dynamicCastPointer<AttributeSection>(this->getObject());
    if(!sec || !sec->numberOfDefinitions())
      {
      return;
      }

    this->createNewAttribute(sec->definition(0));
    }
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
  QTableWidgetItem* item = this->addAttributeListItem(newAtt);
  if(item)
    {
    this->Internals->ListTable->selectRow(item->row());
    }
}
//----------------------------------------------------------------------------
void qtAttributeSection::onAttributeModified()
{
  this->onViewBy(VIEWBY_Attribute);
  int last = this->Internals->ListTable->rowCount()-1;
  if(last >=0)
    {
    this->Internals->ListTable->selectRow(last);
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
    //  QTableWidgetItem* item = this->addAttributeListItem(newAttribute);
    //  if(item)
    //    {
    //    this->Internals->ListTable->setCurrentItem(item);
    //    }
    //  }
    }
}
//----------------------------------------------------------------------------
void qtAttributeSection::onDeleteSelected()
{
  slctk::AttributePtr selObject = this->getSelectedAttribute();
  if(selObject)
    {

    AttributeDefinitionPtr attDef = selObject->definition();
    Manager *attManager = attDef->manager();
    attManager->removeAttribute(selObject);

    QTableWidgetItem* selItem = this->getSelectedItem();
    this->Internals->ListTable->removeRow(selItem->row());
    }
}
//----------------------------------------------------------------------------
void qtAttributeSection::addAttributePropertyItems(
  slctk::AttributePtr childData, const QString& group)
{
  if(!childData)
    {
    return;
    }

  bool multiDef = this->hasMultiDefinition(group);

  Qt::ItemFlags nonEditableFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  // Now lets process its items
  std::size_t i, n = childData->numberOfItems();
  for (i = 0; i < n; i++)
    {
    slctk::AttributeItemPtr attItem = childData->item(i);
    if(attItem->definition()->isMemberOf(group.toStdString()) &&
      qtUIManager::instance()->passItemAdvancedCheck(
      childData->definition()->advanceLevel(),
      attItem->definition()->advanceLevel()))
      {
      // No User data, not editable
      QTableWidgetItem* item = new QTableWidgetItem(
        QString::fromUtf8(attItem->name().c_str()),
        slctk_USER_DATA_TYPE);

      QVariant vdata;
      vdata.setValue((void*)(attItem.get()));
      item->setData(Qt::UserRole, vdata);
      item->setFlags(nonEditableFlags);

      int numRows = this->Internals->ListTable->rowCount();
      this->Internals->ListTable->setRowCount(++numRows);
      this->Internals->ListTable->setItem(numRows-1, 0, item);
      if(multiDef)
        {
        // add the type column too.
        QTableWidgetItem* defitem = new QTableWidgetItem(
          QString::fromUtf8(childData->definition()->type().c_str()),
          slctk_USER_DATA_TYPE);
        defitem->setFlags(nonEditableFlags);
        this->Internals->ListTable->setItem(numRows-1, 1, defitem);
        }

      if(attItem->definition()->advanceLevel())
        {
        item->setFont(qtUIManager::instance()->instance()->advancedFont());
        }
      }
    }
}

//----------------------------------------------------------------------------
QTableWidgetItem* qtAttributeSection::addAttributeListItem(
  slctk::AttributePtr childData)
{
  QString strCategory = this->Internals->ShowCategoryCombo->currentText();

  QTableWidgetItem* item = new QTableWidgetItem(
      QString::fromUtf8(childData->name().c_str()),
      slctk_USER_DATA_TYPE);
  Qt::ItemFlags nonEditableFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  QVariant vdata;
  vdata.setValue((void*)(childData.get()));
  item->setData(Qt::UserRole, vdata);
  item->setFlags(nonEditableFlags | Qt::ItemIsEditable);

  int numRows = this->Internals->ListTable->rowCount();
  this->Internals->ListTable->setRowCount(++numRows);
  this->Internals->ListTable->setItem(numRows-1, 0, item);
  bool multiDef = this->hasMultiDefinition(strCategory);
  if(multiDef)
    {
    // add the type column too.
    QTableWidgetItem* defitem = new QTableWidgetItem(
      QString::fromUtf8(childData->definition()->type().c_str()),
      slctk_USER_DATA_TYPE);
    defitem->setFlags(nonEditableFlags);
    this->Internals->ListTable->setItem(numRows-1, 1, defitem);
   }
  return item;
}

//----------------------------------------------------------------------------
void qtAttributeSection::onViewBy(int viewBy)
{
  slctk::AttributeSectionPtr sec =
    slctk::dynamicCastPointer<AttributeSection>(this->getObject());
  if(!sec || !sec->numberOfDefinitions())
    {
    return;
    }

  bool viewAtt = (viewBy == VIEWBY_Attribute);
  this->Internals->ButtonsFrame->setEnabled(viewAtt);
  this->Internals->ListTable->blockSignals(true);
  this->Internals->ListTable->clear();
  this->Internals->ListTable->setRowCount(0);

  QString strCategory = this->Internals->ShowCategoryCombo->currentText();
  bool multiDef = this->hasMultiDefinition(strCategory);
  this->Internals->ListTable->setColumnCount(multiDef ? 2 : 1);
  
  this->Internals->ListTable->setHorizontalHeaderItem(
    0, new QTableWidgetItem(viewAtt ? "Attribute" : "Property"));
  if(multiDef)
    {
    this->Internals->ListTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Type"));
    }
  
  foreach (AttributeDefinitionPtr attDef,
    this->Internals->AttDefMap[strCategory])
    {
    this->onViewByWithDefinition(viewBy, attDef);
    }
  this->Internals->ListTable->blockSignals(false);
  if(this->Internals->ListTable->rowCount())
    {
    this->Internals->ListTable->selectRow(0);
    }
}
//----------------------------------------------------------------------------
void qtAttributeSection::onViewByWithDefinition(
  int viewBy, slctk::AttributeDefinitionPtr attDef)
{
  std::vector<slctk::AttributePtr> result;
  Manager *attManager = attDef->manager();
  attManager->findAttributes(attDef, result);
  if(result.size())
    {
    QString strCategory = this->Internals->ShowCategoryCombo->currentText();
      //this->Internals->ButtonsFrame->setEnabled(true);
    std::vector<slctk::AttributePtr>::iterator it;
    for (it=result.begin(); it!=result.end(); ++it)
      {
      if(viewBy == VIEWBY_Attribute)
        {
        QTableWidgetItem* item = this->addAttributeListItem(*it);
        if((*it)->definition()->advanceLevel())
          {
          item->setFont(qtUIManager::instance()->instance()->advancedFont());
          }
        }
      else if(viewBy == VIEWBY_PROPERTY)
        {
        //this->Internals->ButtonsFrame->setEnabled(false);
        this->addAttributePropertyItems(*it,strCategory);
        }
      }
    }
}
//----------------------------------------------------------------------------
void qtAttributeSection::onShowCategory(int category)
{
  int viewBy = this->Internals->ViewByCombo->currentIndex();
  this->onViewBy(viewBy);
}

//----------------------------------------------------------------------------
void qtAttributeSection::updateTableWithAttribute(
  slctk::AttributePtr att, const QString& group)
{
  QTableWidget* widget = this->Internals->ValuesTable;
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
  if(!sec || !sec->numberOfDefinitions())
    {
    return;
    }

  AttributeDefinitionPtr attDef = sec->definition(0);

  std::vector<slctk::AttributePtr> result;
  Manager *attManager = attDef->manager();
  attManager->findAttributes(attDef, result);

  QTableWidget* widget = this->Internals->ValuesTable;
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
  QTableWidget* widget = this->Internals->ValuesTable;
  const ValueItemDefinition *vItemDef = 
    dynamic_cast<const ValueItemDefinition*>(attItem->definition().get());

  QString unitText = vItemDef && !vItemDef->units().empty() ? vItemDef->units().c_str() : "";
  QTableWidgetItem* unitItem = new QTableWidgetItem(unitText);
  unitItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  widget->setItem(numRows-1, 2, unitItem);

  int numAdded = 0; /// LIKELY NEED MORE WORK

    qtItem* qItem = qtAttribute::createItem(attItem, this->Internals->ValuesTable);
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
//----------------------------------------------------------------------------
void qtAttributeSection::getAllDefinitions()
{
  std::vector<slctk::AttributeDefinitionPtr> defs;
  slctk::AttributeSectionPtr sec =
    slctk::dynamicCastPointer<AttributeSection>(this->getObject());
  if(!sec || !sec->numberOfDefinitions())
    {
    return;
    }
  std::size_t i, n = sec->numberOfDefinitions();
  for (i = 0; i < n; i++)
    {
    AttributeDefinitionPtr attDef = sec->definition(i);
    this->qtSection::getDefinitions(attDef, defs);
    }

  std::vector<slctk::AttributeDefinitionPtr>::iterator itDef;
  for (itDef=defs.begin(); itDef!=defs.end(); ++itDef)
    {
    foreach(QString category, this->Internals->AttDefMap.keys())
      {
      if((*itDef)->isMemberOf(category.toStdString()))
        {
        this->Internals->AttDefMap[category].push_back(*itDef);
        }
      }
    }
}
//----------------------------------------------------------------------------
bool qtAttributeSection::hasMultiDefinition(const QString& group)
{
  return (this->Internals->AttDefMap[group].count() > 1);
}
