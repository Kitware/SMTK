//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAssociationWidget.h"
#include "smtk/extension/qt/qtActiveObjects.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>
#include <QVariant>

#include <algorithm>
#include <sstream>

#include "ui_qtAttributeAssociation.h"

namespace Ui
{
class qtAttributeAssociation;
}

using namespace smtk::attribute;
using namespace smtk::extension;
using smtk::model::EntityRef;
using smtk::model::EntityRefs;
using smtk::model::Group;

namespace detail
{
enum NodalType
{
  AllNodesType,
  BoundaryNodesType,
  InteriorNodesType,
};
}

class qtAssociationWidgetInternals : public Ui::qtAttributeAssociation
{
public:
  QPointer<QComboBox> NodalDropDown;
  WeakAttributePtr CurrentAtt;
  smtk::common::UUID CurrentModelGroup;
  QPointer<qtBaseView> View;
};

qtAssociationWidget::qtAssociationWidget(QWidget* _p, qtBaseView* bview)
  : QWidget(_p)
{
  this->Internals = new qtAssociationWidgetInternals;
  this->Internals->setupUi(this);
  this->Internals->View = bview;

  this->initWidget();
  this->Internals->domainGroup->setVisible(false);
  this->Internals->boundaryGroup->setVisible(false);
  std::ostringstream receiverSource;
  receiverSource << "qtAssociationWidget_" << this;
  m_selectionSourceName = receiverSource.str();
}

qtAssociationWidget::~qtAssociationWidget()
{
  delete this->Internals;
}

void qtAssociationWidget::initWidget()
{
  this->Internals->NodalDropDown = new QComboBox(this);
  this->Internals->NodalDropDown->setVisible(false);
  QStringList nodalOptions;
  nodalOptions << "All Nodes" //needs to be in same order as NodalType enum
               << "Boundary Nodes"
               << "Interior Nodes";
  this->Internals->NodalDropDown->addItems(nodalOptions);

  this->Internals->ButtonsFrame->layout()->addWidget(this->Internals->NodalDropDown);

  QObject::connect(this->Internals->NodalDropDown, SIGNAL(currentIndexChanged(int)), this,
    SLOT(onNodalOptionChanged(int)));

  // signals/slots
  QObject::connect(this->Internals->MoveToRight, SIGNAL(clicked()), this, SLOT(onRemoveAssigned()));
  QObject::connect(this->Internals->MoveToLeft, SIGNAL(clicked()), this, SLOT(onAddAvailable()));
  QObject::connect(this->Internals->ExchangeLeftRight, SIGNAL(clicked()), this, SLOT(onExchange()));
}

void qtAssociationWidget::showDomainsAssociation(
  std::vector<smtk::model::Group>& theDomains, std::vector<smtk::attribute::DefinitionPtr>& attDefs)
{
  this->Internals->domainGroup->setVisible(true);
  this->Internals->boundaryGroup->setVisible(false);

  this->Internals->CurrentAtt = smtk::attribute::AttributePtr();
  this->Internals->CurrentModelGroup = smtk::common::UUID();

  this->Internals->DomainMaterialTable->blockSignals(true);
  this->Internals->DomainMaterialTable->setRowCount(0);
  this->Internals->DomainMaterialTable->clear();

  if (theDomains.size() == 0 || attDefs.size() == 0)
  {
    this->Internals->DomainMaterialTable->blockSignals(false);
    return;
  }

  std::vector<smtk::attribute::DefinitionPtr>::iterator itAttDef = attDefs.begin();
  ResourcePtr attResource = (*itAttDef)->resource();
  QList<smtk::attribute::AttributePtr> allAtts;
  for (; itAttDef != attDefs.end(); ++itAttDef)
  {
    if ((*itAttDef)->isAbstract())
    {
      continue;
    }
    if ((*itAttDef)->associatesWithVolume())
    {
      std::vector<smtk::attribute::AttributePtr> result;
      attResource->findAttributes(*itAttDef, result);
      std::vector<smtk::attribute::AttributePtr>::iterator itAtt;
      for (itAtt = result.begin(); itAtt != result.end(); ++itAtt)
      {
        if (!allAtts.contains(*itAtt))
        {
          allAtts.push_back(*itAtt);
        }
      }
    }
  }

  std::vector<smtk::model::Group>::iterator itDomain;
  for (itDomain = theDomains.begin(); itDomain != theDomains.end(); ++itDomain)
  {
    this->addDomainListItem(*itDomain, allAtts);
  }

  this->Internals->DomainMaterialTable->blockSignals(false);
}

