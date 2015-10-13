//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAttributeDisplay.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/model/AttributeAssignments.h"
#include "smtk/model/Manager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QTableWidgetItem>
#include <QVariant>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QModelIndex>
#include <QModelIndexList>
#include <QMessageBox>
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
class qtAttributeDisplayInternals
{
public:

  const QList<smtk::attribute::DefinitionPtr> getCurrentDefs(
    const std::string& strCategory) const
  {

    if(this->AttDefMap.keys().contains(strCategory))
      {
      return this->AttDefMap[strCategory];
      }
    return this->AllAssignedDefs;
  }
  qtTableWidget* ValuesTable;

  QFrame* FiltersFrame;

  // <category, AttDefinitions>
  QMap<std::string, QList<smtk::attribute::DefinitionPtr> > AttDefMap;

  // All definitions list
  QList<smtk::attribute::DefinitionPtr> AllAssignedDefs;

  // For filtering the attribute properties by combobox.
  QPointer<QCheckBox> FilterByCheck;
  QPointer<QComboBox> ShowCategoryCombo;
  QPointer<QComboBox> PropDefsCombo;
  QPointer<QComboBox> SelectPropCombo;

  QPointer<smtk::attribute::qtUIManager> UIManager;
  QPointer<QStandardItemModel> PropComboModel;

};

//----------------------------------------------------------------------------
qtAttributeDisplay::
qtAttributeDisplay(QWidget* p, smtk::attribute::qtUIManager* uiman) : QWidget(p)
{
  this->Internals = new qtAttributeDisplayInternals;
  this->Internals->UIManager = uiman;
  this->createWidget( );
}

//----------------------------------------------------------------------------
qtAttributeDisplay::~qtAttributeDisplay()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtAttributeDisplay::createWidget( )
{
  if(!this->Internals->UIManager)
    {
    return;
    }

  QVBoxLayout* thislayout = new QVBoxLayout(this);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  // create a filter-frame with Category-combo
  this->Internals->FiltersFrame = new QFrame(this);
  QGridLayout* filterLayout = new QGridLayout(this->Internals->FiltersFrame);
  filterLayout->setMargin(0);
  this->Internals->FiltersFrame->setSizePolicy(sizeFixedPolicy);
/*
  QFrame* BottomFrame = new QFrame(this);
  this->Internals->BottomFrame = BottomFrame;
  QVBoxLayout* BottomLayout = new QVBoxLayout(BottomFrame);
  BottomLayout->setMargin(0);
  BottomFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
*/
  const System* attSys = this->Internals->UIManager->attSystem();

  this->Internals->FilterByCheck = new QCheckBox(this->Internals->FiltersFrame);
  this->Internals->FilterByCheck->setText("Show by Category: ");
  this->Internals->FilterByCheck->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  this->Internals->ShowCategoryCombo = new QComboBox(this->Internals->FiltersFrame);
  std::set<std::string>::const_iterator it;
  const std::set<std::string> &cats = attSys->categories();
  for (it = cats.begin(); it != cats.end(); it++)
    {
    this->Internals->ShowCategoryCombo->addItem(it->c_str());
    }
  this->Internals->ShowCategoryCombo->setEnabled(false);

  int layoutcol = 0;
  if(this->Internals->ShowCategoryCombo->count() == 0)
    {
    this->Internals->FilterByCheck->setVisible(0);
    this->Internals->ShowCategoryCombo->setVisible(0);
    }
  else
    {
    filterLayout->addWidget(this->Internals->FilterByCheck, 0, layoutcol++);
    filterLayout->addWidget(this->Internals->ShowCategoryCombo, 0, layoutcol++);
    }
  QObject::connect(this->Internals->FilterByCheck,
    SIGNAL(stateChanged(int)), this, SLOT(enableShowBy(int)));

  this->Internals->PropDefsCombo = new QComboBox(this->Internals->FiltersFrame);
//  this->Internals->PropDefsCombo->setVisible(false);
  this->Internals->PropDefsCombo->setToolTip("Select definition to filter attributes");
  filterLayout->addWidget(this->Internals->PropDefsCombo, 0, layoutcol++);
  QObject::connect(this->Internals->PropDefsCombo,  SIGNAL(currentIndexChanged(int)),
    this, SLOT(onAttributeDefSelected()), Qt::QueuedConnection);

  this->Internals->SelectPropCombo = new QComboBox(this->Internals->FiltersFrame);
  this->Internals->SelectPropCombo->setToolTip("Select properties");
  this->Internals->PropComboModel = new QStandardItemModel(this);
  this->Internals->SelectPropCombo->setModel(
    this->Internals->PropComboModel);
  QObject::connect(this->Internals->SelectPropCombo,  SIGNAL(currentIndexChanged(int)),
    this, SLOT(onFieldSelected()), Qt::QueuedConnection);
  filterLayout->addWidget(this->Internals->SelectPropCombo, 0, layoutcol);

  this->Internals->AttDefMap.clear();

  thislayout->addWidget(this->Internals->FiltersFrame);
//  this->addWidget(BottomFrame);

  QObject::connect(this->Internals->ShowCategoryCombo,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onShowCategory()));

