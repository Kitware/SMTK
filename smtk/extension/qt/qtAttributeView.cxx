//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAttributeView.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtAttributeRefItem.h"
#include "smtk/extension/qt/qtAssociationWidget.h"
#include "smtk/extension/qt/qtCheckItemComboBox.h"
#include "smtk/extension/qt/qtReferencesWidget.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtVoidItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/View.h"

#include <QGridLayout>
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
#include <QBrush>
#include <QColorDialog>
#include <QHeaderView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>

#include <iostream>
#include <set>
using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtAttributeViewInternals
{
public:

  const QList<smtk::attribute::DefinitionPtr> getCurrentDefs(
    const QString strCategory) const
  {

    if(this->AttDefMap.keys().contains(strCategory))
      {
      return this->AttDefMap[strCategory];
      }
    return this->AllDefs;
  }
  qtTableWidget* ListTable;
  qtTableWidget* ValuesTable;

  QPushButton* AddButton;
  QComboBox* DefsCombo;
  QPushButton* DeleteButton;
  QPushButton* CopyButton;

  QFrame* FiltersFrame;
  QFrame* ButtonsFrame;
  QFrame* TopFrame; // top
  QFrame* BottomFrame; // bottom

  QComboBox* ViewByCombo;
  QComboBox* PropDefsCombo;

  QPointer<qtAssociationWidget> AssociationsWidget;
  QPointer<qtReferencesWidget> ReferencesWidget;

  // <category, AttDefinitions>
  QMap<QString, QList<smtk::attribute::DefinitionPtr> > AttDefMap;

  // All definitions list
  QList<smtk::attribute::DefinitionPtr> AllDefs;

  // Attribute widget
  QPointer<qtAttribute> CurrentAtt;
  QPointer<QFrame> AttFrame;

  // Model for filtering the attribute by combobox.
  QPointer<QStandardItemModel> checkableAttComboModel;
  QMap<std::string, Qt::CheckState> AttSelections;
  qtCheckItemComboBox* SelectAttCombo;

  // Model for filtering the attribute properties by combobox.
  QPointer<QStandardItemModel> checkablePropComboModel;
  qtCheckItemComboBox* SelectPropCombo;
  QMap<std::string, Qt::CheckState> AttProperties;
  std::vector<smtk::attribute::DefinitionPtr> m_attDefinitions;
  bool m_okToCreateModelEntities;
  smtk::model::BitFlags m_modelEntityMask;

};

//----------------------------------------------------------------------------
qtBaseView *
qtAttributeView::createViewWidget(smtk::common::ViewPtr dataObj,
                                  QWidget* p, qtUIManager* uiman)
{
  qtAttributeView *view = new qtAttributeView(dataObj, p, uiman);
  view->buildUI();
  return view;
}

//----------------------------------------------------------------------------
qtAttributeView::
qtAttributeView(smtk::common::ViewPtr dataObj, QWidget* p, qtUIManager* uiman) :
  qtBaseView(dataObj, p, uiman)
{
  this->Internals = new qtAttributeViewInternals;
}

//----------------------------------------------------------------------------
qtAttributeView::~qtAttributeView()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
const QMap<QString, QList<smtk::attribute::DefinitionPtr> > &qtAttributeView::
 attDefinitionMap() const
 {
   return this->Internals->AttDefMap;
 }