bool qtAssociationWidget::hasSelectedItem()
{
  return this->Internals->AvailableList->selectedItems().isEmpty() ? false : true;
}

void qtAssociationWidget::showAttributeAssociation(
  smtk::model::EntityRef theEntiy, std::vector<smtk::attribute::DefinitionPtr>& attDefs)
{
  this->Internals->domainGroup->setVisible(false);
  this->Internals->boundaryGroup->setVisible(true);

  this->Internals->CurrentAtt = smtk::attribute::AttributePtr();
  this->Internals->CurrentModelGroup = theEntiy.entity();

  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  this->Internals->CurrentList->clear();
  this->Internals->AvailableList->clear();

  if (!theEntiy.isValid() || attDefs.size() == 0)
  {
    this->Internals->CurrentList->blockSignals(false);
    this->Internals->AvailableList->blockSignals(false);
    return;
  }

  std::vector<smtk::attribute::DefinitionPtr>::iterator itAttDef = attDefs.begin();
  ResourcePtr attResource = (*itAttDef)->resource();

  // figure out how many unique definitions this model item has
  QList<smtk::attribute::DefinitionPtr> uniqueDefs =
    this->processDefUniqueness(theEntiy, attResource);

  std::set<smtk::attribute::AttributePtr> doneAtts;
  for (; itAttDef != attDefs.end(); ++itAttDef)
  {
    if ((*itAttDef)->isAbstract())
    {
      continue;
    }
    std::vector<smtk::attribute::AttributePtr> result;
    attResource->findAttributes(*itAttDef, result);
    std::vector<smtk::attribute::AttributePtr>::iterator itAtt;
    for (itAtt = result.begin(); itAtt != result.end(); ++itAtt)
    {
      if (doneAtts.find(*itAtt) != doneAtts.end())
      {
        continue;
      }
      doneAtts.insert(*itAtt);

      if (theEntiy.hasAttribute((*itAtt)->id()))
      {
        this->addAttributeAssociationItem(this->Internals->CurrentList, *itAtt, false);
      }
      else if (!uniqueDefs.contains(*itAttDef))
      {
        // we need to make sure this att is not associated with other
        // same type model entities.
        this->addAttributeAssociationItem(this->Internals->AvailableList, *itAtt, false);
      }
    }
  }

  this->Internals->CurrentList->sortItems();
  this->Internals->AvailableList->sortItems();
  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
}

