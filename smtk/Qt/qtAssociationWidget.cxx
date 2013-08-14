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

#include "smtk/Qt/qtAssociationWidget.h"

#include "smtk/Qt/qtUIManager.h"
#include "smtk/Qt/qtTableWidget.h"
#include "smtk/Qt/qtAttribute.h"
#include "smtk/Qt/qtItem.h"

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

#include <QStringList>
#include <QComboBox>
#include <QVariant>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QKeyEvent>
#include <QPointer>
#include <QMessageBox>

#include <set>

#include "ui_qtAttributeAssociation.h"

namespace Ui { class qtAttributeAssociation; }

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtAssociationWidgetInternals : public Ui::qtAttributeAssociation
{
public:
  QPointer<QComboBox> NodalDropDown;
  smtk::WeakAttributePtr CurrentAtt;
  smtk::WeakModelItemPtr CurrentModelGroup;
};

//----------------------------------------------------------------------------
qtAssociationWidget::qtAssociationWidget(
  QWidget* _p): QWidget(_p)
{
  this->Internals = new qtAssociationWidgetInternals;
  this->Internals->setupUi(this);

  this->initWidget( );
  this->Internals->domainGroup->setVisible(false);
  this->Internals->boundaryGroup->setVisible(false);
}

//----------------------------------------------------------------------------
qtAssociationWidget::~qtAssociationWidget()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtAssociationWidget::initWidget( )
{
  this->Internals->NodalDropDown = new QComboBox(this);
  this->Internals->NodalDropDown->setVisible(false);
  smtk::ModelPtr refModel = qtUIManager::instance()->attManager()->refModel();
  QStringList nodalOptions;
  nodalOptions << smtk::model::Model::convertNodalTypeToString(
         smtk::model::Model::AllNodesType).c_str()
    << smtk::model::Model::convertNodalTypeToString(
         smtk::model::Model::BoundaryNodesType).c_str()
    << smtk::model::Model::convertNodalTypeToString(
         smtk::model::Model::InteriorNodesType).c_str();
  this->Internals->NodalDropDown->addItems(nodalOptions);

  this->Internals->ButtonsFrame->layout()->addWidget(
    this->Internals->NodalDropDown);

  QObject::connect(this->Internals->NodalDropDown, SIGNAL(currentIndexChanged(int)), 
    this, SLOT(onNodalOptionChanged(int)));

  // signals/slots
  QObject::connect(this->Internals->CurrentList,
    SIGNAL(currentItemChanged (QListWidgetItem * , QListWidgetItem * )),
    this, SLOT(onCurrentListSelectionChanged(QListWidgetItem * , QListWidgetItem * )));
  QObject::connect(this->Internals->AvailableList,
    SIGNAL(currentItemChanged (QListWidgetItem * , QListWidgetItem * )),
    this, SLOT(onAvailableListSelectionChanged(QListWidgetItem * , QListWidgetItem * )));

  QObject::connect(this->Internals->MoveToRight,
    SIGNAL(clicked()), this, SLOT(onRemoveAssigned()));
  QObject::connect(this->Internals->MoveToLeft,
    SIGNAL(clicked()), this, SLOT(onAddAvailable()));
  QObject::connect(this->Internals->ExchangeLeftRight,
    SIGNAL(clicked()), this, SLOT(onExchange()));

}

//----------------------------------------------------------------------------
void qtAssociationWidget::showAdvanced(int checked)
{
}

