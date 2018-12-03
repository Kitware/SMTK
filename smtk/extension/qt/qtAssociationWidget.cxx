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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/resource/Component.h"
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

class qtAssociationWidgetInternals : public Ui::qtAttributeAssociation
{
public:
  WeakAttributePtr currentAtt;
  QPointer<qtBaseView> view;
};

qtAssociationWidget::qtAssociationWidget(QWidget* _p, qtBaseView* bview)
  : QWidget(_p)
{
  this->Internals = new qtAssociationWidgetInternals;
  this->Internals->setupUi(this);
  this->Internals->view = bview;

  this->initWidget();
  std::ostringstream receiverSource;
  receiverSource << "qtAssociationWidget_" << this;
  m_selectionSourceName = receiverSource.str();
  auto opManager = this->Internals->view->uiManager()->operationManager();
  if (opManager != nullptr)
  {
    m_operationObserverKey = opManager->observers().insert(
      [this](smtk::operation::Operation::Ptr oper, smtk::operation::EventType event,
        smtk::operation::Operation::Result result) -> int {
        return this->handleOperationEvent(oper, event, result);
      });
  }
  else
  {
    m_operationObserverKey = -1;
    std::cerr << "qtAssociationWidget: Could not find Operation Manager!\n";
  }
  auto resManager = this->Internals->view->uiManager()->resourceManager();
  if (resManager != nullptr)
  {
    m_resourceObserverKey =
      resManager->observers().insert([this](smtk::resource::Resource::Ptr resource,
        smtk::resource::EventType event) { this->handleResourceEvent(resource, event); });
  }
  else
  {
    m_resourceObserverKey = -1;
    std::cerr << "qtAssociationWidget: Could not find Resource Manager!\n";
  }
}

qtAssociationWidget::~qtAssociationWidget()
{
  if (m_operationObserverKey != -1)
  {
    auto opManager = this->Internals->view->uiManager()->operationManager();
    if (opManager != nullptr)
    {
      opManager->observers().erase(m_operationObserverKey);
    }
  }
  if (m_resourceObserverKey != -1)
  {
    auto resManager = this->Internals->view->uiManager()->resourceManager();
    if (resManager != nullptr)
    {
      resManager->observers().erase(m_resourceObserverKey);
    }
  }
  delete this->Internals;
}

void qtAssociationWidget::initWidget()
{
  // signals/slots
  QObject::connect(this->Internals->MoveToRight, SIGNAL(clicked()), this, SLOT(onRemoveAssigned()));
  QObject::connect(this->Internals->MoveToLeft, SIGNAL(clicked()), this, SLOT(onAddAvailable()));
  QObject::connect(this->Internals->ExchangeLeftRight, SIGNAL(clicked()), this, SLOT(onExchange()));
}

bool qtAssociationWidget::hasSelectedItem()
{
  return this->Internals->AvailableList->selectedItems().isEmpty() ? false : true;
}

void qtAssociationWidget::showEntityAssociation(smtk::attribute::AttributePtr theAtt)
{
  this->Internals->currentAtt = theAtt;
  this->refreshAssociations();
}