//----------------------------------------------------------------------------
void qtAttributeView::createWidget( )
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
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QVBoxLayout* TopLayout = new QVBoxLayout(TopFrame);
  TopLayout->setMargin(0);
  TopFrame->setSizePolicy(sizeFixedPolicy);
  QVBoxLayout* BottomLayout = new QVBoxLayout(BottomFrame);
  BottomLayout->setMargin(0);
  BottomFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // create a filter-frame with ViewBy-combo
  this->Internals->FiltersFrame = new QFrame(frame);
  QGridLayout* filterLayout = new QGridLayout(this->Internals->FiltersFrame);
  filterLayout->setMargin(0);
  this->Internals->FiltersFrame->setSizePolicy(sizeFixedPolicy);

  this->Internals->ViewByCombo = new QComboBox(this->Internals->FiltersFrame);
  this->Internals->ViewByCombo->addItem("Attribute");
  this->Internals->ViewByCombo->addItem("Property");
  QLabel* labelViewBy = new QLabel("View By: ", this->Internals->FiltersFrame);
  filterLayout->addWidget(labelViewBy, 0, 0);
  filterLayout->addWidget(this->Internals->ViewByCombo, 0, 1);

  this->Internals->PropDefsCombo = new QComboBox(TopFrame);
  this->Internals->PropDefsCombo->setVisible(false);
  this->Internals->PropDefsCombo->setToolTip("Select definition to filter attributes and properties");
  filterLayout->addWidget(this->Internals->PropDefsCombo, 0, 2);
  QObject::connect(this->Internals->PropDefsCombo,  SIGNAL(currentIndexChanged(int)),
    this, SLOT(onPropertyDefSelected()), Qt::QueuedConnection);

  this->Internals->SelectPropCombo = new qtCheckItemComboBox(TopFrame, "Properties");
  this->Internals->SelectPropCombo->setVisible(false);
  this->Internals->SelectPropCombo->setToolTip("Select properties to compare");
  this->Internals->checkablePropComboModel = new QStandardItemModel();
  this->Internals->SelectPropCombo->setModel(
    this->Internals->checkablePropComboModel);
  this->Internals->SelectPropCombo->setItemDelegate(
    new qtCheckableComboItemDelegate(this->Internals->SelectPropCombo));
  filterLayout->addWidget(this->Internals->SelectPropCombo, 0, 3);

  this->Internals->SelectAttCombo = new qtCheckItemComboBox(TopFrame, "Attributes");
  this->Internals->SelectAttCombo->setVisible(false);
  this->Internals->SelectPropCombo->setToolTip("Select attributes to compare");
  this->Internals->checkableAttComboModel = new QStandardItemModel();
  this->Internals->SelectAttCombo->setModel(
    this->Internals->checkableAttComboModel);
  this->Internals->SelectAttCombo->setItemDelegate(
    new qtCheckableComboItemDelegate(this->Internals->SelectAttCombo));
  filterLayout->addWidget(this->Internals->SelectAttCombo, 0, 4);

  const System* attSys = this->uiManager()->attSystem();
  std::set<std::string>::const_iterator it;
  const std::set<std::string> &cats = attSys->categories();
  this->Internals->AttDefMap.clear();
  for (it = cats.begin(); it != cats.end(); it++)
    {
    QList<smtk::attribute::DefinitionPtr> attdeflist;
    this->Internals->AttDefMap[it->c_str()] = attdeflist;
    }

  QSizePolicy tableSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // create a list box for all the entries
  this->Internals->ListTable = new qtTableWidget(frame);
  this->Internals->ListTable->setSelectionMode(QAbstractItemView::SingleSelection);
  this->Internals->ListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->Internals->ListTable->setSizePolicy(tableSizePolicy);

  // Buttons frame
  this->Internals->ButtonsFrame = new QFrame(frame);
  QHBoxLayout* buttonLayout = new QHBoxLayout(this->Internals->ButtonsFrame);
  buttonLayout->setMargin(0);
  this->Internals->ButtonsFrame->setSizePolicy(sizeFixedPolicy);

  this->Internals->AddButton = new QPushButton("New", this->Internals->ButtonsFrame);
  this->Internals->AddButton->setSizePolicy(sizeFixedPolicy);
  this->Internals->DeleteButton = new QPushButton("Delete", this->Internals->ButtonsFrame);
  this->Internals->DeleteButton->setSizePolicy(sizeFixedPolicy);
  this->Internals->CopyButton = new QPushButton("Copy", this->Internals->ButtonsFrame);
  this->Internals->CopyButton->setSizePolicy(sizeFixedPolicy);
  this->Internals->DefsCombo = new QComboBox(this->Internals->ButtonsFrame);
  this->Internals->DefsCombo->setVisible(false);
  buttonLayout->addWidget(this->Internals->DefsCombo);

  buttonLayout->addWidget(this->Internals->AddButton);
  buttonLayout->addWidget(this->Internals->CopyButton);
  buttonLayout->addWidget(this->Internals->DeleteButton);

  // Attribute table
  this->Internals->ValuesTable = new qtTableWidget(frame);
  this->Internals->ValuesTable->setSizePolicy(tableSizePolicy);

  TopLayout->addWidget(this->Internals->FiltersFrame);
  TopLayout->addWidget(this->Internals->ButtonsFrame);
  TopLayout->addWidget(this->Internals->ListTable);

  BottomLayout->addWidget(this->Internals->ValuesTable);
  this->Internals->ValuesTable->setVisible(0);

  // Attribte frame
  this->Internals->AttFrame = new QFrame(frame);
  new QVBoxLayout(this->Internals->AttFrame);
  BottomLayout->addWidget(this->Internals->AttFrame);
//  this->Internals->AttFrame->setVisible(0);

  frame->addWidget(TopFrame);
  frame->addWidget(BottomFrame);

  // the association widget
  this->Internals->ReferencesWidget = new qtReferencesWidget(frame);
  this->Internals->AssociationsWidget = new qtAssociationWidget(frame, this);
  this->updateAssociationEnableState(smtk::attribute::AttributePtr());
  BottomLayout->addWidget(this->Internals->AssociationsWidget);
  BottomLayout->addWidget(this->Internals->ReferencesWidget);

  this->Internals->ListTable->horizontalHeader()->setResizeMode(
    QHeaderView::ResizeToContents);
  this->Internals->ValuesTable->horizontalHeader()->setResizeMode(
    QHeaderView::ResizeToContents);
  this->Internals->ValuesTable->verticalHeader()->setResizeMode(
    QHeaderView::ResizeToContents);
  this->Internals->ListTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  this->Internals->ValuesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  // signals/slots
  QObject::connect(this->Internals->AssociationsWidget,
    SIGNAL(attAssociationChanged()), this, SIGNAL(attAssociationChanged()));

  QObject::connect(this->Internals->ViewByCombo,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onViewBy(int)));

  QObject::connect(this->Internals->ListTable,
    SIGNAL(itemClicked (QTableWidgetItem*)),
    this, SLOT(onListBoxClicked(QTableWidgetItem*)));
  QObject::connect(this->Internals->ListTable,
    SIGNAL(itemSelectionChanged ()),
    this, SLOT(onListBoxSelectionChanged()));
  QObject::connect(this->Internals->ListTable,
    SIGNAL(itemChanged (QTableWidgetItem *)),
    this, SLOT(onAttributeNameChanged(QTableWidgetItem * )));
  // we need this so that the attribute name will also be changed
  // when a recorded test is play back, which is using setText
  // on the underline QLineEdit of the cell.
  QObject::connect(this->Internals->ListTable,
    SIGNAL(cellChanged (int, int)),
    this, SLOT(onAttributeCellChanged(int, int)));

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

  this->Internals->ValuesTable->setVisible(0);

  this->Widget = frame;
  this->getAllDefinitions();

  this->updateModelAssociation();
}

//----------------------------------------------------------------------------
void qtAttributeView::updateModelAssociation()
{
  this->onShowCategory();
}
//-----------------------------------------------------------------------------
smtk::attribute::AttributePtr qtAttributeView::getAttributeFromItem(
  QTableWidgetItem * item)
{
  if(this->Internals->ViewByCombo->currentIndex() == VIEWBY_Attribute)
    {
    Attribute* rawPtr = item ?
      static_cast<Attribute*>(item->data(Qt::UserRole).value<void *>()) : NULL;
    return rawPtr ? rawPtr->pointer() : smtk::attribute::AttributePtr();
    }
  return smtk::attribute::AttributePtr();
}
//-----------------------------------------------------------------------------
smtk::attribute::ItemPtr qtAttributeView::getAttributeItemFromItem(
  QTableWidgetItem * item)
{
  if(this->Internals->ViewByCombo->currentIndex() == VIEWBY_PROPERTY)
    {
    Item* rawPtr = item ?
      static_cast<Item*>(item->data(Qt::UserRole).value<void *>()) : NULL;
    return rawPtr ? rawPtr->pointer() : smtk::attribute::ItemPtr();
    }
  return smtk::attribute::ItemPtr();
}
//-----------------------------------------------------------------------------
smtk::attribute::AttributePtr qtAttributeView::getSelectedAttribute()
{
  return this->getAttributeFromItem(this->getSelectedItem());
}