void qtAssociationWidget::showEntityAssociation(smtk::attribute::AttributePtr theAtt)
{
  this->Internals->domainGroup->setVisible(false);
  this->Internals->boundaryGroup->setVisible(true);

  this->Internals->CurrentAtt = theAtt;
  this->Internals->CurrentModelGroup = smtk::common::UUID();

  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  this->Internals->CurrentList->clear();
  this->Internals->AvailableList->clear();

  if (!theAtt || theAtt->definition()->associationMask() == 0)
  {
    this->Internals->CurrentList->blockSignals(false);
    this->Internals->AvailableList->blockSignals(false);
    return;
  }

  attribute::DefinitionPtr attDef = theAtt->definition();
  bool isNodal = attDef->isNodal();
  this->Internals->NodalDropDown->setVisible(isNodal);
  if (isNodal)
  {
    this->Internals->NodalDropDown->blockSignals(true);
    int idx = detail::AllNodesType;
    if (theAtt->appliesToBoundaryNodes())
    {
      idx = detail::BoundaryNodesType;
    }
    else if (theAtt->appliesToInteriorNodes())
    {
      idx = detail::InteriorNodesType;
    }
    this->Internals->NodalDropDown->setCurrentIndex(idx);
    this->Internals->NodalDropDown->blockSignals(false);
  }

  // Add currently-associated items to the list displaying associations.
  smtk::model::ManagerPtr modelManager = attDef->resource()->refModelManager();

  if (!modelManager)
  {
    return;
  }

  smtk::model::EntityRefs modelEnts = theAtt->associatedModelEntities<smtk::model::EntityRefs>();

  typedef smtk::model::EntityRefs::const_iterator cit;
  for (cit i = modelEnts.begin(); i != modelEnts.end(); ++i)
  {
    this->addModelAssociationListItem(this->Internals->CurrentList, *i, false, true);
  }
  this->Internals->CurrentList->sortItems();
  // The returned usedModelEnts should include all of the input modelEnts AND, if attDef is unique,
  // entities that have been associated with the attributes with same definition.
  std::set<smtk::model::EntityRef> usedModelEnts = this->processAttUniqueness(attDef, modelEnts);

  // Now that we have added all the used model entities, we need to move on to all model
  // entities that are not associated with the attribute, but could be associated.
  // We use the "no-exact match required" flag to catch any entity that could possibly match
  // the association mask. This gets pruned below.
  smtk::model::EntityRefs entityrefs =
    modelManager->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(
      attDef->associationMask(), false);

  EntityRefs avail;
  // So the available list should be the difference between all "entityrefs" satisfying the mask
  // and the usedModelEnts that have been associated.
  // Subtract the set of already-associated entities.
  std::set_difference(entityrefs.begin(), entityrefs.end(), usedModelEnts.begin(),
    usedModelEnts.end(), std::inserter(avail, avail.end()));

  // Add a subset of the remainder which also belongs to current active model
  // to the list of available entities.
  // We create a temporary group and use Group::meetsMembershipConstraints()
  // to test whether the mask allows association.
  smtk::model::Model activeModel = qtActiveObjects::instance().activeModel();
  smtk::model::Manager::Ptr tmpMgr = smtk::model::Manager::create();
  Group tmpGrp = tmpMgr->addGroup();
  tmpGrp.setMembershipMask(attDef->associationMask());

  for (EntityRefs::iterator i = avail.begin(); i != avail.end(); ++i)
  {
    if (tmpGrp.meetsMembershipConstraints(*i) && i->owningModel().entity() == activeModel.entity())
    {
      this->addModelAssociationListItem(this->Internals->AvailableList, *i, false);
    }
  }
  this->Internals->AvailableList->sortItems();
  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
}

std::set<smtk::model::EntityRef> qtAssociationWidget::processAttUniqueness(
  smtk::attribute::DefinitionPtr attDef, const smtk::model::EntityRefs& assignedIds)
{
  std::set<smtk::model::EntityRef> allUsedModelIds(assignedIds.begin(), assignedIds.end());
  if (attDef->isUnique())
  {
    // we need to exclude any entities that are already assigned another att
    // Get the most "basic" definition that is unique
    ResourcePtr attResource = attDef->resource();
    smtk::model::ManagerPtr modelManager = attResource->refModelManager();

    smtk::attribute::ConstDefinitionPtr baseDef = attResource->findIsUniqueBaseClass(attDef);
    smtk::attribute::DefinitionPtr bdef(smtk::const_pointer_cast<Definition>(baseDef));
    std::vector<smtk::attribute::DefinitionPtr> newdefs;
    attResource->findAllDerivedDefinitions(bdef, true, newdefs);
    std::vector<smtk::attribute::DefinitionPtr>::iterator itDef;
    for (itDef = newdefs.begin(); itDef != newdefs.end(); ++itDef)
    {
      std::vector<smtk::attribute::AttributePtr> result;
      attResource->findAttributes(*itDef, result);
      std::vector<smtk::attribute::AttributePtr>::iterator itAtt;
      for (itAtt = result.begin(); itAtt != result.end(); ++itAtt)
      {
        smtk::model::EntityRefs modelEnts;
        smtk::model::EntityRef::EntityRefsFromUUIDs(
          modelEnts, modelManager, (*itAtt)->associatedModelEntityIds());

        typedef smtk::model::EntityRefs::const_iterator cit;
        for (cit i = modelEnts.begin(); i != modelEnts.end(); ++i)
        { //add them all, and let the set sort it out
          allUsedModelIds.insert(*i);
        }
      }
    }
  }
  return allUsedModelIds;
}