//----------------------------------------------------------------------------
void qtAssociationWidget::showDomainsAssociation(
  std::vector<smtk::ModelGroupItemPtr>& theDomains, const QString& category,
  std::vector<smtk::AttributeDefinitionPtr>& attDefs)
{
  this->Internals->domainGroup->setVisible(true);
  this->Internals->boundaryGroup->setVisible(false);

  this->Internals->CurrentAtt = smtk::AttributePtr();
  this->Internals->CurrentModelGroup = smtk::ModelItemPtr();

  this->Internals->DomainMaterialTable->blockSignals(true);
  this->Internals->DomainMaterialTable->setRowCount(0);
  this->Internals->DomainMaterialTable->clear();

  if(theDomains.size()==0 || attDefs.size()==0)
    {
    this->Internals->DomainMaterialTable->blockSignals(false);
    return;
    }

  Manager *attManager = qtUIManager::instance()->attManager();
  std::vector<smtk::AttributeDefinitionPtr>::iterator itAttDef;
  QList<QString> attNames;
  for (itAttDef=attDefs.begin(); itAttDef!=attDefs.end(); ++itAttDef)
    {
    if((*itAttDef)->isAbstract())
      {
      continue;
      }
    if((*itAttDef)->associatesWithRegion())
      {
      std::vector<smtk::AttributePtr> result;
      attManager->findAttributes(*itAttDef, result);
      std::vector<smtk::AttributePtr>::iterator itAtt;
      QString attname;
      for (itAtt=result.begin(); itAtt!=result.end(); ++itAtt)
        {
        attname = (*itAtt)->name().c_str();
        if(!attNames.contains(attname))
          {
          attNames.push_back(attname);
          }
        }
      }
    }

  std::vector<smtk::ModelGroupItemPtr>::iterator itDomain;
  for (itDomain=theDomains.begin(); itDomain!=theDomains.end(); ++itDomain)
    {
    this->addDomainListItem(*itDomain, attNames);
    }

  this->Internals->DomainMaterialTable->blockSignals(false);
}

//----------------------------------------------------------------------------
void qtAssociationWidget::showAttributeAssociation(
  smtk::ModelItemPtr theEntiy, const QString& category,
  std::vector<smtk::AttributeDefinitionPtr>& attDefs)
{
  this->Internals->domainGroup->setVisible(false);
  this->Internals->boundaryGroup->setVisible(true);

  this->Internals->CurrentAtt = smtk::AttributePtr();
  this->Internals->CurrentModelGroup = theEntiy;

  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  this->Internals->CurrentList->clear();
  this->Internals->AvailableList->clear();

  if(!theEntiy || attDefs.size()==0)
    {
    this->Internals->CurrentList->blockSignals(false);
    this->Internals->AvailableList->blockSignals(false);
    return;
    }

  Manager *attManager = qtUIManager::instance()->attManager();
  std::vector<smtk::AttributeDefinitionPtr>::iterator itAttDef;
  std::set<smtk::AttributePtr> doneAtts;
  for (itAttDef=attDefs.begin(); itAttDef!=attDefs.end(); ++itAttDef)
    {
    if((*itAttDef)->isAbstract())
      {
      continue;
      }
    std::vector<smtk::AttributePtr> result;
    attManager->findAttributes(*itAttDef, result);
    std::vector<smtk::AttributePtr>::iterator itAtt;
    for (itAtt=result.begin(); itAtt!=result.end(); ++itAtt)
      {
      if(doneAtts.find(*itAtt) != doneAtts.end())
        {
        continue;
        }
      doneAtts.insert(*itAtt);
      if(theEntiy->isAttributeAssociated(*itAtt))
        {
        this->addAttributeAssociationItem(
          this->Internals->CurrentList, *itAtt);
        }
      else
        {
        this->addAttributeAssociationItem(
          this->Internals->AvailableList, *itAtt);
        }
      }
    }

  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
}
//----------------------------------------------------------------------------
void qtAssociationWidget::showEntityAssociation( smtk::AttributePtr theAtt,
                                                 const QString& category)
{
  this->Internals->domainGroup->setVisible(false);
  this->Internals->boundaryGroup->setVisible(true);

  this->Internals->CurrentAtt = theAtt;
  this->Internals->CurrentModelGroup = smtk::ModelItemPtr();
  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  this->Internals->CurrentList->clear();
  this->Internals->AvailableList->clear();

  if(!theAtt || theAtt->definition()->associationMask()==0)
    {
    this->Internals->CurrentList->blockSignals(false);
    this->Internals->AvailableList->blockSignals(false);
    return;
    }

  AttributeDefinitionPtr attDef = theAtt->definition();
  bool isNodal = attDef->isNodal();
  this->Internals->NodalDropDown->setVisible(isNodal);
  if(isNodal)
    {
    this->Internals->NodalDropDown->blockSignals(true);
    int idx=smtk::model::Model::AllNodesType;
    if(theAtt->appliesToBoundaryNodes())
      {
      idx = smtk::model::Model::BoundaryNodesType;
      }
    else if(theAtt->appliesToInteriorNodes())
      {
      idx = smtk::model::Model::BoundaryNodesType;
      }
    this->Internals->NodalDropDown->setCurrentIndex(idx);
    this->Internals->NodalDropDown->blockSignals(false);
    }

  smtk::ModelPtr refModel = qtUIManager::instance()->attManager()->refModel();
  // add current-associated items
  int numAstItems = (int)theAtt->numberOfAssociatedEntities();
  std::set<smtk::ModelItemPtr>::const_iterator it = theAtt->associatedEntities();
  QList<int> assignedIds;
  for(int i=0; i<numAstItems; ++it, ++i)
    {
    assignedIds.append((*it)->id());
    this->addModelAssociationListItem(this->Internals->CurrentList, *it);
    }

  bool isMaterial = theAtt->definition()->associatesWithRegion();
  int numItems = (int)refModel->numberOfItems();
  std::map<int, smtk::ModelItemPtr>::const_iterator itemIt =
    refModel->itemIterator();
  for(int i=0; i<numItems; ++itemIt, ++i)
    {
    // add available, but not-associated-yet items
    if(itemIt->second->type() == smtk::model::Item::MODEL_DOMAIN)
      {
      // item for the whole model
      }
    else if(itemIt->second->type() == smtk::model::Item::GROUP)
      {
      smtk::ModelGroupItemPtr itemGroup =
        smtk::dynamicCastPointer<smtk::model::GroupItem>(itemIt->second);
      bool bRegion = itemGroup->canContain(smtk::model::Item::REGION);
      bool bFace = itemGroup->canContain(smtk::model::Item::FACE);
      bool bEdge = itemGroup->canContain(smtk::model::Item::EDGE);
      if(isMaterial == bRegion) //both true, or both false
        {
        if(!assignedIds.contains(itemIt->second->id()))
          {
          if(bRegion)
            {
            this->addModelAssociationListItem(
              this->Internals->AvailableList, itemIt->second);
            }
          else // for BCs, we need to differentiate face and/or edge assocations
            {
            if((bFace && attDef->associatesWithFace()) ||
              (bEdge && attDef->associatesWithEdge()))
              {
              this->addModelAssociationListItem(
                this->Internals->AvailableList, itemIt->second);
              }
            }
          }
        }
      }
    }
  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
}