//-----------------------------------------------------------------------------
QTableWidgetItem *qtAttributeView::getSelectedItem()
{
  return this->Internals->ListTable->selectedItems().count()>0 ?
    this->Internals->ListTable->selectedItems().value(0) : NULL;
}

//----------------------------------------------------------------------------
void qtAttributeView::updateAssociationEnableState(
  smtk::attribute::AttributePtr theAtt)
{
  bool rvisible=false, avisible=false;
  if(theAtt)
    {
    if(theAtt->definition()->associationMask())
      {
      avisible = true;
      this->Internals->AssociationsWidget->showEntityAssociation(theAtt);
      }
    }
  this->Internals->AssociationsWidget->setVisible(avisible);
  this->Internals->ReferencesWidget->setVisible(rvisible);
}

//----------------------------------------------------------------------------
void qtAttributeView::onListBoxSelectionChanged()
{
  if(this->Internals->ViewByCombo->currentIndex() == VIEWBY_PROPERTY)
    {
    return;
    }

  this->Internals->ValuesTable->blockSignals(true);
  this->Internals->ValuesTable->clear();
  this->Internals->ValuesTable->setRowCount(0);
  this->Internals->ValuesTable->setColumnCount(0);
  QTableWidgetItem* current = this->getSelectedItem();

  if(current)
    {
    smtk::attribute::AttributePtr dataItem = this->getAttributeFromItem(current);
    this->updateAssociationEnableState(dataItem);
    if(dataItem)
      {
      this->updateTableWithAttribute(dataItem);
      }
    }
  else
    {
    delete this->Internals->CurrentAtt;
    this->Internals->CurrentAtt = NULL;
    this->updateAssociationEnableState(smtk::attribute::AttributePtr());
    }

  this->Internals->ValuesTable->blockSignals(false);
  this->Internals->ValuesTable->resizeRowsToContents();
  this->Internals->ValuesTable->resizeColumnsToContents();
  this->Internals->ValuesTable->update();
}

//----------------------------------------------------------------------------
void qtAttributeView::onAttributeNameChanged(QTableWidgetItem* item)
{
  smtk::attribute::AttributePtr aAttribute = this->getAttributeFromItem(item);
  if(aAttribute && item->text().toStdString() != aAttribute->name())
    {
    System *attSystem = aAttribute->definition()->system();
    attSystem->rename(aAttribute, item->text().toStdString());
    //aAttribute->definition()->setLabel(item->text().toAscii().constData());
    }
}
//----------------------------------------------------------------------------
void qtAttributeView::onAttributeCellChanged(int row, int col)
{
  if(col == 0 && this->Internals->ViewByCombo->currentIndex() == VIEWBY_Attribute)
    {
    QTableWidgetItem* item = this->Internals->ListTable->item(row, col);
    this->onAttributeNameChanged(item);
    }
}
//----------------------------------------------------------------------------
void qtAttributeView::updateTableWithProperties()
{
  if(this->Internals->ViewByCombo->currentIndex() == VIEWBY_Attribute)
    {
    return;
    }

  this->Internals->ValuesTable->blockSignals(true);
  this->Internals->ValuesTable->clear();
  this->Internals->ValuesTable->setRowCount(0);
  this->Internals->ValuesTable->setColumnCount(0);
//  this->Internals->ComparedPropMap.clear();
  this->updateAssociationEnableState(smtk::attribute::AttributePtr());

  Definition* rawPtr = static_cast<Definition*>(
    this->Internals->PropDefsCombo->itemData(
    this->Internals->PropDefsCombo->currentIndex(), Qt::UserRole).value<void *>());
  if(!rawPtr)
    {
    this->Internals->ValuesTable->blockSignals(false);
    return;
    }

  std::vector<smtk::attribute::AttributePtr> result;
  System *attSystem = rawPtr->pointer()->system();
  attSystem->findAttributes(rawPtr->pointer(), result);
  if(result.size() == 0)
    {
    this->Internals->ValuesTable->blockSignals(false);
    return;
    }

  // create column headers
  QTableWidget* vtWidget = this->Internals->ValuesTable;
  vtWidget->setColumnCount(1);
  vtWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Property"));

  std::vector<smtk::attribute::AttributePtr>::iterator it;
  int j = 1;
  for (it=result.begin(); it!=result.end(); ++it)
    {
    if(!this->Internals->AttSelections.contains((*it)->name()) ||
       this->Internals->AttSelections[(*it)->name()] == Qt::Unchecked)
      {
      continue;
      }
    this->insertTableColumn(vtWidget, j, (*it)->name().c_str(),
      rawPtr->pointer()->advanceLevel());
    j++;
    }

  for(int r=1; r<this->Internals->SelectPropCombo->count(); r++)
    {
    QStandardItem* current = this->Internals->checkablePropComboModel->item(r);
    if(current && current->checkState() == Qt::Checked)
      {
      Item* irawPtr =
        static_cast<Item*>(current->data(Qt::UserRole).value<void *>());
      if(irawPtr)
        {
        smtk::attribute::ItemPtr attItem =
          irawPtr ? irawPtr->pointer() : smtk::attribute::ItemPtr();
        smtk::attribute::AttributePtr att = attItem->attribute();
        this->addComparativeProperty(current, att->definition());
        }
      }
    }

  this->Internals->ValuesTable->blockSignals(false);
  this->Internals->ValuesTable->resizeColumnsToContents();
  this->Internals->ValuesTable->resizeRowsToContents();
}

