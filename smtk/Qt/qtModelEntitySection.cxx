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

#include "smtk/Qt/qtModelEntitySection.h"

#include "smtk/Qt/qtUIManager.h"
#include "smtk/Qt/qtTableWidget.h"
#include "smtk/Qt/qtAttribute.h"
#include "smtk/Qt/qtItem.h"
#include "smtk/Qt/qtAssociationWidget.h"

#include "smtk/attribute/ModelEntitySection.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/model/Model.h"
#include "smtk/model/Item.h"
#include "smtk/model/GroupItem.h"

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

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtModelEntitySectionInternals
{
public:
  QListWidget* ListBox;
  QComboBox* ShowCategoryCombo;
  QFrame* FiltersFrame;
  QFrame* topFrame;
  QFrame* bottomFrame;
  QPointer<qtAssociationWidget> AssociationsWidget;
  std::vector<smtk::AttributeDefinitionPtr> attDefs;
};

//----------------------------------------------------------------------------
qtModelEntitySection::qtModelEntitySection(
  smtk::SectionPtr dataObj, QWidget* p) : qtSection(dataObj, p)
{
  this->Internals = new qtModelEntitySectionInternals;
  this->createWidget( );
}

//----------------------------------------------------------------------------
qtModelEntitySection::~qtModelEntitySection()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtModelEntitySection::createWidget( )
{
  if(!this->getObject())
    {
    return;
    }
  smtk::ModelEntitySectionPtr sec =
    smtk::dynamicCastPointer<ModelEntitySection>(this->getObject());
  if(!sec)
    {
    return;
    }

  Manager *attManager = qtUIManager::instance()->attManager();
  unsigned long mask = sec->modelEntityMask();
  if(mask != 0)
    {
    attManager->findDefinitions(mask, this->Internals->attDefs);
    }

  // Create a frame to contain all gui components for this object
  // Create a list box for the group entries
  // Create a table widget
  // Add link from the listbox selection to the table widget
  // A common add/delete/(copy/paste ??) widget

  QSplitter* frame = new QSplitter(this->parentWidget());
  //this panel looks better in a over / under layout, rather than left / right
  frame->setOrientation( Qt::Vertical );

  QFrame* topFrame = new QFrame(frame);
  QFrame* bottomFrame = new QFrame(frame);

  this->Internals->topFrame = topFrame;
  this->Internals->bottomFrame = bottomFrame;

  QVBoxLayout* leftLayout = new QVBoxLayout(topFrame);
  leftLayout->setMargin(0);
  QVBoxLayout* rightLayout = new QVBoxLayout(bottomFrame);
  rightLayout->setMargin(0);
  
  this->Internals->FiltersFrame = new QFrame(bottomFrame);
  QHBoxLayout* filterLayout = new QHBoxLayout(this->Internals->FiltersFrame);
  filterLayout->setMargin(0);
  this->Internals->ShowCategoryCombo = new QComboBox(this->Internals->FiltersFrame);

  const Manager* attMan = qtUIManager::instance()->attManager();
  std::set<std::string>::const_iterator it;
  const std::set<std::string> &cats = attMan->categories();
  for (it = cats.begin(); it != cats.end(); it++)
    {
    this->Internals->ShowCategoryCombo->addItem(it->c_str());
    }

  QLabel* labelShow = new QLabel("Show Category: ", this->Internals->FiltersFrame);
  filterLayout->addWidget(labelShow);
  filterLayout->addWidget(this->Internals->ShowCategoryCombo);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  // create a list box for all the entries
  this->Internals->ListBox = new QListWidget(topFrame);
  this->Internals->ListBox->setSelectionMode(QAbstractItemView::SingleSelection);

  this->Internals->AssociationsWidget = new qtAssociationWidget(bottomFrame);
  rightLayout->addWidget(this->Internals->FiltersFrame);
  rightLayout->addWidget(this->Internals->AssociationsWidget);

  leftLayout->addWidget(this->Internals->ListBox);

  frame->addWidget(topFrame);
  frame->addWidget(bottomFrame);

  // if there is a definition, the section should
  // display all model entities of the requested mask along
  // with the attribute of this type in a table view
  AttributeDefinitionPtr attDef = sec->definition();
  if(attDef)
    {
    this->Internals->attDefs.push_back(attDef);
    }

  // signals/slots
  QObject::connect(this->Internals->ShowCategoryCombo,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onShowCategory()));

  QObject::connect(this->Internals->ListBox,
    SIGNAL(currentItemChanged (QListWidgetItem * , QListWidgetItem * )),
    this, SLOT(onListBoxSelectionChanged(QListWidgetItem * , QListWidgetItem * )));

  this->Widget = frame;
  if(this->parentWidget()->layout())
    {
    this->parentWidget()->layout()->addWidget(frame);
    }
  this->updateModelAssociation();
}