QList<smtk::attribute::DefinitionPtr> qtAssociationWidget::processDefUniqueness(
  const smtk::model::EntityRef& theEntity, smtk::attribute::ResourcePtr attResource)
{
  QList<smtk::attribute::DefinitionPtr> uniqueDefs;

  smtk::common::UUIDs associatedAtts = theEntity.attributeIds();
  if (associatedAtts.size() == 0)
  {
    return uniqueDefs;
  }

  typedef smtk::common::UUIDs::const_iterator cit;
  for (cit i = associatedAtts.begin(); i != associatedAtts.end(); ++i)
  {
    smtk::attribute::AttributePtr attPtr = attResource->findAttribute((*i));
    if (attPtr)
    {
      smtk::attribute::DefinitionPtr attDef = attPtr->definition();
      if (attDef->isUnique())
      {
        smtk::attribute::ConstDefinitionPtr baseDef = attResource->findIsUniqueBaseClass(attDef);
        smtk::attribute::DefinitionPtr bdef(smtk::const_pointer_cast<Definition>(baseDef));
        std::vector<smtk::attribute::DefinitionPtr> newdefs;
        attResource->findAllDerivedDefinitions(bdef, true, newdefs);
        std::vector<smtk::attribute::DefinitionPtr>::iterator itDef;
        for (itDef = newdefs.begin(); itDef != newdefs.end(); ++itDef)
        {
          uniqueDefs.append(*itDef);
        }
      }
    }
  }

  return uniqueDefs;
}

void qtAssociationWidget::onEntitySelected()
{
  QListWidget* const listW = qobject_cast<QListWidget*>(QObject::sender());
  if (!listW)
  {
    return;
  }

  smtk::model::EntityRefs selents;
  QList<QListWidgetItem*> selItems = listW->selectedItems();
  foreach (QListWidgetItem* currentItem, selItems)
  {
    smtk::model::EntityRef entref = this->getModelEntityItem(currentItem);
    if (entref.isValid())
    {
      selents.insert(entref);
    }
  }
}

smtk::attribute::AttributePtr qtAssociationWidget::getSelectedAttribute(QListWidgetItem* item)
{
  return this->getAttribute(item);
}

smtk::attribute::AttributePtr qtAssociationWidget::getAttribute(QListWidgetItem* item)
{
  Attribute* rawPtr =
    item ? static_cast<Attribute*>(item->data(Qt::UserRole).value<void*>()) : NULL;
  return rawPtr ? rawPtr->shared_from_this() : smtk::attribute::AttributePtr();
}

smtk::model::EntityRef qtAssociationWidget::getSelectedModelEntityItem(QListWidgetItem* item)
{
  return this->getModelEntityItem(item);
}

smtk::model::EntityRef qtAssociationWidget::getModelEntityItem(QListWidgetItem* item)
{
  if (item)
  {
    QVariant var = item->data(Qt::UserRole);
    smtk::common::UUID uid(var.toString().toStdString());
    smtk::attribute::ResourcePtr attResource = this->Internals->View->uiManager()->attResource();
    if (attResource)
    {
      smtk::model::ManagerPtr modelManager = attResource->refModelManager();
      return smtk::model::EntityRef(modelManager, uid);
    }
  }
  return smtk::model::EntityRef();
}

QList<QListWidgetItem*> qtAssociationWidget::getSelectedItems(QListWidget* theList) const
{
  if (theList->selectedItems().count())
  {
    return theList->selectedItems();
  }
  QList<QListWidgetItem*> result;
  if (theList->count() == 1)
  {
    result.push_back(theList->item(0));
  }
  return result;
}

void qtAssociationWidget::removeItem(QListWidget* theList, QListWidgetItem* selItem)
{
  if (theList && selItem)
  {
    theList->takeItem(theList->row(selItem));
  }
}

