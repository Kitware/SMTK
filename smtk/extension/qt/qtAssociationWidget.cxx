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

#include "smtk/extension/qt/qtAssociationWidget.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/extension/qt/qtEntityItemModel.h"

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

#include <algorithm>

#include "ui_qtAttributeAssociation.h"

namespace Ui { class qtAttributeAssociation; }

using namespace smtk::attribute;

namespace detail
{
  enum NodalType
  {
    AllNodesType,
    BoundaryNodesType,
    InteriorNodesType,
  };
}

//----------------------------------------------------------------------------
class qtAssociationWidgetInternals : public Ui::qtAttributeAssociation
{
public:

  QPointer<QComboBox> NodalDropDown;
  WeakAttributePtr CurrentAtt;
  smtk::common::UUID CurrentModelGroup;
  QPointer<qtBaseView> View;
};

//----------------------------------------------------------------------------
qtAssociationWidget::qtAssociationWidget(
  QWidget* _p, qtBaseView* bview): QWidget(_p)
{
  this->Internals = new qtAssociationWidgetInternals;
  this->Internals->setupUi(this);
  this->Internals->View = bview;

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
  QStringList nodalOptions;
  nodalOptions << "All Nodes" //needs to be in same order as NodalType enum
               << "Boundary Nodes"
               << "Interior Nodes";
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
void qtAssociationWidget::showDomainsAssociation(
  std::vector<smtk::model::GroupEntity>& theDomains,
  std::vector<smtk::attribute::DefinitionPtr>& attDefs)
{
  this->Internals->domainGroup->setVisible(true);
  this->Internals->boundaryGroup->setVisible(false);

  this->Internals->CurrentAtt = smtk::attribute::AttributePtr();
  this->Internals->CurrentModelGroup = smtk::common::UUID();

  this->Internals->DomainMaterialTable->blockSignals(true);
  this->Internals->DomainMaterialTable->setRowCount(0);
  this->Internals->DomainMaterialTable->clear();

  if(theDomains.size()==0 || attDefs.size()==0)
    {
    this->Internals->DomainMaterialTable->blockSignals(false);
    return;
    }

  std::vector<smtk::attribute::DefinitionPtr>::iterator itAttDef = attDefs.begin();
  Manager *attManager = (*itAttDef)->manager();
  QList<smtk::attribute::AttributePtr> allAtts;
  for (; itAttDef!=attDefs.end(); ++itAttDef)
    {
    if((*itAttDef)->isAbstract())
      {
      continue;
      }
    if((*itAttDef)->associatesWithVolume())
      {
      std::vector<smtk::attribute::AttributePtr> result;
      attManager->findAttributes(*itAttDef, result);
      std::vector<smtk::attribute::AttributePtr>::iterator itAtt;
      for (itAtt=result.begin(); itAtt!=result.end(); ++itAtt)
        {
        if(!allAtts.contains(*itAtt))
          {
          allAtts.push_back(*itAtt);
          }
        }
      }
    }

  std::vector<smtk::model::GroupEntity>::iterator itDomain;
  for (itDomain=theDomains.begin(); itDomain!=theDomains.end(); ++itDomain)
    {
    this->addDomainListItem(*itDomain, allAtts);
    }

  this->Internals->DomainMaterialTable->blockSignals(false);
}

//----------------------------------------------------------------------------
void qtAssociationWidget::showAttributeAssociation(
  smtk::model::ModelEntity theEntiy,
  std::vector<smtk::attribute::DefinitionPtr>& attDefs)
{
  this->Internals->domainGroup->setVisible(false);
  this->Internals->boundaryGroup->setVisible(true);

  this->Internals->CurrentAtt = smtk::attribute::AttributePtr();
  this->Internals->CurrentModelGroup = theEntiy.entity();

  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  this->Internals->CurrentList->clear();
  this->Internals->AvailableList->clear();

  if( !theEntiy.isValid() || attDefs.size()==0)
    {
    this->Internals->CurrentList->blockSignals(false);
    this->Internals->AvailableList->blockSignals(false);
    return;
    }

  std::vector<smtk::attribute::DefinitionPtr>::iterator itAttDef = attDefs.begin();
  Manager *attManager = (*itAttDef)->manager();

  // figure out how many unique definitions this model item has
  QList<smtk::attribute::DefinitionPtr> uniqueDefs =  this->processDefUniqueness(theEntiy,
                                                                                 attManager);


  std::set<smtk::attribute::AttributePtr> doneAtts;
  for (; itAttDef!=attDefs.end(); ++itAttDef)
    {
    if((*itAttDef)->isAbstract())
      {
      continue;
      }
    std::vector<smtk::attribute::AttributePtr> result;
    attManager->findAttributes(*itAttDef, result);
    std::vector<smtk::attribute::AttributePtr>::iterator itAtt;
    for (itAtt=result.begin(); itAtt!=result.end(); ++itAtt)
      {
      if(doneAtts.find(*itAtt) != doneAtts.end())
        {
        continue;
        }
      doneAtts.insert(*itAtt);

      if( theEntiy.hasAttribute( (*itAtt)->id() ) )
        {
        this->addAttributeAssociationItem(
          this->Internals->CurrentList, *itAtt);
        }
      else if(!uniqueDefs.contains(*itAttDef))
        {
        // we need to make sure this att is not associated with other
        // same type model entities.
        this->addAttributeAssociationItem(
          this->Internals->AvailableList, *itAtt);
        }
      }
    }

  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
}
//----------------------------------------------------------------------------
void qtAssociationWidget::showEntityAssociation(
  smtk::attribute::AttributePtr theAtt)
{
  this->Internals->domainGroup->setVisible(false);
  this->Internals->boundaryGroup->setVisible(true);

  this->Internals->CurrentAtt = theAtt;
  this->Internals->CurrentModelGroup = smtk::common::UUID();

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

  attribute::DefinitionPtr attDef = theAtt->definition();
  bool isNodal = attDef->isNodal();
  this->Internals->NodalDropDown->setVisible(isNodal);
  if(isNodal)
    {
    this->Internals->NodalDropDown->blockSignals(true);
    int idx = detail::AllNodesType;
    if(theAtt->appliesToBoundaryNodes())
      {
      idx = detail::BoundaryNodesType;
      }
    else if(theAtt->appliesToInteriorNodes())
      {
      idx = detail::InteriorNodesType;
      }
    this->Internals->NodalDropDown->setCurrentIndex(idx);
    this->Internals->NodalDropDown->blockSignals(false);
    }


  //add current-associated items to the current attribute
  smtk::model::ManagerPtr modelManager = attDef->manager()->refModelManager();
  smtk::model::ModelEntities modelEnts;
  smtk::model::Cursor::CursorsFromUUIDs(
    modelEnts,
    modelManager,
    theAtt->associatedModelEntityIds());

  typedef smtk::model::ModelEntities::const_iterator cit;
  for (cit i =modelEnts.begin(); i != modelEnts.end(); ++i)
    {
    this->addModelAssociationListItem(this->Internals->CurrentList, *i);
    }
  std::set<smtk::model::ModelEntity> usedModelEnts = this->processAttUniqueness(attDef, modelEnts);

  //now that we have add all the used model entities, we need to move onto
  //all model entities that havent been matched
  const smtk::model::BitFlags emask = smtk::model::MODEL_ENTITY;
  smtk::model::Cursors cursors;
  smtk::model::Cursor::CursorsFromUUIDs(
    cursors,
    modelManager,
    modelManager->entitiesMatchingFlags(emask));

  smtk::model::EntityListPhrasePtr entityList = smtk::model::EntityListPhrase::create();
  entityList->setup(cursors);
  //set the subphrase generator:
  entityList->setDelegate(smtk::model::SimpleModelSubphrases::create());

  //walk the tree getting all entities in the tree in a brute force manner
  smtk::model::DescriptivePhrases subs = entityList->subphrases();
  smtk::model::Cursors allEntities;
  while(subs.size() > 0)
    {
    smtk::model::DescriptivePhrases nested_subs = subs[subs.size()-1]->subphrases();
    subs.insert(subs.begin(),nested_subs.begin(),nested_subs.end());
    allEntities.insert(subs[0]->relatedEntity());
    subs.pop_back();
    }

  typedef smtk::model::Cursors::const_iterator me_it;
  for(me_it i = allEntities.begin(); i != allEntities.end(); ++i)
    {
      bool match = false;
      if(attDef->associatesWithVertex() && i->isVertex())
        { match = true; }
      if(attDef->associatesWithEdge() && i->isEdge())
        { match = true; }
      if(attDef->associatesWithFace() && i->isFace())
        { match = true; }
      if(attDef->associatesWithVolume() && i->isVolume())
        { match = true; }

      //only add this item if we haven't seen it already
      if(match && usedModelEnts.count(*i) == 0)
        {
        this->addModelAssociationListItem(this->Internals->AvailableList, *i);
        }
    }

  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
}

//----------------------------------------------------------------------------
std::set<smtk::model::ModelEntity> qtAssociationWidget::processAttUniqueness(
                                            smtk::attribute::DefinitionPtr attDef,
                                            const smtk::model::ModelEntities &assignedIds)
{
  std::set<smtk::model::ModelEntity> allUsedModelIds(assignedIds.begin(),
                                                     assignedIds.end());
  if(attDef->isUnique())
    {
    // we need to exclude any entities that are already assigned another att
    // Get the most "basic" definition that is unique
    Manager *attManager = attDef->manager();
    smtk::model::ManagerPtr modelManager = attManager->refModelManager();

    smtk::attribute::ConstDefinitionPtr baseDef =
      attManager->findIsUniqueBaseClass(attDef);
    smtk::attribute::DefinitionPtr bdef(smtk::const_pointer_cast<Definition>(baseDef));
    std::vector<smtk::attribute::DefinitionPtr> newdefs;
    attManager->findAllDerivedDefinitions(bdef, true, newdefs);
    std::vector<smtk::attribute::DefinitionPtr>::iterator itDef;
    for (itDef=newdefs.begin(); itDef!=newdefs.end(); ++itDef)
      {
      std::vector<smtk::attribute::AttributePtr> result;
      attManager->findAttributes(*itDef, result);
      std::vector<smtk::attribute::AttributePtr>::iterator itAtt;
      for (itAtt=result.begin(); itAtt!=result.end(); ++itAtt)
        {
        smtk::model::ModelEntities modelEnts;
        smtk::model::Cursor::CursorsFromUUIDs(
          modelEnts,
          modelManager,
          (*itAtt)->associatedModelEntityIds());

        typedef smtk::model::ModelEntities::const_iterator cit;
        for (cit i = modelEnts.begin(); i != modelEnts.end(); ++i)
          { //add them all, and let the set sort it out
          allUsedModelIds.insert(*i);
          }
        }
      }
    }
  return allUsedModelIds;
}

//----------------------------------------------------------------------------
QList<smtk::attribute::DefinitionPtr>
qtAssociationWidget::processDefUniqueness(const smtk::model::ModelEntity& theEntity,
                                          smtk::attribute::Manager* attManager)
{
  QList<smtk::attribute::DefinitionPtr> uniqueDefs;

  smtk::model::AttributeSet associatedAtts = theEntity.attributes();
  if(associatedAtts.size() == 0)
    {
    return uniqueDefs;
    }

  typedef smtk::model::AttributeSet::const_iterator cit;
  for (cit i = associatedAtts.begin(); i != associatedAtts.end(); ++i)
    {
    smtk::attribute::AttributePtr attPtr = attManager->findAttribute( (*i) );
    if(attPtr)
      {
      smtk::attribute::DefinitionPtr attDef = attPtr->definition();
      if(attDef->isUnique())
        {
        Manager *attManager = attDef->manager();
        smtk::attribute::ConstDefinitionPtr baseDef =
          attManager->findIsUniqueBaseClass(attDef);
        smtk::attribute::DefinitionPtr bdef(smtk::const_pointer_cast<Definition>(baseDef));
        std::vector<smtk::attribute::DefinitionPtr> newdefs;
        attManager->findAllDerivedDefinitions(bdef, true, newdefs);
        std::vector<smtk::attribute::DefinitionPtr>::iterator itDef;
        for (itDef=newdefs.begin(); itDef!=newdefs.end(); ++itDef)
          {
          uniqueDefs.append(*itDef);
          }
        }
      }
    }


  return uniqueDefs;
}


//----------------------------------------------------------------------------
void qtAssociationWidget::onCurrentListSelectionChanged(
  QListWidgetItem * /*current*/, QListWidgetItem * /*previous*/)
{
}

//----------------------------------------------------------------------------
void qtAssociationWidget::onAvailableListSelectionChanged(
  QListWidgetItem * /*current*/, QListWidgetItem * /*previous*/)
{
}

//-----------------------------------------------------------------------------
smtk::attribute::AttributePtr qtAssociationWidget::getSelectedAttribute(
  QListWidget* theList)
{
  return this->getAttribute(this->getSelectedItem(theList));
}

//-----------------------------------------------------------------------------
smtk::attribute::AttributePtr qtAssociationWidget::getAttribute(
  QListWidgetItem * item)
{
  Attribute* rawPtr = item ?
    static_cast<Attribute*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->pointer() : smtk::attribute::AttributePtr();
}

//-----------------------------------------------------------------------------
smtk::model::ModelEntity qtAssociationWidget::getSelectedModelItem(
  QListWidget* theList)
{
  return this->getModelItem(this->getSelectedItem(theList));
}

//-----------------------------------------------------------------------------
smtk::model::ModelEntity qtAssociationWidget::getModelItem(
  QListWidgetItem * item)
{
  if(item)
    {
    QVariant var = item->data(Qt::UserRole);
    smtk::common::UUID uid( var.toString().toStdString() );
    smtk::attribute::Manager *attManager = this->Internals->View->uiManager()->attManager();
    if(attManager)
      {
      smtk::model::ManagerPtr modelManager = attManager->refModelManager();
      return smtk::model::ModelEntity(modelManager,uid);
      }
    }
  return smtk::model::ModelEntity();
}

//-----------------------------------------------------------------------------
QListWidgetItem *qtAssociationWidget::getSelectedItem(QListWidget* theList)
{
  if(theList->count() == 1)
    {
    return theList->item(0);
    }
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
  QListWidget* theList, smtk::model::ModelEntity modelItem)
{
  QListWidgetItem* item = new QListWidgetItem(
                            QString::fromStdString(modelItem.name()),
                            theList,
                            smtk_USER_DATA_TYPE);
  //save the entity as a uuid string
  QVariant vdata( QString::fromStdString(modelItem.entity().toString()) );
  item->setData(Qt::UserRole, vdata);
  theList->addItem(item);
  return item;
}

//----------------------------------------------------------------------------
QListWidgetItem* qtAssociationWidget::addAttributeAssociationItem(
  QListWidget* theList, smtk::attribute::AttributePtr att)
{
  QString txtLabel(att->name().c_str());

  QListWidgetItem* item = new QListWidgetItem(txtLabel,
      theList, smtk_USER_DATA_TYPE);
  QVariant vdata;
  vdata.setValue(static_cast<void*>(att.get()));
  item->setData(Qt::UserRole, vdata);
  theList->addItem(item);
  return item;
}

//----------------------------------------------------------------------------
void qtAssociationWidget::addDomainListItem(
  const smtk::model::GroupEntity& domainEnt, QList<smtk::attribute::AttributePtr>& allAtts)
{
  smtk::attribute::Manager *attManager = this->Internals->View->uiManager()->attManager();

  QTableWidgetItem* domainItem = new QTableWidgetItem(  QString::fromStdString(domainEnt.name()) );
  domainItem->setFlags(Qt::ItemIsEnabled);

  int numRows = this->Internals->DomainMaterialTable->rowCount();
  this->Internals->DomainMaterialTable->setRowCount(++numRows);
  this->Internals->DomainMaterialTable->setItem(numRows-1, 0, domainItem);

  QList<QString> attNames;
  QString attname;
  foreach (smtk::attribute::AttributePtr att, allAtts)
    {
    attname = att->name().c_str();
    if(!attNames.contains(attname))
      {
      attNames.push_back(attname);
      }
    }

  QComboBox* combo = new QComboBox(this);
  combo->addItems(attNames);
  int idx = -1;
  smtk::model::AttributeSet associatedAtts = domainEnt.attributes();
  if(associatedAtts.size() > 0)
    {
    smtk::attribute::AttributePtr first_att = attManager->findAttribute( (*associatedAtts.begin()) );
    if(first_att)
      {
      idx = attNames.indexOf(QString::fromStdString(first_att->name()));
      }
    }
  combo->setCurrentIndex(idx);
  QObject::connect(combo, SIGNAL(currentIndexChanged(int)),
    this, SLOT(onDomainAssociationChanged()), Qt::QueuedConnection);

  QVariant vdata( QString::fromStdString( domainEnt.entity().toString() ) );
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
    smtk::model::ModelEntity currentItem = this->getSelectedModelItem(
      this->Internals->CurrentList);
    if(!currentItem.entity().isNull())
      {
      this->Internals->CurrentAtt.lock()->disassociateEntity(currentItem.entity());
      this->removeSelectedItem(this->Internals->CurrentList);
      this->addModelAssociationListItem(
        this->Internals->AvailableList, currentItem);
      emit this->attAssociationChanged();
      }
    }
  else if(!this->Internals->CurrentModelGroup.isNull())
    {
    smtk::attribute::AttributePtr currentAtt = this->getSelectedAttribute(
      this->Internals->CurrentList);
    if(currentAtt)
      {
      currentAtt->disassociateEntity(this->Internals->CurrentModelGroup);
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
    smtk::model::ModelEntity currentItem = this->getSelectedModelItem(
      this->Internals->AvailableList);
    if(!currentItem.entity().isNull())
      {
      this->Internals->CurrentAtt.lock()->associateEntity(currentItem.entity());
      this->removeSelectedItem(this->Internals->AvailableList);
      this->addModelAssociationListItem(
        this->Internals->CurrentList, currentItem);
      emit this->attAssociationChanged();
      }
    }
  else if(!this->Internals->CurrentModelGroup.isNull())
    {
    smtk::attribute::AttributePtr currentAtt = this->getSelectedAttribute(
      this->Internals->AvailableList);
    if(currentAtt)
      {
      if(currentAtt->definition()->isUnique() &&
        this->Internals->CurrentList->count())
        {
        this->onExchange();
        return;
        }
      currentAtt->associateEntity(this->Internals->CurrentModelGroup);
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
    smtk::model::ModelEntity currentItem = this->getSelectedModelItem(
      this->Internals->CurrentList);
    smtk::model::ModelEntity availableItem = this->getSelectedModelItem(
      this->Internals->AvailableList);
    if(!currentItem.entity().isNull() && availableItem.isValid())
      {
      this->Internals->CurrentAtt.lock()->disassociateEntity(currentItem.entity());
      this->Internals->CurrentAtt.lock()->associateEntity(availableItem.entity());
      this->removeSelectedItem(this->Internals->CurrentList);
      this->addModelAssociationListItem(
        this->Internals->CurrentList, availableItem);

      this->removeSelectedItem(this->Internals->AvailableList);
      this->addModelAssociationListItem(
        this->Internals->AvailableList, currentItem);
      emit this->attAssociationChanged();
      }
    }
  else if(!this->Internals->CurrentModelGroup.isNull())
    {
    smtk::attribute::AttributePtr availAtt = this->getSelectedAttribute(
      this->Internals->AvailableList);
    smtk::attribute::AttributePtr currentAtt = this->getSelectedAttribute(
      this->Internals->CurrentList);
    if(availAtt &&currentAtt)
      {
      currentAtt->disassociateEntity(this->Internals->CurrentModelGroup);
      this->addAttributeAssociationItem(
        this->Internals->AvailableList, currentAtt);
      this->removeSelectedItem(this->Internals->CurrentList);

      availAtt->associateEntity(this->Internals->CurrentModelGroup);
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
  smtk::attribute::AttributePtr currAtt = this->Internals->CurrentAtt.lock();
  if(!currAtt)
    {
    return;
    }
  switch(idx)
    {
    case detail::InteriorNodesType:
      currAtt->setAppliesToBoundaryNodes(false);
      currAtt->setAppliesToInteriorNodes(true);
      break;
    case detail::BoundaryNodesType:
      currAtt->setAppliesToBoundaryNodes(true);
      currAtt->setAppliesToInteriorNodes(false);
      break;
    case detail::AllNodesType:
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
  smtk::attribute::Manager *attManager = this->Internals->View->uiManager()->attManager();
  smtk::model::ManagerPtr modelManager = attManager->refModelManager();

  QComboBox* const combo = qobject_cast<QComboBox*>(
    QObject::sender());
  if(!combo)
    {
    return;
    }
  QString domainStr = combo->property("DomainEntityObj").toString();
  smtk::common::UUID uid( domainStr.toStdString() );
  smtk::model::GroupEntity domainItem(modelManager,uid);
  if(!domainItem.isValid())
    {
    return;
    }

  smtk::model::AttributeAssignments atts = domainItem.attributes();
  atts.attributes().clear(); //detach all attributes

  if(combo->currentText().isEmpty())
    {
    return;
    }


  QString attName = combo->currentText();
  AttributePtr attPtr = attManager->findAttribute(attName.toStdString());
  if(attPtr)
    {
    domainItem.attachAttribute(attPtr->id());
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