//----------------------------------------------------------------------------
void qtAttributeView::insertTableColumn(QTableWidget* vtWidget, int insertCol,
  const QString& title, int advancedlevel)
{
  vtWidget->insertColumn(insertCol);
  vtWidget->setHorizontalHeaderItem(insertCol, new QTableWidgetItem(title));

  if(advancedlevel)
    {
    vtWidget->horizontalHeaderItem(insertCol)->setFont(this->uiManager()->advancedFont());
    }
}

//----------------------------------------------------------------------------
void qtAttributeView::onAttributeValueChanged(QTableWidgetItem* item)
{
  Item* linkedData = item ?
    static_cast<Item*>(item->data(Qt::UserRole).value<void *>()) : NULL;

  if(linkedData && linkedData->isOptional())
    {
    linkedData->setIsEnabled(item->checkState()==Qt::Checked);
    this->updateChildWidgetsEnableState(
     linkedData->pointer(), item);
    }
}
//----------------------------------------------------------------------------
void qtAttributeView::updateChildWidgetsEnableState(
  smtk::attribute::ItemPtr attItem, QTableWidgetItem* item)
{
  if(!item || !attItem || !attItem->isOptional())
    {
    return;
    }
  bool bEnabled = attItem->isEnabled();
  int startRow = item->row();

  if(attItem->type() == smtk::attribute::Item::GROUP)
    {
    smtk::attribute::GroupItemPtr grpItem = dynamic_pointer_cast<GroupItem>(attItem);
    if(grpItem)
      {
      int numItems = static_cast<int>(grpItem->numberOfItemsPerGroup());
      for (int j = 0; j < numItems; j++) // expecting one item for each column
        {
        this->updateItemWidgetsEnableState(
          grpItem->item(j), startRow,bEnabled);
        }
      }
    }
  else
    {
    this->updateItemWidgetsEnableState(attItem, startRow, bEnabled);
    }
}

//----------------------------------------------------------------------------
void qtAttributeView::updateItemWidgetsEnableState(
  smtk::attribute::ItemPtr inData, int& startRow, bool enabled)
{
  QTableWidget* tableWidget = this->Internals->ValuesTable;
  if(inData->type() == smtk::attribute::Item::ATTRIBUTE_REF)
    {
    QWidget* cellWidget = tableWidget->cellWidget(startRow, 1);
    if(cellWidget)
      {
      cellWidget->setEnabled(enabled);
      }
    }
  else if(inData->type() == smtk::attribute::Item::VOID)
    {
    QWidget* cellWidget = tableWidget->cellWidget(startRow, 0);
    if(cellWidget)
      {
      cellWidget->setEnabled(enabled);
      }
    }
  else if(dynamic_pointer_cast<ValueItem>(inData))
    {
    smtk::attribute::ValueItemPtr linkedData = dynamic_pointer_cast<ValueItem>(inData);
    for(std::size_t row=0; row<linkedData->numberOfValues(); row++, startRow++)
      {
      QWidget* cellWidget = tableWidget->cellWidget(startRow, 1);
      if(cellWidget)
        {
        cellWidget->setEnabled(enabled);
        }
      }
    }
}

//----------------------------------------------------------------------------
void qtAttributeView::onCreateNew()
{
  if(!this->Internals->m_attDefinitions.size())
    {
    return;
    }
  attribute::DefinitionPtr newAttDef = this->Internals->m_attDefinitions[0];

  QString strDef = this->Internals->DefsCombo->currentText();
  foreach (attribute::DefinitionPtr attDef,
    this->Internals->getCurrentDefs(
      this->uiManager()->currentCategory().c_str()))
    {
    std::string txtDef = attDef->label().empty() ?
      attDef->type() : attDef->label();
    if(strDef == QString::fromUtf8(txtDef.c_str()))
      {
      newAttDef = attDef;
      break;
      }
    }
  this->createNewAttribute(newAttDef);
}

//----------------------------------------------------------------------------
void qtAttributeView::createNewAttribute(
  smtk::attribute::DefinitionPtr attDef)
{
  if(!attDef)
    {
    return;
    }

  System *attSystem = attDef->system();

  smtk::attribute::AttributePtr newAtt = attSystem->createAttribute(attDef->type());
  QTableWidgetItem* item = this->addAttributeListItem(newAtt);
  if(item)
    {
    this->Internals->ListTable->selectRow(item->row());
    }
  emit this->numOfAttriubtesChanged();
}

//----------------------------------------------------------------------------
void qtAttributeView::onCopySelected()
{
  smtk::attribute::AttributePtr newObject, selObject = this->getSelectedAttribute();
  if(!selObject)
    {
    return;
    }
  
  System *attSystem = selObject->system();
  newObject = attSystem->copyAttribute(selObject);
  if (newObject)
    {
    QTableWidgetItem* item = this->addAttributeListItem(newObject);
    if(item)
      {
      this->Internals->ListTable->selectRow(item->row());
      }
    emit this->numOfAttriubtesChanged();
    }
}

//----------------------------------------------------------------------------
void qtAttributeView::onDeleteSelected()
{
  smtk::attribute::AttributePtr selObject = this->getSelectedAttribute();
  if(selObject)
    {
    std::string keyName = selObject->name();
    this->Internals->AttSelections.remove(keyName);

    attribute::DefinitionPtr attDef = selObject->definition();
    System *attSystem = attDef->system();
    attSystem->removeAttribute(selObject);

    QTableWidgetItem* selItem = this->getSelectedItem();
    this->Internals->ListTable->removeRow(selItem->row());
    emit this->numOfAttriubtesChanged();
    }
}