//----------------------------------------------------------------------------
void qtAssociationWidget::onCurrentListSelectionChanged(
  QListWidgetItem * current, QListWidgetItem * previous)
{
}

//----------------------------------------------------------------------------
void qtAssociationWidget::onAvailableListSelectionChanged(
  QListWidgetItem * current, QListWidgetItem * previous)
{
}

//-----------------------------------------------------------------------------
smtk::AttributePtr qtAssociationWidget::getSelectedAttribute(
  QListWidget* theList)
{
  return this->getAttribute(this->getSelectedItem(theList));
}
//-----------------------------------------------------------------------------
smtk::AttributePtr qtAssociationWidget::getAttribute(
  QListWidgetItem * item)
{
  Attribute* rawPtr = item ? 
    static_cast<Attribute*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->pointer() : smtk::AttributePtr();
}

//-----------------------------------------------------------------------------
smtk::ModelItemPtr qtAssociationWidget::getSelectedModelItem(
  QListWidget* theList)
{
  return this->getModelItem(this->getSelectedItem(theList));
}
//-----------------------------------------------------------------------------
smtk::ModelItemPtr qtAssociationWidget::getModelItem(
  QListWidgetItem * item)
{
  smtk::model::Item* rawPtr = item ? 
    static_cast<smtk::model::Item*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->pointer() : smtk::ModelItemPtr();
}

//-----------------------------------------------------------------------------
QListWidgetItem *qtAssociationWidget::getSelectedItem(QListWidget* theList)
{
  return theList->currentItem();
}
//-----------------------------------------------------------------------------
void qtAssociationWidget::removeSelectedItem(QListWidget* theList)
{
  QListWidgetItem* selItem = this->getSelectedItem(theList);
  if(selItem)
    {
    theList->takeItem(theList->row(selItem));
    }
}
//----------------------------------------------------------------------------
QListWidgetItem* qtAssociationWidget::addModelAssociationListItem(
  QListWidget* theList, smtk::ModelItemPtr modelItem)
{
  QString txtLabel(modelItem->name().c_str());

  QListWidgetItem* item = new QListWidgetItem(txtLabel,
      theList, smtk_USER_DATA_TYPE);
  QVariant vdata;
  vdata.setValue((void*)(modelItem.get()));
  item->setData(Qt::UserRole, vdata);
  //item->setFlags(item->flags() | Qt::ItemIsEditable);
  theList->addItem(item);
  return item;
}

