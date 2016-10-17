//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtGroupItem.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtAttributeRefItem.h"

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QPointer>
#include <QMap>
#include <QToolButton>
#include <QTableWidget>
#include <QCoreApplication>
#include <QHeaderView>

using namespace smtk::extension;

//----------------------------------------------------------------------------
class qtGroupItemInternals
{
public:
  QPointer<QFrame> ChildrensFrame;
  Qt::Orientation VectorItemOrient;
  QMap<QToolButton*, QList<qtItem* > > ExtensibleMap;
  QList<QToolButton*> MinusButtonIndices;
  QPointer<QToolButton> AddItemButton;
  QPointer<QTableWidget> ItemsTable;
};

//----------------------------------------------------------------------------
qtGroupItem::qtGroupItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p, qtBaseView* bview,
  Qt::Orientation enVectorItemOrient) :
   qtItem(dataObj, p, bview)
{
  this->Internals = new qtGroupItemInternals;
  this->IsLeafItem = true;
  this->Internals->VectorItemOrient = enVectorItemOrient;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtGroupItem::~qtGroupItem()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtGroupItem::setLabelVisible(bool visible)
{
  if(!this->getObject())
    {
    return;
    }
  smtk::attribute::GroupItemPtr item =dynamic_pointer_cast<attribute::GroupItem>(this->getObject());
  if(!item || !item->numberOfGroups())
    {
    return;
    }

  QGroupBox* groupBox = qobject_cast<QGroupBox*>(this->Widget);
  groupBox->setTitle(visible ?
    item->label().c_str() : "");
}

//----------------------------------------------------------------------------
void qtGroupItem::createWidget()
{
  if(!this->getObject())
    {
    return;
    }
  this->clearChildItems();
  smtk::attribute::GroupItemPtr item =dynamic_pointer_cast<attribute::GroupItem>(this->getObject());
  if(!item || (!item->numberOfGroups() && !item->isExtensible()))
    {
    return;
    }

  QString title = item->label().empty() ? item->name().c_str() : item->label().c_str();
  QGroupBox* groupBox = new QGroupBox(title, this->parentWidget());
  this->Widget = groupBox;
  // Instantiate a layout for the widget, but do *not* assign it to a variable.
  // because that would cause a compiler warning, since the layout is not
  // explicitly referenced anywhere in this scope. (There is no memory
  // leak because the layout instance is parented by the widget.)
  new QVBoxLayout(this->Widget);
  this->Widget->layout()->setMargin(0);
  this->Internals->ChildrensFrame = new QFrame(groupBox);
  new QVBoxLayout(this->Internals->ChildrensFrame);

  this->Widget->layout()->addWidget(this->Internals->ChildrensFrame);

  if(this->parentWidget())
    {
    this->parentWidget()->layout()->setAlignment(Qt::AlignTop);
    this->parentWidget()->layout()->addWidget(this->Widget);
    }
  this->updateItemData();

  // If the group is optional, we need a checkbox
  if(item->isOptional())
    {
    groupBox->setCheckable(true);
    groupBox->setChecked(item->isEnabled());
    this->Internals->ChildrensFrame->setVisible(item->isEnabled());
    connect(groupBox, SIGNAL(toggled(bool)),
            this, SLOT(setEnabledState(bool)));
    }
}

//----------------------------------------------------------------------------
void qtGroupItem::setEnabledState(bool checked)
{
  this->Internals->ChildrensFrame->setVisible(checked);
  if(!this->getObject())
    {
    return;
    }

  if(checked != this->getObject()->isEnabled())
    {
    this->getObject()->setIsEnabled(checked);
    emit this->modified();
    this->baseView()->valueChanged(this->getObject());
    }
}

//----------------------------------------------------------------------------
void qtGroupItem::updateItemData()
{
  smtk::attribute::GroupItemPtr item =dynamic_pointer_cast<attribute::GroupItem>(this->getObject());
  if(!item || (!item->numberOfGroups() && !item->isExtensible()))
    {
    return;
    }

  std::size_t i, n = item->numberOfGroups();
  if(item->isExtensible())
    {
    //clear mapping
    QMapIterator<QToolButton*, QList<qtItem* > > mit(this->Internals->ExtensibleMap);
    while(mit.hasNext())
      {
      mit.next();
      QListIterator<qtItem* > lit(mit.value());
      while(lit.hasNext())
        {
        qtItem* tmpItem = lit.next();
        delete tmpItem->widget();
        delete tmpItem;
        }
      delete mit.key();
      }
    this->Internals->ExtensibleMap.clear();
    this->Internals->MinusButtonIndices.clear();
    if(this->Internals->ItemsTable)
      {
      this->Internals->ItemsTable->blockSignals(true);
      this->Internals->ItemsTable->clear();
      this->Internals->ItemsTable->setRowCount(0);
      this->Internals->ItemsTable->setColumnCount(0);
      this->Internals->ItemsTable->blockSignals(false);
      }

    // The new item button
    if(!this->Internals->AddItemButton)
      {
      this->Internals->AddItemButton = new QToolButton(this->Internals->ChildrensFrame);
      this->Internals->AddItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
      QString iconName(":/icons/attribute/plus.png");
      this->Internals->AddItemButton->setText("Add Sub Group");
      this->Internals->AddItemButton->setIcon(QIcon(iconName));
      this->Internals->AddItemButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      connect(this->Internals->AddItemButton, SIGNAL(clicked()),
        this, SLOT(onAddSubGroup()));
      this->Internals->ChildrensFrame->layout()->addWidget(
        this->Internals->AddItemButton);
      }
    this->Widget->layout()->setSpacing(3);
    }

  for(i = 0; i < n; i++)
    {
    int subIdx = static_cast<int>(i);
    if(item->isExtensible())
      {
      this->addItemsToTable(subIdx);
      }
    else
      {
      this->addSubGroup(subIdx);
      }
    }
  this->qtItem::updateItemData();
}

//----------------------------------------------------------------------------
void qtGroupItem::onAddSubGroup()
{
  smtk::attribute::GroupItemPtr item =
    dynamic_pointer_cast<attribute::GroupItem>(this->getObject());
  if(!item || (!item->numberOfGroups() && !item->isExtensible()))
    {
    return;
    }
  if(item->appendGroup())
    {
    int subIdx = static_cast<int>(item->numberOfGroups()) - 1;
    if(item->isExtensible())
      {
      this->addItemsToTable(subIdx);
      }
    else
      {
      this->addSubGroup(subIdx);
      }
    emit this->widgetSizeChanged();
    emit this->modified();
    }
}

//----------------------------------------------------------------------------
void qtGroupItem::addSubGroup(int i)
{
  smtk::attribute::GroupItemPtr item =
    dynamic_pointer_cast<attribute::GroupItem>(this->getObject());
  if(!item || (!item->numberOfGroups() && !item->isExtensible()))
    {
    return;
    }

  std::size_t j, m = item->numberOfItemsPerGroup();
  QBoxLayout* frameLayout = qobject_cast<QBoxLayout*>(
    this->Internals->ChildrensFrame->layout());
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QBoxLayout* subGrouplayout = new QVBoxLayout();
  subGrouplayout->setMargin(0);
  subGrouplayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  QList<qtItem*> itemList;

  QList<smtk::attribute::ItemDefinitionPtr> childDefs;
  for (j = 0; j < m; j++)
    {
    smtk::attribute::ConstItemDefinitionPtr itDef = item->item(i,
      static_cast<int>(j))->definition();
    //smtk::attribute::ItemDefinitionPtr childDef(
    //  smtk::const_pointer_cast<ItemDefinition>(itDef));
    childDefs.push_back(smtk::const_pointer_cast<attribute::ItemDefinition>(itDef));
    }
  int tmpLen = this->baseView()->uiManager()->getWidthOfItemsMaxLabel(
    childDefs, this->baseView()->uiManager()->advancedFont());
  int currentLen = this->baseView()->fixedLabelWidth();
  this->baseView()->setFixedLabelWidth(tmpLen);

  for (j = 0; j < m; j++)
    {
    qtItem* childItem = qtAttribute::createItem(item->item(i,
      static_cast<int>(j)), this->Widget, this->baseView(), this->Internals->VectorItemOrient);
    if(childItem)
      {
      subGrouplayout->addWidget(childItem->widget());
      itemList.push_back(childItem);
      connect(childItem, SIGNAL(modified()), this, SLOT(onChildItemModified()));
      }
    }
  this->baseView()->setFixedLabelWidth(currentLen);
  frameLayout->addLayout(subGrouplayout);
  this->onChildWidgetSizeChanged();
}
//----------------------------------------------------------------------------
void qtGroupItem::onRemoveSubGroup()
{
  QToolButton* const minusButton = qobject_cast<QToolButton*>(
    QObject::sender());
  if(!minusButton || !this->Internals->ExtensibleMap.contains(minusButton))
    {
    return;
    }

  int gIdx = this->Internals->MinusButtonIndices.indexOf(minusButton);//minusButton->property("SubgroupIndex").toInt();
  smtk::attribute::GroupItemPtr item =
    dynamic_pointer_cast<attribute::GroupItem>(this->getObject());
  if(!item || gIdx < 0 || gIdx >= static_cast<int>(item->numberOfGroups()))
    {
    return;
    }

  foreach(qtItem* qi, this->Internals->ExtensibleMap.value(minusButton))
    {
    delete qi->widget();
    delete qi;
    }
//  delete this->Internals->ExtensibleMap.value(minusButton).first;
  this->Internals->ExtensibleMap.remove(minusButton);

  item->removeGroup(gIdx);
  int rowIdx = -1, rmIdx = -1;
  // normally rowIdx is same as gIdx, but we need to find
  // explicitly since minusButton could be NULL in MinusButtonIndices
  foreach(QToolButton* tb, this->Internals->MinusButtonIndices)
    {
    rowIdx = tb != NULL ? rowIdx + 1 : rowIdx;
    if(tb == minusButton)
      {
      rmIdx = rowIdx;
      break;
      }
    }
  if(rmIdx >=0 && rmIdx < this->Internals->ItemsTable->rowCount())
    {
    this->Internals->ItemsTable->removeRow(rmIdx);
    }
  this->Internals->MinusButtonIndices.removeOne(minusButton);
  delete minusButton;
  this->updateExtensibleState();
  emit this->modified();
}

//----------------------------------------------------------------------------
void qtGroupItem::updateExtensibleState()
{
  smtk::attribute::GroupItemPtr item =
    dynamic_pointer_cast<attribute::GroupItem>(this->getObject());
  if(!item || !item->isExtensible())
    {
    return;
    }
  bool maxReached = (item->maxNumberOfGroups() > 0) &&
    (item->maxNumberOfGroups() == item->numberOfGroups());
  this->Internals->AddItemButton->setEnabled(!maxReached);

  bool minReached = (item->numberOfRequiredGroups() > 0) &&
    (item->numberOfRequiredGroups() == item->numberOfGroups());
  foreach(QToolButton* tButton, this->Internals->ExtensibleMap.keys())
    {
    tButton->setEnabled(!minReached);
    }
}

//----------------------------------------------------------------------------
void qtGroupItem::addItemsToTable(int i)
{
  smtk::attribute::GroupItemPtr item =
    dynamic_pointer_cast<attribute::GroupItem>(this->getObject());
  if(!item || !item->isExtensible())
    {
    return;
    }

  std::size_t j, m = item->numberOfItemsPerGroup();
  QBoxLayout* frameLayout = qobject_cast<QBoxLayout*>(
    this->Internals->ChildrensFrame->layout());
  if(!this->Internals->ItemsTable)
    {
    this->Internals->ItemsTable = new QTableWidget(this->Internals->ChildrensFrame);
    this->Internals->ItemsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //this->Internals->ItemsTable->horizontalHeader()->setStretchLastSection(true);
    //this->Internals->ItemsTable->setFixedHeight(120);
    this->Internals->ItemsTable->setColumnCount(1); // for minus button
    frameLayout->addWidget(this->Internals->ItemsTable);
    }

  this->Internals->ItemsTable->blockSignals(true);
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QList<qtItem*> itemList;
  int added = 0;
  int numRows = this->Internals->ItemsTable->rowCount();
  for (j = 0; j < m; j++)
    {
    smtk::attribute::ItemPtr attItem = item->item(i, static_cast<int>(j));
    qtItem* childItem = qtAttribute::createItem(attItem, this->Widget,
      this->baseView(), Qt::Horizontal);
    if(childItem)
      {
      if(added == 0)
        {
        this->Internals->ItemsTable->insertRow(numRows);
        }
      int numCols = this->Internals->ItemsTable->columnCount() - 1;
      if(added >= numCols)
        {
        this->Internals->ItemsTable->insertColumn(numCols + 1);
        std::string strItemLabel = attItem->label().empty() ? attItem->name() : attItem->label();
        this->Internals->ItemsTable->setHorizontalHeaderItem(numCols + 1,
          new QTableWidgetItem(strItemLabel.c_str()));
        }
      childItem->setLabelVisible(false);
      qtAttributeRefItem* arItem = qobject_cast<qtAttributeRefItem*>(childItem);
      if(arItem)
        {
        arItem->setAttributeWidgetVisible(false);
        }
      this->Internals->ItemsTable->setCellWidget(numRows, added+1, childItem->widget());
      this->Internals->ItemsTable->setItem(numRows, added+1, new QTableWidgetItem());
      itemList.push_back(childItem);
      connect(childItem, SIGNAL(widgetSizeChanged()),
        this, SLOT(onChildWidgetSizeChanged()));
      added++;
      connect(childItem, SIGNAL(modified()), this, SLOT(onChildItemModified()));
      }
    }
  QToolButton* minusButton = NULL;
  // if there are items
  if(added > 0)
    {
    minusButton = new QToolButton(this->Internals->ChildrensFrame);
    QString iconName(":/icons/attribute/minus.png");
    minusButton->setFixedSize(QSize(16, 16));
    minusButton->setIcon(QIcon(iconName));
    minusButton->setSizePolicy(sizeFixedPolicy);
    minusButton->setToolTip("Remove sub group");
    //QVariant vdata(static_cast<int>(i));
    //minusButton->setProperty("SubgroupIndex", vdata);
    connect(minusButton, SIGNAL(clicked()),
      this, SLOT(onRemoveSubGroup()));
    this->Internals->ItemsTable->setCellWidget(numRows, 0, minusButton);

    this->Internals->ExtensibleMap[minusButton] = itemList;
    }
  this->Internals->MinusButtonIndices.push_back(minusButton);
  this->updateExtensibleState();

  this->Internals->ItemsTable->blockSignals(false);
  this->onChildWidgetSizeChanged();
}

//----------------------------------------------------------------------------
void qtGroupItem::onChildWidgetSizeChanged()
{
  if(this->Internals->ItemsTable)
    {
    this->Internals->ItemsTable->resizeColumnsToContents();
    this->Internals->ItemsTable->resizeRowsToContents();
    }
}

/* Slot for properly emitting signals when an attribute's item is modified */
void qtGroupItem::onChildItemModified()
{
  emit this->modified();
}