QListWidgetItem* qtAssociationWidget::addModelAssociationListItem(
  QListWidget* theList, smtk::model::EntityRef modelItem, bool sort, bool appendModelName)
{
  std::string name = appendModelName ? (modelItem.name() + " - " + modelItem.owningModel().name())
                                     : modelItem.name();
  QListWidgetItem* item =
    new QListWidgetItem(QString::fromStdString(name), theList, smtk_USER_DATA_TYPE);
  //save the entity as a uuid string
  QVariant vdata(QString::fromStdString(modelItem.entity().toString()));
  item->setData(Qt::UserRole, vdata);
  theList->addItem(item);
  if (sort)
  {
    theList->sortItems();
  }
  return item;
}

QListWidgetItem* qtAssociationWidget::addAttributeAssociationItem(
  QListWidget* theList, smtk::attribute::AttributePtr att, bool sort)
{
  QString txtLabel(att->name().c_str());

  QListWidgetItem* item = new QListWidgetItem(txtLabel, theList, smtk_USER_DATA_TYPE);
  QVariant vdata;
  vdata.setValue(static_cast<void*>(att.get()));
  item->setData(Qt::UserRole, vdata);
  theList->addItem(item);
  if (sort)
  {
    theList->sortItems();
  }
  return item;
}

void qtAssociationWidget::addDomainListItem(
  const smtk::model::Group& domainEnt, QList<smtk::attribute::AttributePtr>& allAtts)
{
  smtk::attribute::ResourcePtr attResource = this->Internals->View->uiManager()->attResource();

  QTableWidgetItem* domainItem = new QTableWidgetItem(QString::fromStdString(domainEnt.name()));
  domainItem->setFlags(Qt::ItemIsEnabled);

  int numRows = this->Internals->DomainMaterialTable->rowCount();
  this->Internals->DomainMaterialTable->setRowCount(++numRows);
  this->Internals->DomainMaterialTable->setItem(numRows - 1, 0, domainItem);

  QList<QString> attNames;
  QString attname;
  foreach (smtk::attribute::AttributePtr att, allAtts)
  {
    attname = att->name().c_str();
    if (!attNames.contains(attname))
    {
      attNames.push_back(attname);
    }
  }

  QComboBox* combo = new QComboBox(this);
  combo->addItems(attNames);
  int idx = -1;
  smtk::common::UUIDs associatedAtts = domainEnt.attributeIds();
  if (associatedAtts.size() > 0)
  {
    smtk::attribute::AttributePtr first_att = attResource->findAttribute((*associatedAtts.begin()));
    if (first_att)
    {
      idx = attNames.indexOf(QString::fromStdString(first_att->name()));
    }
  }
  combo->setCurrentIndex(idx);
  QObject::connect(combo, SIGNAL(currentIndexChanged(int)), this,
    SLOT(onDomainAssociationChanged()), Qt::QueuedConnection);

  QVariant vdata(QString::fromStdString(domainEnt.entity().toString()));
  combo->setProperty("DomainEntityObj", vdata);

  this->Internals->DomainMaterialTable->setCellWidget(numRows - 1, 1, combo);
  this->Internals->DomainMaterialTable->setItem(numRows - 1, 1, new QTableWidgetItem());
}

void qtAssociationWidget::onRemoveAssigned()
{
  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);

  QListWidgetItem* selItem = NULL;
  QListWidget* theList = this->Internals->CurrentList;
  QList<QListWidgetItem*> selItems = this->getSelectedItems(theList);
  foreach (QListWidgetItem* item, selItems)
  {
    if (this->Internals->CurrentAtt.lock())
    {
      smtk::model::EntityRef currentItem = this->getSelectedModelEntityItem(item);
      if (currentItem.isValid())
      {
        this->Internals->CurrentAtt.lock()->disassociateEntity(currentItem);
        this->removeItem(theList, item);
        // only add entityRef back to availableList when it belongs to
        // the current active model
        if (currentItem.owningModel().entity() ==
          qtActiveObjects::instance().activeModel().entity())
        {
          selItem = this->addModelAssociationListItem(this->Internals->AvailableList, currentItem);
        }
      }
    }
    else if (!this->Internals->CurrentModelGroup.isNull())
    {
      smtk::attribute::AttributePtr currentAtt = this->getSelectedAttribute(item);
      if (currentAtt)
      {
        currentAtt->disassociateEntity(this->Internals->CurrentModelGroup);
        this->removeItem(theList, item);
        selItem = this->addAttributeAssociationItem(this->Internals->AvailableList, currentAtt);
      }
    }
  }

  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
  if (selItem)
  {
    emit this->attAssociationChanged();
    // highlight selected item in AvailableList
    this->updateListItemSelectionAfterChange(selItems, this->Internals->AvailableList);
    this->Internals->CurrentList->setCurrentItem(NULL);
    this->Internals->CurrentList->clearSelection();
  }
}