//  if(this->parentWidget()->layout())
//    {
//    this->parentWidget()->layout()->addWidget(this);
//    }

  this->getDefinitionsWithAssociations();
}

//----------------------------------------------------------------------------
void qtAttributeDisplay::enableShowBy(int enable)
{
  this->Internals->ShowCategoryCombo->setEnabled(enable);
  this->onShowCategory();
}

//-----------------------------------------------------------------------------
smtk::attribute::ItemPtr qtAttributeDisplay::getAttributeItemFromItem(
  QTableWidgetItem * item)
{
  Item* rawPtr = item ?
    static_cast<Item*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->pointer() : smtk::attribute::ItemPtr();
}

//----------------------------------------------------------------------------
void qtAttributeDisplay::onShowCategory()
{
  bool useCategory = this->Internals->FilterByCheck->isChecked();
  this->onShowCategory(useCategory ?
    this->Internals->ShowCategoryCombo->currentText().toStdString() : "");
}

//----------------------------------------------------------------------------
void qtAttributeDisplay::onShowCategory(const std::string& strCategory)
{
  this->Internals->SelectPropCombo->blockSignals(true);
  this->Internals->PropDefsCombo->blockSignals(true);
  QString currentDef = this->Internals->PropDefsCombo->currentText();
  QString currentItem = this->Internals->SelectPropCombo->currentText();
  this->Internals->SelectPropCombo->clear();
  this->Internals->PropDefsCombo->clear();
  this->Internals->SelectPropCombo->blockSignals(false);
  this->Internals->PropDefsCombo->blockSignals(false);

  this->getDefinitionsWithAssociations();
  if(!this->Internals->AllAssignedDefs.size())
    {
    return;
    }

  this->Internals->PropDefsCombo->blockSignals(true);

  QList<smtk::attribute::DefinitionPtr> currentDefs =
    this->Internals->getCurrentDefs(strCategory);
  foreach (attribute::DefinitionPtr attDef, currentDefs)
    {
    if(!attDef->isAbstract())
      {
      std::string txtDef = attDef->label().empty() ?
        attDef->type() : attDef->label();
      this->Internals->PropDefsCombo->addItem(
        QString::fromUtf8(txtDef.c_str()));
      QVariant vdata;
      vdata.setValue(static_cast<void*>(attDef.get()));
      int idx = this->Internals->PropDefsCombo->count()-1;
      this->Internals->PropDefsCombo->setItemData(idx, vdata, Qt::UserRole);
      }
    }

//  this->Internals->PropDefsCombo->setVisible(!viewAtt);
  if(this->Internals->PropDefsCombo->count() > 0)
    {
    int prevIdx = this->Internals->PropDefsCombo->findText(currentDef);
    this->Internals->PropDefsCombo->setCurrentIndex(prevIdx >= 0 ? prevIdx : 0);
    this->initSelectionFilters(currentItem);
    }

  this->Internals->PropDefsCombo->blockSignals(false);
//  this->updateTableWithProperties();
}

//----------------------------------------------------------------------------
void qtAttributeDisplay::initSelectionFilters(const QString& currentItemName)
{
//  this->Internals->ValuesTable->setVisible(1);

  Definition* rawPtr = static_cast<Definition*>(
    this->Internals->PropDefsCombo->itemData(
    this->Internals->PropDefsCombo->currentIndex(), Qt::UserRole).value<void *>());
  if(!rawPtr)
    return;

  this->initSelectPropCombo(rawPtr->pointer(), currentItemName);
  this->onFieldSelected();
  //  this->updateTableWithProperties();
}