//----------------------------------------------------------------------------
QTableWidgetItem* qtAttributeView::addAttributeListItem(
  smtk::attribute::AttributePtr childData)
{
  QTableWidgetItem* item = new QTableWidgetItem(
      QString::fromUtf8(childData->name().c_str()),
      smtk_USER_DATA_TYPE);
  Qt::ItemFlags nonEditableFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  QVariant vdata;
  vdata.setValue(static_cast<void*>(childData.get()));
  item->setData(Qt::UserRole, vdata);
  item->setFlags(nonEditableFlags | Qt::ItemIsEditable);

  int numRows = this->Internals->ListTable->rowCount();
  this->Internals->ListTable->setRowCount(++numRows);
  this->Internals->ListTable->setItem(numRows-1, 0, item);

  // add the type column too.
  std::string txtDef = childData->definition()->label().empty() ?
    childData->definition()->type() : childData->definition()->label();

  QTableWidgetItem* defitem = new QTableWidgetItem(
    QString::fromUtf8(txtDef.c_str()),
    smtk_USER_DATA_TYPE);
  defitem->setFlags(nonEditableFlags);
  this->Internals->ListTable->setItem(numRows-1, 1, defitem);

  QTableWidgetItem* colorItem =  new QTableWidgetItem();
  this->Internals->ListTable->setItem(numRows-1, 2,colorItem);
  const double* rgba = childData->color();
  QBrush bgBrush(QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]));
  colorItem->setBackground(bgBrush);
  colorItem->setFlags(Qt::ItemIsEnabled);
  return item;
}

//----------------------------------------------------------------------------
void qtAttributeView::onViewBy(int viewBy)
{
  if(!this->Internals->m_attDefinitions.size())
    {
    return;
    }

  QList<smtk::attribute::DefinitionPtr> currentDefs =
    this->Internals->getCurrentDefs(
    this->uiManager()->currentCategory().c_str());
  this->Internals->AddButton->setEnabled(currentDefs.count()>0);

  bool viewAtt = (viewBy == VIEWBY_Attribute);
  this->Internals->ButtonsFrame->setVisible(viewAtt);
  this->Internals->ListTable->setVisible(viewAtt);
  this->Internals->ListTable->blockSignals(true);
  this->Internals->ListTable->clear();
  this->Internals->ListTable->setRowCount(0);

  int numCols = viewAtt ? 3 : 2;
  this->Internals->ListTable->setColumnCount(numCols);
  this->Internals->ListTable->setHorizontalHeaderItem(
    0, new QTableWidgetItem(viewAtt ? "Attribute" : "Property"));
  this->Internals->ListTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Type"));
  if(viewAtt)
    {
    this->Internals->ListTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Color"));
    }
  this->Internals->DefsCombo->clear();
  this->Internals->PropDefsCombo->clear();
  foreach (attribute::DefinitionPtr attDef, currentDefs)
    {
    if(!attDef->isAbstract())
      {
      std::string txtDef = attDef->label().empty() ?
        attDef->type() : attDef->label();
      this->Internals->DefsCombo->addItem(
        QString::fromUtf8(txtDef.c_str()));
      this->Internals->PropDefsCombo->addItem(
        QString::fromUtf8(txtDef.c_str()));
      QVariant vdata;
      vdata.setValue(static_cast<void*>(attDef.get()));
      int idx = this->Internals->PropDefsCombo->count()-1;
      this->Internals->PropDefsCombo->setItemData(idx, vdata, Qt::UserRole);
      }
    }

  this->Internals->DefsCombo->setCurrentIndex(0);
  this->Internals->DefsCombo->setVisible(true);
  this->Internals->PropDefsCombo->setVisible(!viewAtt);
  this->Internals->PropDefsCombo->blockSignals(true);
  this->Internals->PropDefsCombo->setCurrentIndex(0);
  this->Internals->PropDefsCombo->blockSignals(false);

  this->initSelectionFilters();
  if(viewAtt)
    {
    foreach (attribute::DefinitionPtr attDef, currentDefs)
      {
      this->onViewByWithDefinition(viewBy, attDef);
      }
    }
  this->Internals->ListTable->blockSignals(false);
  this->Internals->ListTable->resizeColumnsToContents();

  QSplitter* frame = qobject_cast<QSplitter*>(this->Widget);
  if(viewAtt)
    {
    if(this->Internals->ListTable->rowCount() && !this->getSelectedItem())
      {
      this->Internals->ListTable->selectRow(0);
      }
    else
      {
      this->onListBoxSelectionChanged();
      }
    }
  else
    {
    this->updateTableWithProperties();
    QList<int> sizes;
    sizes.push_back(this->Internals->ViewByCombo->height());
    sizes.push_back(this->Internals->ValuesTable->height());
    frame->setSizes(sizes);
    }
  frame->handle(1)->setEnabled(viewAtt);
}

//----------------------------------------------------------------------------
void qtAttributeView::onViewByWithDefinition(
  int viewBy, smtk::attribute::DefinitionPtr attDef)
{
  if(viewBy == VIEWBY_PROPERTY)
    {
    return;
    }
  std::vector<smtk::attribute::AttributePtr> result;
  System *attSystem = attDef->system();
  attSystem->findAttributes(attDef, result);
  if(result.size() && viewBy == VIEWBY_Attribute)
    {
    std::vector<smtk::attribute::AttributePtr>::iterator it;
    for (it=result.begin(); it!=result.end(); ++it)
      {
      QTableWidgetItem* item = this->addAttributeListItem(*it);
      if((*it)->definition()->advanceLevel())
        {
        item->setFont(this->uiManager()->advancedFont());
        }
      }
    }
}
//----------------------------------------------------------------------------
void qtAttributeView::onShowCategory()
{
  int viewBy = this->Internals->ViewByCombo->currentIndex();
  this->onViewBy(viewBy);
}