void qtAssociationWidget::refreshAssociations()
{
  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  this->Internals->CurrentList->clear();
  this->Internals->AvailableList->clear();

  auto theAttribute = this->Internals->currentAtt.lock();

  if (!theAttribute)
  {
    this->Internals->CurrentList->blockSignals(false);
    this->Internals->AvailableList->blockSignals(false);
    return;
  }

  attribute::DefinitionPtr attDef = theAttribute->definition();

  // Lets also find the base definition of the attribute that forces the unique condition
  ResourcePtr attResource = attDef->resource();
  smtk::attribute::ConstDefinitionPtr baseDef = attResource->findIsUniqueBaseClass(attDef);

  auto objects = this->associatableObjects();

  // Now go through the list of objects and for each see if it has attributes related to
  // the base type.  If it doesn't then it can be added to the available list else
  // if it has this attribute associated with it then it is added to the current list
  // else it already has a different attribute of a related type associated with it and
  // can be skipped.
  for (auto obj : objects)
  {
    auto atts = baseDef->attributes(obj);
    if (atts.size() == 0)
    {
      // Object doesn't have any appropriate attribute associated with it
      this->addObjectAssociationListItem(this->Internals->AvailableList, obj, false);
    }
    else if ((*atts.begin()) == theAttribute)
    {
      // Entity is associated with the attribute already
      this->addObjectAssociationListItem(this->Internals->CurrentList, obj, false, true);
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

smtk::resource::PersistentObjectPtr qtAssociationWidget::selectedObject(QListWidgetItem* item)
{
  return this->object(item);
}

std::set<smtk::resource::PersistentObjectPtr> qtAssociationWidget::associatableObjects() const
{
  std::set<smtk::resource::PersistentObjectPtr> result;
  // First we need to determin if the attribute resource has resources associated with it
  // if not we need to go to resource manager to get the information
  auto theAttribute = this->Internals->currentAtt.lock();
  auto attResource = theAttribute->attributeResource();
  auto associationItem = theAttribute->associatedObjects();
  auto assocMap = associationItem->acceptableEntries();

  auto resources = attResource->associations();
  if (!resources.empty())
  {
    // Iterate over the acceptable entries
    decltype(assocMap.equal_range("")) range;
    for (auto i = assocMap.begin(); i != assocMap.end(); i = range.second)
    {
      // Get the range for the current key
      range = assocMap.equal_range(i->first);

      // Lets see if any of the resources match this type
      for (auto resource : resources)
      {
        if (resource->isOfType(i->first))
        {
          // We need to find all of the component types for
          // this resource.  If a string is empty then the resource
          // itself can be associated with the attribute
          for (auto j = range.first; j != range.second; ++j)
          {
            if (j->second.empty())
            {
              result.insert(resource);
            }
            else
            {
              auto comps = resource->find(j->second);
              result.insert(comps.begin(), comps.end());
            }
          }
        }
      }
    }
  }
  else // we need to use the resource manager
  {
    // Iterate over the acceptable entries
    auto resManager = this->Internals->view->uiManager()->resourceManager();
    decltype(assocMap.equal_range("")) range;
    for (auto i = assocMap.begin(); i != assocMap.end(); i = range.second)
    {
      // Get the range for the current key
      range = assocMap.equal_range(i->first);

      // As the resource manager to get all appropriate resources
      resources = resManager->find(i->first);
      // Need to process all of these resources
      for (auto resource : resources)
      {
        // We need to find all of the component types for
        // this resource.  If a string is empty then the resource
        // itself can be associated with the attribute
        for (auto j = range.first; j != range.second; ++j)
        {
          if (j->second.empty())
          {
            result.insert(resource);
          }
          else
          {
            auto comps = resource->find(j->second);
            result.insert(comps.begin(), comps.end());
          }
        }
      }
    }
  }
  return result;
}

smtk::resource::PersistentObjectPtr qtAssociationWidget::object(QListWidgetItem* item)
{
  auto resManager = this->Internals->view->uiManager()->resourceManager();
  smtk::resource::PersistentObjectPtr object;
  if (item == nullptr)
  {
    smtk::resource::PersistentObjectPtr obj;
    return obj;
  }

  QVariant var = item->data(Qt::UserRole);
  smtk::common::UUID uid = qtSMTKUtilities::QVariantToUUID(var);
  // Get the resource
  smtk::resource::ResourcePtr res = resManager->get(uid);
  if (res == nullptr)
  {
    std::cerr << "Could not find Item's Resource!\n";
    return res;
  }
  // Now check to see if there is data associated with UserRole+1 which would mean we are
  // dealing with a resource component
  var = item->data(Qt::UserRole + 1);
  if (!var.isValid())
  {
    return res;
  }

  uid = qtSMTKUtilities::QVariantToUUID(var);
  auto comp = res->find(uid);
  if (comp == nullptr)
  {
    std::cerr << "Could not find Item's Resource Component!\n";
  }
  return comp;
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

QListWidgetItem* qtAssociationWidget::addObjectAssociationListItem(QListWidget* theList,
  const smtk::resource::PersistentObjectPtr& object, bool sort, bool appendResourceName)
{
  std::string name;
  auto res = std::dynamic_pointer_cast<smtk::resource::Resource>(object);
  auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(object);
  // Are we dealing with a resource or are we not appending the component's resource name?
  if (res || !appendResourceName)
  {
    name = object->name();
  }
  else
  {
    name = object->name() + " - " + comp->resource()->name();
  }

  QListWidgetItem* item =
    new QListWidgetItem(QString::fromStdString(name), theList, smtk_USER_DATA_TYPE);
  QVariant vdata;
  //save the entity as a uuid strings
  if (res)
  {
    vdata = qtSMTKUtilities::UUIDToQVariant(object->id());
    item->setData(Qt::UserRole, vdata);
  }
  else
  {
    vdata = qtSMTKUtilities::UUIDToQVariant(comp->resource()->id());
    item->setData(Qt::UserRole, vdata);
    vdata = qtSMTKUtilities::UUIDToQVariant(comp->id());
    item->setData(Qt::UserRole + 1, vdata);
  }
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

void qtAssociationWidget::onRemoveAssigned()
{
  auto att = this->Internals->currentAtt.lock();
  if (att == nullptr)
  {
    return; // there is nothing to do
  }

  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);

  QListWidgetItem* selItem = nullptr;
  QListWidget* theList = this->Internals->CurrentList;
  QList<QListWidgetItem*> selItems = this->getSelectedItems(theList);
  foreach (QListWidgetItem* item, selItems)
  {
    auto currentItem = this->selectedObject(item);
    if (currentItem)
    {
      att->disassociate(currentItem);
      this->removeItem(theList, item);
      selItem = this->addObjectAssociationListItem(this->Internals->AvailableList, currentItem);
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
  auto att = this->Internals->currentAtt.lock();
  if (att == nullptr)
  {
    return; // Nothing to do
  }

  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  QListWidgetItem* selItem = NULL;
  QListWidget* theList = this->Internals->AvailableList;
  QList<QListWidgetItem*> selItems = this->getSelectedItems(theList);
  foreach (QListWidgetItem* item, selItems)
  {
    auto currentItem = this->selectedObject(item);
    if (currentItem)
    {
      if (att->associate(currentItem))
      {
        this->removeItem(theList, item);
        selItem =
          this->addObjectAssociationListItem(this->Internals->CurrentList, currentItem, true, true);
      }
      else // failed to associate with new entity
      {
        QMessageBox::warning(
          this, tr("Associate Entities"), tr("Failed to associate with new object!"));
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
  auto att = this->Internals->currentAtt.lock();
  if (att == nullptr)
  {
    return; // Nothing to do
  }

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
    auto currentItem = this->selectedObject(selCurrentItems[i]);
    auto availableItem = this->selectedObject(selAvailItems[i]);
    if (currentItem && availableItem)
    {
      att->disassociate(currentItem);

      if (att->associate(availableItem))
      {
        this->removeItem(this->Internals->CurrentList, selCurrentItems[i]);
        selCurrentItem = this->addObjectAssociationListItem(
          this->Internals->CurrentList, availableItem, true, true);

        this->removeItem(this->Internals->AvailableList, selAvailItems[i]);
        selAvailItem =
          this->addObjectAssociationListItem(this->Internals->AvailableList, currentItem);
      }
      else // faied to exchange, add back association
      {
        QMessageBox::warning(
          this, tr("Exchange Attributes"), tr("Failed to associate with new object!"));
        att->associate(currentItem);
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

int qtAssociationWidget::handleOperationEvent(smtk::operation::OperationPtr,
  smtk::operation::EventType event, smtk::operation::Operation::Result result)
{
  if (event != smtk::operation::EventType::DID_OPERATE)
  {
    return 0;
  }

  std::size_t count = smtk::operation::extractResources(result).size();
  // If nothing has changed then just return
  if (count == 0)
  {
    return 0;
  }

  // The simplest solution is just to refresh the widget
  this->refreshAssociations();
  return 0;
}

void qtAssociationWidget::handleResourceEvent(
  smtk::resource::Resource::Ptr resource, smtk::resource::EventType event)
{
  if (event == smtk::resource::EventType::REMOVED)
  {
    // The simplest solution is just to refresh the widget
    this->refreshAssociations();
  }
}