//----------------------------------------------------------------------------
void qtModelEntitySection::updateModelAssociation()
{
  bool isRegion = this->isRegionDomain();
  this->Internals->topFrame->setVisible(!isRegion);
  if(!isRegion)
    {
    this->updateModelItems();
    }
  this->onShowCategory();
}
//----------------------------------------------------------------------------
bool qtModelEntitySection::isRegionDomain()
{
  smtk::ModelEntitySectionPtr sec =
    smtk::dynamicCastPointer<ModelEntitySection>(this->getObject());
  if(!sec)
    {
    return false;
    }

  Manager *attManager = qtUIManager::instance()->attManager();
  unsigned long mask = sec->modelEntityMask();
  if(mask & smtk::model::Item::REGION)
    {
    return true;
    }
  AttributeDefinitionPtr attDef = sec->definition();
  if(attDef && attDef->associatesWithRegion())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void qtModelEntitySection::updateModelItems()
{
  this->Internals->ListBox->blockSignals(true);
  this->Internals->ListBox->clear();

  smtk::ModelEntitySectionPtr sec =
    smtk::dynamicCastPointer<ModelEntitySection>(this->getObject());
  if(!sec)
    {
    this->Internals->ListBox->blockSignals(false);   
    return;
    }
  if(unsigned int mask = sec->modelEntityMask())
    {
    smtk::ModelPtr refModel = qtUIManager::instance()->attManager()->refModel();
    std::vector<smtk::ModelGroupItemPtr> result;
    refModel->findGroupItems(mask, result);
    std::vector<smtk::ModelGroupItemPtr>::iterator it = result.begin();
    for(; it!=result.end(); ++it)
      {
      this->addModelItem(*it);
      }
    }
  if(this->Internals->ListBox->count())
    {
    this->Internals->ListBox->setCurrentRow(0);
    }
  this->Internals->ListBox->blockSignals(false);
}

//----------------------------------------------------------------------------
void qtModelEntitySection::onShowCategory()
{
  if(this->isRegionDomain())
    {
    smtk::ModelEntitySectionPtr sec =
      smtk::dynamicCastPointer<ModelEntitySection>(this->getObject());
    unsigned int mask = sec->modelEntityMask() ? sec->modelEntityMask() :
      smtk::model::Item::REGION;
    smtk::ModelPtr refModel = qtUIManager::instance()->attManager()->refModel();
    std::vector<smtk::ModelGroupItemPtr> result;
    refModel->findGroupItems(mask, result);
    this->Internals->AssociationsWidget->showDomainsAssociation(
      result, this->Internals->ShowCategoryCombo->currentText(),
      this->Internals->attDefs);
    }
  else
    {
    smtk::ModelItemPtr theItem = this->getSelectedModelItem();
    if(theItem)
      {
      this->Internals->AssociationsWidget->showAttributeAssociation(
        theItem, this->Internals->ShowCategoryCombo->currentText(),
        this->Internals->attDefs);
      }
    }
}
//----------------------------------------------------------------------------
void qtModelEntitySection::onListBoxSelectionChanged(
  QListWidgetItem * current, QListWidgetItem * previous)
{
  this->onShowCategory();
}
//-----------------------------------------------------------------------------
smtk::ModelItemPtr qtModelEntitySection::getSelectedModelItem()
{
  return this->getModelItem(this->getSelectedItem());
}
//-----------------------------------------------------------------------------
smtk::ModelItemPtr qtModelEntitySection::getModelItem(
  QListWidgetItem * item)
{
  smtk::model::Item* rawPtr = item ? 
    static_cast<smtk::model::Item*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->pointer() : smtk::ModelItemPtr();
}
//-----------------------------------------------------------------------------
QListWidgetItem *qtModelEntitySection::getSelectedItem()
{
  return this->Internals->ListBox->currentItem();
}
//----------------------------------------------------------------------------
QListWidgetItem* qtModelEntitySection::addModelItem(
  smtk::ModelItemPtr childData)
{
  QListWidgetItem* item = new QListWidgetItem(
      QString::fromUtf8(childData->name().c_str()),
      this->Internals->ListBox, smtk_USER_DATA_TYPE);
  QVariant vdata;
  vdata.setValue((void*)(childData.get()));
  item->setData(Qt::UserRole, vdata);
//  item->setFlags(item->flags() | Qt::ItemIsEditable);
  this->Internals->ListBox->addItem(item);
  return item;
}