//----------------------------------------------------------------------------
void qtAttributeView::updateTableWithAttribute(
  smtk::attribute::AttributePtr att)
{
  this->Internals->ValuesTable->setVisible(0);
  this->Internals->AttFrame->setVisible(1);

  if(this->Internals->CurrentAtt && this->Internals->CurrentAtt->widget())
    {
    delete this->Internals->CurrentAtt;
    }

  int currentLen = this->fixedLabelWidth();
  int tmpLen = this->uiManager()->getWidthOfAttributeMaxLabel(
    att->definition(), this->uiManager()->advancedFont());
  this->setFixedLabelWidth(tmpLen);

  this->Internals->CurrentAtt = new qtAttribute(att, this->Internals->AttFrame, this);
  // By default use the basic layout with no model associations since this class
  // takes care of it
  this->Internals->CurrentAtt->createBasicLayout(false);
  this->setFixedLabelWidth(currentLen);
  if(this->Internals->CurrentAtt && this->Internals->CurrentAtt->widget())
    {
    this->Internals->AttFrame->layout()->addWidget(
      this->Internals->CurrentAtt->widget());
    if(this->advanceLevelVisible())
      {
      this->Internals->CurrentAtt->showAdvanceLevelOverlay(true);
      }
    }
}
//----------------------------------------------------------------------------
void qtAttributeView::initSelectionFilters()
{
  bool viewAtt = this->Internals->ViewByCombo->currentIndex() == VIEWBY_Attribute;
  this->Internals->SelectAttCombo->setVisible(!viewAtt);
  this->Internals->SelectPropCombo->setVisible(!viewAtt);
  if(viewAtt)
    {
    return;
    }
  this->Internals->AttFrame->setVisible(0);
  this->Internals->ValuesTable->setVisible(1);

  Definition* rawPtr = static_cast<Definition*>(
    this->Internals->PropDefsCombo->itemData(
    this->Internals->PropDefsCombo->currentIndex(), Qt::UserRole).value<void *>());
  if(!rawPtr)
    {
    return;
    }
  this->initSelectPropCombo(rawPtr->pointer());
  this->initSelectAttCombo(rawPtr->pointer());
  this->updateTableWithProperties();
}

//----------------------------------------------------------------------------
void qtAttributeView::initSelectPropCombo(
  smtk::attribute::DefinitionPtr attDef)
{
  this->Internals->SelectPropCombo->blockSignals(true);
  this->Internals->SelectPropCombo->clear();
  this->Internals->SelectPropCombo->init();
  this->Internals->checkablePropComboModel->disconnect();
  if(!attDef)
    {
    this->Internals->SelectPropCombo->blockSignals(false);
    return;
    }
  std::vector<smtk::attribute::AttributePtr> result;
  System *attSystem = attDef->system();
  attSystem->findAttributes(attDef, result);
  if(result.size() == 0)
    {
    this->Internals->SelectPropCombo->blockSignals(false);
    return;
    }

  smtk::attribute::AttributePtr childData = result[0];
  // Now lets process its items
  std::size_t i, n = childData->numberOfItems();
  int row=1;
  for (i = 0; i < n; i++)
    {
    smtk::attribute::ItemPtr attItem = childData->item(static_cast<int>(i));
    if(this->uiManager()->passItemCategoryCheck(
        attItem->definition()) &&
      this->uiManager()->passAdvancedCheck(
      attItem->advanceLevel()))
      {
      // No User data, not editable
      std::string strItemLabel = attItem->label().empty() ? attItem->name() : attItem->label();
      std::string keyName = childData->definition()->type() + strItemLabel;
      QStandardItem* item = new QStandardItem;
      item->setText(strItemLabel.c_str());
      if(!this->Internals->AttProperties.contains(keyName))
        {
        this->Internals->AttProperties[keyName] = Qt::Unchecked;
        }

      item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
      //item->setData(this->Internals->AttSelections[keyName], Qt::CheckStateRole);
      item->setData(Qt::Unchecked, Qt::CheckStateRole);
      item->setCheckable(true);
      item->setCheckState(this->Internals->AttProperties[keyName]);
      QVariant vdata;
      vdata.setValue(static_cast<void*>(attItem.get()));
      item->setData(vdata, Qt::UserRole);
      this->Internals->checkablePropComboModel->insertRow(row++, item);
      if(attItem->advanceLevel())
        {
        item->setFont(this->uiManager()->advancedFont());
        }
      }
    }

  connect(this->Internals->checkablePropComboModel,
    SIGNAL(dataChanged ( const QModelIndex&, const QModelIndex&)),
    this, SLOT(propertyFilterChanged(const QModelIndex&, const QModelIndex&)));

  this->Internals->SelectPropCombo->blockSignals(false);
  this->Internals->SelectPropCombo->updateText();
  this->Internals->SelectPropCombo->hidePopup();
}

//----------------------------------------------------------------------------
void qtAttributeView::initSelectAttCombo(smtk::attribute::DefinitionPtr attDef)
{
  this->Internals->SelectAttCombo->blockSignals(true);
  this->Internals->SelectAttCombo->clear();
  this->Internals->SelectAttCombo->init();
  this->Internals->checkableAttComboModel->disconnect();

  if(!attDef)
    {
    this->Internals->SelectAttCombo->blockSignals(false);
    return;
    }

  std::vector<smtk::attribute::AttributePtr> result;
  System *attSystem = attDef->system();
  attSystem->findAttributes(attDef, result);
  std::vector<smtk::attribute::AttributePtr>::iterator it;
  int row=1;
  for (it=result.begin(); it!=result.end(); ++it, ++row)
    {
    QStandardItem* item = new QStandardItem;
    item->setText((*it)->name().c_str());
    std::string keyName = (*it)->name();
    if(!this->Internals->AttSelections.contains(keyName))
      {
      this->Internals->AttSelections[keyName] = Qt::Unchecked;
      }

    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    //item->setData(this->Internals->AttSelections[keyName], Qt::CheckStateRole);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    item->setCheckable(true);
    item->setCheckState(this->Internals->AttSelections[keyName]);
    QVariant vdata;
    vdata.setValue(static_cast<void*>((*it).get()));
    item->setData(vdata, Qt::UserRole);
    this->Internals->checkableAttComboModel->insertRow(row, item);
    if((*it)->definition()->advanceLevel())
      {
      item->setFont(this->uiManager()->advancedFont());
      }
    }
  connect(this->Internals->checkableAttComboModel,
    SIGNAL(dataChanged ( const QModelIndex&, const QModelIndex&)),
    this, SLOT(attributeFilterChanged(const QModelIndex&, const QModelIndex&)));

  //connect(this->Internals->checkableAttComboModel, SIGNAL(itemChanged ( QStandardItem*)),
  //  this, SLOT(attributeFilterChanged(QStandardItem*)));
  this->Internals->SelectAttCombo->blockSignals(false);
  this->Internals->SelectAttCombo->updateText();
  this->Internals->SelectAttCombo->hidePopup();
}