//----------------------------------------------------------------------------
QListWidgetItem* qtAssociationWidget::addAttributeAssociationItem(
  QListWidget* theList, smtk::AttributePtr att)
{
  QString txtLabel(att->name().c_str());

  QListWidgetItem* item = new QListWidgetItem(txtLabel,
      theList, smtk_USER_DATA_TYPE);
  QVariant vdata;
  vdata.setValue((void*)(att.get()));
  item->setData(Qt::UserRole, vdata);
  //item->setFlags(item->flags() | Qt::ItemIsEditable);
  theList->addItem(item);
  return item;
}
//----------------------------------------------------------------------------
void qtAssociationWidget::addDomainListItem(
  smtk::ModelItemPtr domainEnt, QList<QString>& attNames)
{
  QString domainName = domainEnt->name().c_str();
  QTableWidgetItem* domainItem = new QTableWidgetItem(domainName);
  domainItem->setFlags(Qt::ItemIsEnabled);
  int numRows = this->Internals->DomainMaterialTable->rowCount();
  this->Internals->DomainMaterialTable->setRowCount(++numRows);
  this->Internals->DomainMaterialTable->setItem(numRows-1, 0, domainItem);

  QComboBox* combo = new QComboBox(this);
  combo->addItems(attNames);
  int idx = -1;
  if(domainEnt->numberOfAssociatedAttributes() > 0)
    {
    std::set<smtk::AttributePtr>::const_iterator associatedAtt =
      domainEnt->associatedAttributes();
    idx = attNames.indexOf((*associatedAtt)->name().c_str());
    }
  combo->setCurrentIndex(idx);
  QObject::connect(combo, SIGNAL(currentIndexChanged(int)), 
    this, SLOT(onDomainAssociationChanged()), Qt::QueuedConnection);

  QVariant vdata;
  vdata.setValue((void*)(domainEnt.get()));
  combo->setProperty("DomainEntityObj", vdata);

  this->Internals->DomainMaterialTable->setCellWidget(numRows-1, 1, combo);
  this->Internals->DomainMaterialTable->setItem(numRows-1, 1, new QTableWidgetItem());
}