//----------------------------------------------------------------------------
void qtAttributeDisplay::initSelectPropCombo(
  smtk::attribute::DefinitionPtr attDef, const QString& currentItemName)
{
  this->Internals->SelectPropCombo->blockSignals(true);
  this->Internals->SelectPropCombo->clear();
//  this->Internals->SelectPropCombo->init();
//  this->Internals->PropComboModel->disconnect();
  // Adding an entry "Attribute" for the case that we just
  // want the fact that the attribute existence, not an item value
  QStandardItem* pitem = new QStandardItem;
  pitem->setText("Attribute");
  this->Internals->PropComboModel->insertRow(0, pitem);
 
  if(!attDef)
    {
    this->Internals->SelectPropCombo->setCurrentIndex(0);
    this->Internals->SelectPropCombo->blockSignals(false);
    return;
    }
  std::vector<smtk::attribute::AttributePtr> result;
  System *attSystem = attDef->system();
  attSystem->findAttributes(attDef, result);
  if(result.size() == 0)
    {
    this->Internals->SelectPropCombo->setCurrentIndex(0);
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
    if(this->Internals->UIManager->passItemCategoryCheck(
        attItem->definition()) /*&&
      this->Internals->UIManager->passAdvancedCheck(
      attItem->advanceLevel()) */)
      {
      // No User data, not editable
      std::string strItemLabel = attItem->label().empty() ? attItem->name() : attItem->label();
      std::string keyName = childData->definition()->type() + strItemLabel;
      pitem = new QStandardItem;
      pitem->setText(strItemLabel.c_str());

      QVariant vdata;
      vdata.setValue(static_cast<void*>(attItem.get()));
      pitem->setData(vdata, Qt::UserRole);
      this->Internals->PropComboModel->insertRow(row++, pitem);
      if(attItem->advanceLevel())
        {
        pitem->setFont(this->Internals->UIManager->advancedFont());
        }
      }
    }
  int prevIdx = this->Internals->SelectPropCombo->findText(currentItemName);
  this->Internals->SelectPropCombo->setCurrentIndex(prevIdx >= 0 ? prevIdx : 0);
  this->Internals->SelectPropCombo->blockSignals(false);
}


//----------------------------------------------------------------------------
void qtAttributeDisplay::onFieldSelected()
{
   Definition* rawPtr = static_cast<Definition*>(
    this->Internals->PropDefsCombo->itemData(
    this->Internals->PropDefsCombo->currentIndex(), Qt::UserRole).value<void *>());
  if(!rawPtr)
    return;

  Item* irawPtr = NULL;
  int selrow = this->Internals->SelectPropCombo->currentIndex();
  if(selrow >= 0)
    {
    QStandardItem* item = this->Internals->PropComboModel->item(selrow);
    irawPtr = static_cast<Item*>(item->data(Qt::UserRole).value<void *>());
    }

  emit this->attributeFieldSelected(rawPtr->pointer()->type().c_str(),
    irawPtr ? irawPtr->pointer()->name().c_str() : "");
}

//----------------------------------------------------------------------------
void qtAttributeDisplay::onAttributeDefSelected()
{
  this->initSelectionFilters(this->Internals->SelectPropCombo->currentText());
  //this->updateTableWithProperties();
}

//----------------------------------------------------------------------------
void qtAttributeDisplay::getDefinitionsWithAssociations()
{
  if(!this->Internals->UIManager)
    return;

  smtk::attribute::System *attsys = this->Internals->UIManager->attSystem();
  this->Internals->AllAssignedDefs.clear();
  this->Internals->AttDefMap.clear();

  std::vector<smtk::attribute::AttributePtr> atts;
  attsys->attributes(atts);
  if(atts.size() == 0)
    return;

  if(!attsys->refModelManager())
    return;

  std::vector<smtk::attribute::AttributePtr>::const_iterator it;
  for(it = atts.begin(); it != atts.end(); ++it)
    {
    if((*it)->associatedModelEntityIds().size() == 0)
      {
      continue;
      }
    smtk::attribute::DefinitionPtr attDef = (*it)->definition();
    if(!this->Internals->AllAssignedDefs.contains(attDef))
      this->Internals->AllAssignedDefs.push_back(attDef);

    const std::set<std::string> &cats = attsys->categories();
    std::set<std::string>::const_iterator catit;
    for(catit = cats.begin(); catit != cats.end(); ++catit)
      {
      if(attDef->isMemberOf(*catit) &&
        !this->Internals->AttDefMap[*catit].contains(attDef))
        {
        this->Internals->AttDefMap[*catit].push_back(attDef);
        }
      }
    }
}