//----------------------------------------------------------------------------
void qtAttributeView::propertyFilterChanged(
  const QModelIndex& topLeft, const QModelIndex& /* bottomRight */)
{
  if(this->Internals->ViewByCombo->currentIndex() == VIEWBY_Attribute)
    {
    return;
    }

  QStandardItem* item = this->Internals->checkablePropComboModel->item(topLeft.row());
  if(!item)
    {
    return;
    }
  smtk::attribute::Item* rawPtr =
    static_cast<smtk::attribute::Item*>(item->data(Qt::UserRole).value<void *>());

  if(rawPtr)
    {
  this->Internals->ValuesTable->blockSignals(true);
    smtk::attribute::ItemPtr attItem = rawPtr->pointer();
    smtk::attribute::AttributePtr att = attItem->attribute();
    std::string keyName = att->definition()->type() + item->text().toStdString();
    this->Internals->AttProperties[keyName] = item->checkState();
    if(item->checkState() == Qt::Checked)
      {
      this->addComparativeProperty(item, att->definition());
      }
    else
      {
      this->removeComparativeProperty(item->text());
      }
    this->Internals->SelectPropCombo->updateText();
    this->Internals->ValuesTable->blockSignals(false);
    this->Internals->ValuesTable->resizeColumnsToContents();
    this->Internals->ValuesTable->resizeRowsToContents();
    }
}

//----------------------------------------------------------------------------
void qtAttributeView::attributeFilterChanged(
  const QModelIndex& topLeft, const QModelIndex& /* bottomRight */)
{
  QStandardItem* item =
    this->Internals->checkableAttComboModel->item(topLeft.row());
  if(!item)
    {
    return;
    }
  Attribute* rawPtr =
    static_cast<Attribute*>(item->data(Qt::UserRole).value<void *>());
  if(rawPtr)
    {
    this->Internals->ValuesTable->blockSignals(true);
    this->Internals->AttSelections[rawPtr->name()] = item->checkState();
    if(item->checkState() == Qt::Checked)
      {
      this->addComparativeAttribute(rawPtr->pointer());
      }
    else
      {
      this->removeComparativeAttribute(rawPtr->pointer());
      }
    this->Internals->SelectAttCombo->updateText();
    this->Internals->ValuesTable->blockSignals(false);
    this->Internals->ValuesTable->resizeColumnsToContents();
    this->Internals->ValuesTable->resizeRowsToContents();
    }
}

//----------------------------------------------------------------------------
void qtAttributeView::addComparativeAttribute(
  smtk::attribute::AttributePtr att)
{
  if(!att)
    {
    return;
    }

  QTableWidget* vtWidget = this->Internals->ValuesTable;
  this->insertTableColumn(vtWidget, vtWidget->columnCount(),
    att->name().c_str(), att->definition()->advanceLevel());

  int col = vtWidget->columnCount() - 1;
  std::size_t i, n = att->numberOfItems();
  for(int row=0; row<vtWidget->rowCount(); ++row)
    {
    for (i = 0; i < n; i++)// for each property
      {
      smtk::attribute::ItemPtr attItem = att->item(static_cast<int>(i));
      std::string strItemLabel = attItem->label().empty() ? attItem->name() : attItem->label();
      if(vtWidget->item(row, 0)->text() == strItemLabel.c_str())
        {
        qtItem* qItem = qtAttribute::createItem(attItem, NULL, this, Qt::Vertical);
        qItem->setLabelVisible(false);
        qtAttributeRefItem* arItem = qobject_cast<qtAttributeRefItem*>(qItem);
        if(arItem)
          {
          arItem->setAttributeWidgetVisible(false);
          }
        vtWidget->setCellWidget(row, col, qItem->widget());
        vtWidget->setItem(row, col, new QTableWidgetItem());
       break;
        }
      }
    }
}

//----------------------------------------------------------------------------
void qtAttributeView::removeComparativeAttribute(
  smtk::attribute::AttributePtr att)
{
  if(!att)
    {
    return;
    }

  QTableWidget* vtWidget = this->Internals->ValuesTable;
  for(int c=1; c< vtWidget->columnCount(); c++)
    {
    if(vtWidget->horizontalHeaderItem(c)->text().toStdString() == att->name())
      {
      vtWidget->removeColumn(c);
      break;
      }
    }
//  vtWidget->resizeRowsToContents();
}

//----------------------------------------------------------------------------
void qtAttributeView::onPropertyDefSelected()
{
  if(this->Internals->ViewByCombo->currentIndex() == VIEWBY_Attribute)
    {
    return;
    }

  Definition* rawPtr = static_cast<Definition*>(
    this->Internals->PropDefsCombo->itemData(
    this->Internals->PropDefsCombo->currentIndex(), Qt::UserRole).value<void *>());
  if(!rawPtr)
    {
    return;
    }

  this->initSelectionFilters();
  this->updateTableWithProperties();
}

