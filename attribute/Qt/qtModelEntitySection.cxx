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

#include "qtModelEntitySection.h"

#include "qtUIManager.h"
#include "qtTableWidget.h"
#include "qtAttribute.h"
#include "qtItem.h"

#include "attribute/ModelEntitySection.h"
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


using namespace slctk::attribute;

//----------------------------------------------------------------------------
class qtModelEntitySectionInternals
{
public:
  QListWidget* ListBox;
  QComboBox* ShowCategoryCombo;
  QFrame* FiltersFrame;
  QFrame* leftFrame;
  QFrame* rightFrame;

};

//----------------------------------------------------------------------------
qtModelEntitySection::qtModelEntitySection(
  slctk::SectionPtr dataObj, QWidget* p) : qtSection(dataObj, p)
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
  // Create a frame to contain all gui components for this object
  // Create a list box for the group entries
  // Create a table widget
  // Add link from the listbox selection to the table widget
  // A common add/delete/(copy/paste ??) widget

  QSplitter* frame = new QSplitter(this->parentWidget());
  //this panel looks better in a over / under layout, rather than left / right
  frame->setOrientation( Qt::Vertical );

  QFrame* leftFrame = new QFrame(frame);
  QFrame* rightFrame = new QFrame(frame);

  this->Internals->leftFrame = leftFrame;
  this->Internals->rightFrame = rightFrame;

  QVBoxLayout* leftLayout = new QVBoxLayout(leftFrame);
  leftLayout->setMargin(0);
  QVBoxLayout* rightLayout = new QVBoxLayout(rightFrame);
  rightLayout->setMargin(0);

  
  this->Internals->ShowCategoryCombo = new QComboBox(this->Internals->FiltersFrame);

  const Manager* attMan = qtUIManager::instance()->attManager();
  std::set<std::string>::const_iterator it;
  const std::set<std::string> &cats = attMan->categories();
  for (it = cats.begin(); it != cats.end(); it++)
    {
    this->Internals->ShowCategoryCombo->addItem(it->c_str());
    }

  this->Internals->FiltersFrame = new QFrame(frame);
  QHBoxLayout* filterLayout = new QHBoxLayout(this->Internals->FiltersFrame);
  filterLayout->setMargin(0);

  QLabel* labelShow = new QLabel("Show Category: ", this->Internals->FiltersFrame);
  filterLayout->addWidget(labelShow);
  filterLayout->addWidget(this->Internals->ShowCategoryCombo);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  // create a list box for all the entries
  this->Internals->ListBox = new QListWidget(frame);
  this->Internals->ListBox->setSelectionMode(QAbstractItemView::SingleSelection);

  leftLayout->addWidget(this->Internals->ListBox);

  frame->addWidget(leftFrame);
  frame->addWidget(rightFrame);


  slctk::ModelEntitySectionPtr sec =
    slctk::dynamicCastPointer<ModelEntitySection>(this->getObject());
  if(!sec || !sec->definition())
    {
    return;
    }

  this->Internals->ListBox->blockSignals(true);

  //AttributeDefinitionPtr attDef = sec->definition();
  //Manager *attManager = attDef->manager();
  //std::vector<slctk::AttributeDefinitionPtr> defs;
  //this->getAllDefinitions(defs);
  //std::vector<slctk::AttributeDefinitionPtr>::iterator itDef;
  //for (itDef=defs.begin(); itDef!=defs.end(); ++itDef)
  //  {
  //  std::vector<slctk::AttributePtr> result;
  //  attManager->findAttributes(*itDef, result);
  //  std::vector<slctk::AttributePtr>::iterator itAtt;
  //  for (itAtt=result.begin(); itAtt!=result.end(); ++itAtt)
  //    {
  //    this->addAttributeListItem(*itAtt);
  //    }
  //  }
  this->Internals->ListBox->blockSignals(false);

  // signals/slots
  QObject::connect(this->Internals->ShowCategoryCombo,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onShowCategory(int)));

  QObject::connect(this->Internals->ListBox,
    SIGNAL(currentItemChanged (QListWidgetItem * , QListWidgetItem * )),
    this, SLOT(onListBoxSelectionChanged(QListWidgetItem * , QListWidgetItem * )));
  QObject::connect(this->Internals->ListBox,
    SIGNAL(itemChanged (QListWidgetItem *)),
    this, SLOT(onAttributeNameChanged(QListWidgetItem * )));

  this->Widget = frame;
  if(this->parentWidget()->layout())
    {
    this->parentWidget()->layout()->addWidget(frame);
    }
  this->onShowCategory(0);
}

//----------------------------------------------------------------------------
void qtModelEntitySection::showAdvanced(int checked)
{

}
//----------------------------------------------------------------------------
void qtModelEntitySection::onShowCategory(int category)
{
  slctk::ModelEntitySectionPtr sec =
    slctk::dynamicCastPointer<ModelEntitySection>(this->getObject());
  if(!sec || !sec->definition())
    {
    return;
    }
  AttributeDefinitionPtr attDef = sec->definition();

  std::vector<slctk::AttributePtr> result;
  Manager *attManager = attDef->manager();
  attManager->findAttributes(attDef, result);

  if(result.size())
    {

    }
}
//----------------------------------------------------------------------------
void qtModelEntitySection::onListBoxSelectionChanged(
  QListWidgetItem * current, QListWidgetItem * previous)
{
}
//-----------------------------------------------------------------------------
slctk::AttributePtr qtModelEntitySection::getSelectedAttribute()
{
  return this->getAttributeFromItem(this->getSelectedItem());
}
//-----------------------------------------------------------------------------
slctk::AttributePtr qtModelEntitySection::getAttributeFromItem(
  QListWidgetItem * item)
{
  Attribute* rawPtr = item ? 
    static_cast<Attribute*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->pointer() : slctk::AttributePtr();
}
//-----------------------------------------------------------------------------
QListWidgetItem *qtModelEntitySection::getSelectedItem()
{
  return this->Internals->ListBox->selectedItems().count()>0 ?
    this->Internals->ListBox->selectedItems().value(0) : NULL;
}
//----------------------------------------------------------------------------
QListWidgetItem* qtModelEntitySection::addAttributeListItem(
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
void qtModelEntitySection::getAllDefinitions(
  QList<slctk::AttributeDefinitionPtr>& defs)
{
  slctk::ModelEntitySectionPtr sec =
    slctk::dynamicCastPointer<ModelEntitySection>(this->getObject());
  if(!sec || !sec->definition())
    {
    return;
    }

  AttributeDefinitionPtr attDef = sec->definition();
  this->qtSection::getDefinitions(attDef, defs);
}
