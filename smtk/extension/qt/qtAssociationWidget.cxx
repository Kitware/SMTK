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
#include "smtk/extension/qt/qtSMTKUtilities.h"
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
#include "smtk/model/Resource.h"

#include "smtk/resource/Manager.h"

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
  smtk::model::ResourcePtr modelResource = this->getModelResource();

  if (!modelResource)
  {
    return;
  }

  // Let get all of the entities that could be possibly associated to the attrbute
  auto entities = modelResource->findAs<smtk::model::EntityArray>(
    smtk::model::Entity::flagSummary(attDef->associationMask()));

  // Lets also find the base definition of the attribute that forces the unique condition
  ResourcePtr attResource = attDef->resource();
  smtk::attribute::ConstDefinitionPtr baseDef = attResource->findIsUniqueBaseClass(attDef);

  // Now go through the list of entities and for each see if it has attributes related to
  // the base type.  If it doesn't then it can be added to the available list else
  // if it has this attribute associated with it then it is added to the current list
  // else it already has a different attribute of a related type associated with it and
  // can be skipped.

  for (auto entity : entities)
  {
    auto atts = entity->attributes(baseDef);
    if (atts.size() == 0)
    {
      // Entity doesn't have any appropriate attribute associated with it
      this->addModelAssociationListItem(this->Internals->AvailableList, entity, false);
    }
    else if (atts.at(0) == theAtt)
    {
      // Entity is associated with the attribute already
      this->addModelAssociationListItem(this->Internals->CurrentList, entity, false, true);
    }
  }
  this->Internals->CurrentList->sortItems();
  this->Internals->AvailableList->sortItems();
  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
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

smtk::model::EntityPtr qtAssociationWidget::getSelectedModelEntityItem(QListWidgetItem* item)
{
  return this->getModelEntityItem(item);
}

smtk::model::ResourcePtr qtAssociationWidget::getModelResource()
{
  auto resManager = this->Internals->View->uiManager()->resourceManager();
  auto modelResources = resManager->find<smtk::model::Resource>();
  smtk::model::ResourcePtr mresource;
  // Grab the first model resource (if any)
  // ToDo This should be using Resource Links
  if (!modelResources.empty())
  {
    mresource = *modelResources.begin();
  }
  return mresource;
}

smtk::model::EntityPtr qtAssociationWidget::getModelEntityItem(QListWidgetItem* item)
{
  smtk::model::EntityPtr entity;
  if (item)
  {
    QVariant var = item->data(Qt::UserRole);
    smtk::common::UUID uid = qtSMTKUtilities::QVariantToUUID(var);
    ;
    smtk::attribute::ResourcePtr attResource = this->Internals->View->uiManager()->attResource();
    if (attResource)
    {
      smtk::model::ResourcePtr modelResource = this->getModelResource();
      entity = modelResource->findEntity(uid);
    }
  }
  return entity;
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
  QListWidget* theList, smtk::model::EntityPtr entity, bool sort, bool appendModelName)
{
  std::string name =
    appendModelName ? (entity->name() + " - " + entity->owningModel()->name()) : entity->name();
  QListWidgetItem* item =
    new QListWidgetItem(QString::fromStdString(name), theList, smtk_USER_DATA_TYPE);
  //save the entity as a uuid string
  QVariant vdata = qtSMTKUtilities::UUIDToQVariant(entity->id());
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
      smtk::model::EntityPtr currentItem = this->getSelectedModelEntityItem(item);
      if (currentItem)
      {
        this->Internals->CurrentAtt.lock()->disassociate(currentItem);
        this->removeItem(theList, item);
        selItem = this->addModelAssociationListItem(this->Internals->AvailableList, currentItem);
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
      smtk::model::EntityPtr currentItem = this->getSelectedModelEntityItem(item);
      if (currentItem)
      {
        if (this->Internals->CurrentAtt.lock()->associate(currentItem))
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
      smtk::model::EntityPtr currentItem = this->getSelectedModelEntityItem(selCurrentItems[i]);
      smtk::model::EntityPtr availableItem = this->getSelectedModelEntityItem(selAvailItems[i]);
      if (currentItem && availableItem)
      {
        this->Internals->CurrentAtt.lock()->disassociate(currentItem);

        if (this->Internals->CurrentAtt.lock()->associate(availableItem))
        {
          this->removeItem(this->Internals->CurrentList, selCurrentItems[i]);
          selCurrentItem = this->addModelAssociationListItem(
            this->Internals->CurrentList, availableItem, true, true);

          this->removeItem(this->Internals->AvailableList, selAvailItems[i]);
          selAvailItem =
            this->addModelAssociationListItem(this->Internals->AvailableList, currentItem);
        }
        else // faied to exchange, add back association
        {
          QMessageBox::warning(
            this, tr("Exchange Attributes"), tr("Failed to associate with new entity!"));
          this->Internals->CurrentAtt.lock()->associate(currentItem);
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
  smtk::model::ResourcePtr modelResource = this->getModelResource();

  QComboBox* const combo = qobject_cast<QComboBox*>(QObject::sender());
  if (!combo)
  {
    return;
  }
  QString domainStr = combo->property("DomainEntityObj").toString();
  smtk::common::UUID uid(domainStr.toStdString());
  smtk::model::Group domainItem(modelResource, uid);
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