//----------------------------------------------------------------------------
void qtAssociationWidget::onRemoveAssigned()
{
  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  if(this->Internals->CurrentAtt.lock())
    {
    smtk::ModelItemPtr currentItem = this->getSelectedModelItem(
      this->Internals->CurrentList);
    if(currentItem)
      {
      this->Internals->CurrentAtt.lock()->disassociateEntity(currentItem);
      this->removeSelectedItem(this->Internals->CurrentList);
      this->addModelAssociationListItem(
        this->Internals->AvailableList, currentItem); 
      emit this->attAssociationChanged();
      }
    }
  else if(this->Internals->CurrentModelGroup.lock())
    {
    smtk::AttributePtr currentAtt = this->getSelectedAttribute(
      this->Internals->CurrentList);
    if(currentAtt)
      {
      currentAtt->disassociateEntity(
        this->Internals->CurrentModelGroup.lock());
      this->removeSelectedItem(this->Internals->CurrentList);
      this->addAttributeAssociationItem(
        this->Internals->AvailableList, currentAtt); 
      emit this->attAssociationChanged();
      }
    }

  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
}
//----------------------------------------------------------------------------
void qtAssociationWidget::onAddAvailable()
{
  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  if(this->Internals->CurrentAtt.lock())
    {
    smtk::ModelItemPtr currentItem = this->getSelectedModelItem(
      this->Internals->AvailableList);
    if(currentItem)
      {
      this->Internals->CurrentAtt.lock()->associateEntity(currentItem);
      this->removeSelectedItem(this->Internals->AvailableList);
      this->addModelAssociationListItem(
        this->Internals->CurrentList, currentItem);
      emit this->attAssociationChanged();
      }
    }
  else if(this->Internals->CurrentModelGroup.lock())
    {
    smtk::AttributePtr currentAtt = this->getSelectedAttribute(
      this->Internals->AvailableList);
    if(currentAtt)
      {
      currentAtt->associateEntity(
        this->Internals->CurrentModelGroup.lock());
      this->removeSelectedItem(this->Internals->AvailableList);
      this->addAttributeAssociationItem(
        this->Internals->CurrentList, currentAtt); 
      emit this->attAssociationChanged();
      }
    }

  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
}
//----------------------------------------------------------------------------
void qtAssociationWidget::onExchange()
{
  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  if(this->Internals->CurrentAtt.lock())
    {
    smtk::ModelItemPtr currentItem = this->getSelectedModelItem(
      this->Internals->CurrentList);
    smtk::ModelItemPtr availableItem = this->getSelectedModelItem(
      this->Internals->AvailableList);
    if(currentItem && availableItem)
      {
      this->Internals->CurrentAtt.lock()->disassociateEntity(currentItem);
      this->Internals->CurrentAtt.lock()->associateEntity(availableItem);
      this->removeSelectedItem(this->Internals->CurrentList);
      this->addModelAssociationListItem(
        this->Internals->CurrentList, availableItem);

      this->removeSelectedItem(this->Internals->AvailableList);
      this->addModelAssociationListItem(
        this->Internals->CurrentList, availableItem);
      emit this->attAssociationChanged();
      }
    }
  else if(this->Internals->CurrentModelGroup.lock())
    {
    smtk::AttributePtr availAtt = this->getSelectedAttribute(
      this->Internals->AvailableList);
    smtk::AttributePtr currentAtt = this->getSelectedAttribute(
      this->Internals->CurrentList);
    if(currentAtt && availAtt)
      {
      currentAtt->disassociateEntity(
        this->Internals->CurrentModelGroup.lock());
      this->removeSelectedItem(this->Internals->AvailableList);
      this->addAttributeAssociationItem(
        this->Internals->AvailableList, currentAtt); 

      availAtt->associateEntity(
        this->Internals->CurrentModelGroup.lock());
      this->removeSelectedItem(this->Internals->AvailableList);
      this->addAttributeAssociationItem(
        this->Internals->CurrentList, availAtt); 
      emit this->attAssociationChanged();
      }
    }

  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
}
//----------------------------------------------------------------------------
void qtAssociationWidget::onNodalOptionChanged(int idx)
{
  smtk::AttributePtr currAtt = this->Internals->CurrentAtt.lock();
  if(!currAtt)
    {
    return;
    } 
  switch(idx)
    {
    case smtk::model::Model::InteriorNodesType:
      currAtt->setAppliesToBoundaryNodes(false);
      currAtt->setAppliesToInteriorNodes(true);
      break;
    case smtk::model::Model::BoundaryNodesType:
      currAtt->setAppliesToBoundaryNodes(true);
      currAtt->setAppliesToInteriorNodes(false);
      break;
    case smtk::model::Model::AllNodesType:
      currAtt->setAppliesToBoundaryNodes(false);
      currAtt->setAppliesToInteriorNodes(false);
      break;
    default:
      break;
    }
}
//----------------------------------------------------------------------------
void qtAssociationWidget::onDomainAssociationChanged()
{
  QComboBox* const combo = qobject_cast<QComboBox*>(
    QObject::sender());
  if(!combo)
    {
    return;
    }
  smtk::model::Item* domainItem =static_cast<smtk::model::Item*>(
    combo->property("DomainEntityObj").value<void *>());
  if(!domainItem)
    {
    return;
    }
  domainItem->detachAllAttributes();
  if(combo->currentText().isEmpty())
    {
    return;
    }
  QString attName = combo->currentText();
  AttributePtr attPtr = qtUIManager::instance()->attManager()->
    findAttribute(attName.toStdString());
  if(attPtr)
    {
    domainItem->attachAttribute(attPtr);
    emit this->attAssociationChanged();
    }
  else
    {
    QString strMessage = QString("Can't find attribute with this name: ") + 
      attName;
    QMessageBox::warning(this, tr("Domain Associations"),strMessage);
    combo->setCurrentIndex(-1);
    }
}