//----------------------------------------------------------------------------
void qtAttributeView::removeComparativeProperty(const QString& propertyName)
{
  if(!this->Internals->m_attDefinitions.size())
    {
    return;
    }

  QTableWidget* vtWidget = this->Internals->ValuesTable;
  for(int c=0; c< vtWidget->rowCount(); c++)
    {
    if(vtWidget->item(c, 0)->text() == propertyName)
      {
      vtWidget->removeRow(c);
      break;
      }
    }
//  vtWidget->resizeRowsToContents();
}
//----------------------------------------------------------------------------
void qtAttributeView::addComparativeProperty(
  QStandardItem* current, smtk::attribute::DefinitionPtr attDef)
{
  if(!this->Internals->m_attDefinitions.size())
    {
    return;
    }

  Definition* rawPtr = static_cast<Definition*>(
    this->Internals->PropDefsCombo->itemData(
    this->Internals->PropDefsCombo->currentIndex(), Qt::UserRole).value<void *>());
  if(rawPtr->pointer() != attDef)
    {
    return;
    }

  QTableWidget* vtWidget = this->Internals->ValuesTable;

  std::vector<smtk::attribute::AttributePtr> result;
  System *attSystem = attDef->system();
  attSystem->findAttributes(attDef, result);

  int numRows = this->Internals->ValuesTable->rowCount();
  int insertRow = numRows;
  vtWidget->insertRow(insertRow);
  vtWidget->setItem(insertRow, 0, new QTableWidgetItem(current->text()));
  vtWidget->item(insertRow, 0)->setFont(current->font());

  std::vector<smtk::attribute::AttributePtr>::iterator it;
  int col=1;
  for (it=result.begin(); it!=result.end(); ++it)
    {
    if(!this->Internals->AttSelections.contains((*it)->name()) ||
       this->Internals->AttSelections[(*it)->name()] == Qt::Unchecked)
      {
      continue;
      }
    std::size_t i, n = (*it)->numberOfItems();
    for (i = 0; i < n; i++)// for each property
      {
      smtk::attribute::ItemPtr attItem = (*it)->item(static_cast<int>(i));
      std::string strItemLabel = attItem->label().empty() ? attItem->name() : attItem->label();
      if(current->text() == strItemLabel.c_str())
        {
        qtItem* qItem = qtAttribute::createItem(attItem, NULL, this, Qt::Vertical);
        qItem->setLabelVisible(false);
        qtAttributeRefItem* arItem = qobject_cast<qtAttributeRefItem*>(qItem);
        if(arItem)
          {
          arItem->setAttributeWidgetVisible(false);
          }
        vtWidget->setCellWidget(insertRow, col, qItem->widget());
        vtWidget->setItem(insertRow, col, new QTableWidgetItem());
        break;
        }
      }
    col++;
    }
}

//----------------------------------------------------------------------------
int qtAttributeView::currentViewBy()
{
  return this->Internals->ViewByCombo->currentIndex();
}

//----------------------------------------------------------------------------
void qtAttributeView::getAllDefinitions()
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view)
    {
    return;
    }

  smtk::attribute::System *sys = this->uiManager()->attSystem();

  std::string attName, defName, val;
  smtk::attribute::AttributePtr att;
  smtk::attribute::DefinitionPtr attDef;
  bool flag;

  // The view should have a single internal component called InstancedAttributes
  if ((view->details().numberOfChildren() != 1) ||
      (view->details().child(0).name() != "AttributeTypes"))
    {
    // Should present error message
    return;
    }

  if (view->details().attributeAsBool("CreateEntities", flag))
    {
    this->Internals->m_okToCreateModelEntities = flag;
    }
  else
    {
    this->Internals->m_okToCreateModelEntities = false;
    }

  if (view->details().attribute("ModelEntityFilter", val))
    {
    smtk::model::BitFlags flags = smtk::model::Entity::specifierStringToFlag(val);
    this->Internals->m_modelEntityMask = flags;
    }
  else
    {
     this->Internals->m_modelEntityMask = 0;
    }

  std::vector<smtk::attribute::AttributePtr> atts;
  smtk::common::View::Component &attsComp = view->details().child(0);
  std::size_t i, n = attsComp.numberOfChildren();
  for (i = 0; i < n; i++)
    {
    if (attsComp.child(i).name() != "Att")
      {
      continue;
      }
    if (!attsComp.child(i).attribute("Type", defName))
      {
      continue;
      }

    attDef = sys->findDefinition(defName);
    this->qtBaseView::getDefinitions(attDef, this->Internals->AllDefs);
    this->Internals->m_attDefinitions.push_back(attDef);
    }

#ifndef _MSC_VER
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wshadow"
#endif
  foreach (smtk::attribute::DefinitionPtr adef, this->Internals->AllDefs)
    {
    foreach(QString category, this->Internals->AttDefMap.keys())
      {
      if(adef->isMemberOf(category.toStdString()) &&
        !this->Internals->AttDefMap[category].contains(adef))
        {
        this->Internals->AttDefMap[category].push_back(adef);
        }
      }
    }
#ifndef _MSC_VER
#  pragma GCC diagnostic pop
#endif
}

//----------------------------------------------------------------------------
void qtAttributeView::onListBoxClicked(QTableWidgetItem* item)
{
  if(this->Internals->ViewByCombo->currentIndex() == VIEWBY_Attribute)
    {
     bool isColor = item->column() == 2;
    if(isColor)
      {
      QTableWidgetItem* selItem = this->Internals->ListTable->item(item->row(), 0);
      smtk::attribute::AttributePtr selAtt = this->getAttributeFromItem(selItem);
      QBrush bgBrush = item->background();
      QColor color = QColorDialog::getColor(bgBrush.color(), this->Widget);
      if(color.isValid() && color != bgBrush.color() && selAtt)
        {
        bgBrush.setColor(color);
        item->setBackground(bgBrush);
        selAtt->setColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        emit this->attColorChanged();
        }
      if(!selItem->isSelected())
        {
        this->Internals->ListTable->setCurrentItem(selItem);
        selItem->setSelected(true);
        }
      }
    }
}

//----------------------------------------------------------------------------
void qtAttributeView::childrenResized()
{
  if(this->Internals->ValuesTable->isVisible())
    {
    this->Internals->ValuesTable->resizeRowsToContents();
    this->Internals->ValuesTable->resizeColumnsToContents();
    this->Internals->ValuesTable->update();
    this->Widget->update();
    }
}
//----------------------------------------------------------------------------
void qtAttributeView::showAdvanceLevelOverlay(bool show)
{
  if(this->Internals->ViewByCombo->currentIndex() == VIEWBY_Attribute &&
    this->Internals->CurrentAtt)
    {
    this->Internals->CurrentAtt->showAdvanceLevelOverlay(show);
    }
  this->qtBaseView::showAdvanceLevelOverlay(show);
}
