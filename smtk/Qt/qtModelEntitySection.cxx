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

  frame->addWidget(topFrame);
  frame->addWidget(bottomFrame);

  this->Internals->ListBox->blockSignals(true);
  std::vector<smtk::AttributeDefinitionPtr> attDefs;
  Manager *attManager = qtUIManager::instance()->attManager();

  if(unsigned long mask = sec->modelEntityMask())
    {
    attManager->findDefinitions(mask, attDefs);
    }

  // if there is a definition, the section should
  // display all model entities of the requested mask along
  // with the attribute of this type in a table view
  AttributeDefinitionPtr attDef = sec->definition();
  if(attDef)
    {
    attDefs.push_back(attDef);
    }

  std::vector<smtk::AttributeDefinitionPtr>::iterator itAttDef;
  for (itAttDef=attDefs.begin(); itAttDef!=attDefs.end(); ++itAttDef)
    {
    std::vector<smtk::AttributePtr> result;
    attManager->findAttributes(*itAttDef, result);
    std::vector<smtk::AttributePtr>::iterator itAtt;
    for (itAtt=result.begin(); itAtt!=result.end(); ++itAtt)
      {
      this->addAttributeListItem(*itAtt);
      }
    }
  this->Internals->ListBox->blockSignals(false);

  // signals/slots
  QObject::connect(this->Internals->ShowCategoryCombo,
    SIGNAL(currentIndexChanged(int)), this, SLOT(onShowCategory(int)));

  QObject::connect(this->Internals->ListBox,
    SIGNAL(currentItemChanged (QListWidgetItem * , QListWidgetItem * )),
    this, SLOT(onListBoxSelectionChanged(QListWidgetItem * , QListWidgetItem * )));

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
  smtk::ModelEntitySectionPtr sec =
    smtk::dynamicCastPointer<ModelEntitySection>(this->getObject());
  if(!sec || !sec->definition())
    {
    return;
    }
  AttributeDefinitionPtr attDef = sec->definition();

  std::vector<smtk::AttributePtr> result;
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
smtk::AttributePtr qtModelEntitySection::getSelectedAttribute()
{
  return this->getAttributeFromItem(this->getSelectedItem());
}
//-----------------------------------------------------------------------------
smtk::AttributePtr qtModelEntitySection::getAttributeFromItem(
  QListWidgetItem * item)
{
  Attribute* rawPtr = item ? 
    static_cast<Attribute*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->pointer() : smtk::AttributePtr();
}
//-----------------------------------------------------------------------------
QListWidgetItem *qtModelEntitySection::getSelectedItem()
{
  return this->Internals->ListBox->selectedItems().count()>0 ?
    this->Internals->ListBox->selectedItems().value(0) : NULL;
}
//----------------------------------------------------------------------------
QListWidgetItem* qtModelEntitySection::addAttributeListItem(
  smtk::AttributePtr childData)
{
  QListWidgetItem* item = new QListWidgetItem(
      QString::fromUtf8(childData->name().c_str()),
      this->Internals->ListBox, smtk_USER_DATA_TYPE);
  QVariant vdata;
  vdata.setValue((void*)(childData.get()));
  item->setData(Qt::UserRole, vdata);
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  this->Internals->ListBox->addItem(item);
  return item;
}