void qtAssociationWidget::onAddAvailable()
{
  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  QListWidgetItem* selItem = NULL;
  QListWidget* theList = this->Internals->AvailableList;
  QList<QListWidgetItem*> selItems = this->getSelectedItems(theList);
  foreach (QListWidgetItem* item, selItems)
  {
    if (this->Internals->CurrentAtt.lock())
    {
      smtk::model::EntityRef currentItem = this->getSelectedModelEntityItem(item);
      if (currentItem.isValid())
      {
        if (this->Internals->CurrentAtt.lock()->associateEntity(currentItem))
        {
          this->removeItem(theList, item);
          selItem = this->addModelAssociationListItem(
            this->Internals->CurrentList, currentItem, true, true);
        }
        else // failed to associate with new entity
        {
          QMessageBox::warning(
            this, tr("Associate Entities"), tr("Failed to associate with new entity!"));
        }
      }
    }
    else if (!this->Internals->CurrentModelGroup.isNull())
    {
      smtk::attribute::AttributePtr currentAtt = this->getSelectedAttribute(item);
      if (currentAtt)
      {
        if (currentAtt->definition()->isUnique() && this->Internals->CurrentList->count())
        {
          this->onExchange();
          return;
        }
        if (currentAtt->associateEntity(this->Internals->CurrentModelGroup))
        {
          this->removeItem(theList, item);
          selItem = this->addAttributeAssociationItem(this->Internals->CurrentList, currentAtt);
        }
        else // failed to associate with new entity
        {
          QMessageBox::warning(
            this, tr("Associate Entities"), tr("Failed to associate with new entity!"));
        }
      }
    }
  }

  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
  if (selItem)
  {
    emit this->attAssociationChanged();
    // highlight selected item in CurrentList
    this->updateListItemSelectionAfterChange(selItems, this->Internals->CurrentList);
    this->Internals->AvailableList->setCurrentItem(NULL);
    this->Internals->AvailableList->clearSelection();
  }
}

void qtAssociationWidget::onExchange()
{
  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  QList<QListWidgetItem*> selCurrentItems = this->getSelectedItems(this->Internals->CurrentList);
  QList<QListWidgetItem*> selAvailItems = this->getSelectedItems(this->Internals->AvailableList);
  if (selCurrentItems.count() != selAvailItems.count())
  {
    QMessageBox::warning(
      this, tr("Exchange Attributes"), tr("The number of items for exchange has to be the same!"));
    return;
  }

  QListWidgetItem* selCurrentItem = NULL;
  QListWidgetItem* selAvailItem = NULL;
  for (int i = 0; i < selCurrentItems.count(); ++i)
  {
    if (this->Internals->CurrentAtt.lock())
    {
      smtk::model::EntityRef currentItem = this->getSelectedModelEntityItem(selCurrentItems[i]);
      smtk::model::EntityRef availableItem = this->getSelectedModelEntityItem(selAvailItems[i]);
      if (currentItem.isValid() && availableItem.isValid())
      {
        this->Internals->CurrentAtt.lock()->disassociateEntity(currentItem);

        if (this->Internals->CurrentAtt.lock()->associateEntity(availableItem))
        {
          this->removeItem(this->Internals->CurrentList, selCurrentItems[i]);
          selCurrentItem = this->addModelAssociationListItem(
            this->Internals->CurrentList, availableItem, true, true);

          this->removeItem(this->Internals->AvailableList, selAvailItems[i]);
          // only add it back to available list when it's part of active model
          if (currentItem.owningModel().entity() ==
            qtActiveObjects::instance().activeModel().entity())
          {
            selAvailItem =
              this->addModelAssociationListItem(this->Internals->AvailableList, currentItem);
          }
        }
        else // faied to exchange, add back association
        {
          QMessageBox::warning(
            this, tr("Exchange Attributes"), tr("Failed to associate with new entity!"));
          this->Internals->CurrentAtt.lock()->associateEntity(currentItem);
        }
      }
    }
    else if (!this->Internals->CurrentModelGroup.isNull())
    {
      smtk::attribute::AttributePtr availAtt = this->getSelectedAttribute(selAvailItems[i]);
      smtk::attribute::AttributePtr currentAtt = this->getSelectedAttribute(selCurrentItems[i]);
      if (availAtt && currentAtt)
      {
        currentAtt->disassociateEntity(this->Internals->CurrentModelGroup);
        if (availAtt->associateEntity(this->Internals->CurrentModelGroup))
        {
          selCurrentItem =
            this->addAttributeAssociationItem(this->Internals->AvailableList, currentAtt);
          this->removeItem(this->Internals->CurrentList, selCurrentItems[i]);

          this->removeItem(this->Internals->AvailableList, selAvailItems[i]);
          selAvailItem = this->addAttributeAssociationItem(this->Internals->CurrentList, availAtt);
        }
        else // faied to exchange, add back association
        {
          QMessageBox::warning(
            this, tr("Exchange Attributes"), tr("Failed to associate with new entity!"));
          currentAtt->associateEntity(this->Internals->CurrentModelGroup);
        }
      }
    }
  }

  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
  if (selCurrentItems.count() || selAvailItems.count())
  {
    emit this->attAssociationChanged();
    if (selCurrentItems.count())
    {
      // highlight selected item in AvailableList
      this->updateListItemSelectionAfterChange(selCurrentItems, this->Internals->AvailableList);
    }
    if (selAvailItems.count())
    {
      // highlight selected item in CurrentList
      this->updateListItemSelectionAfterChange(selAvailItems, this->Internals->CurrentList);
    }
  }
}

void qtAssociationWidget::onNodalOptionChanged(int idx)
{
  smtk::attribute::AttributePtr currAtt = this->Internals->CurrentAtt.lock();
  if (!currAtt)
  {
    return;
  }
  switch (idx)
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

void qtAssociationWidget::onDomainAssociationChanged()
{
  smtk::attribute::ResourcePtr attResource = this->Internals->View->uiManager()->attResource();
  smtk::model::ManagerPtr modelManager = attResource->refModelManager();

  QComboBox* const combo = qobject_cast<QComboBox*>(QObject::sender());
  if (!combo)
  {
    return;
  }
  QString domainStr = combo->property("DomainEntityObj").toString();
  smtk::common::UUID uid(domainStr.toStdString());
  smtk::model::Group domainItem(modelManager, uid);
  if (!domainItem.isValid())
  {
    return;
  }

  domainItem.disassociateAllAttributes(attResource); //detach all attributes

  if (combo->currentText().isEmpty())
  {
    return;
  }

  QString attName = combo->currentText();
  AttributePtr attPtr = attResource->findAttribute(attName.toStdString());
  if (attPtr)
  {
    domainItem.associateAttribute(attResource, attPtr->id());
    emit this->attAssociationChanged();
  }
  else
  {
    QString strMessage = QString("Can't find attribute with this name: ") + attName;
    QMessageBox::warning(this, tr("Domain Associations"), strMessage);
    combo->setCurrentIndex(-1);
  }
}

void qtAssociationWidget::updateListItemSelectionAfterChange(
  QList<QListWidgetItem*> selItems, QListWidget* list)
{
  list->blockSignals(true);
  foreach (QListWidgetItem* item, selItems)
  {
    QList<QListWidgetItem*> findList = list->findItems(item->text(), Qt::MatchExactly);
    foreach (QListWidgetItem* findItem, findList)
    {
      findItem->setSelected(true);
    }
  }
  list->blockSignals(false);
}
