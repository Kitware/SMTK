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
#include "smtk/extension/qt/qtTypeDeclarations.h"
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

#include "smtk/view/Selection.h"

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
  QListWidgetItem* lastHighlightedItem;
  QBrush normalBackground;
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
  auto uiManager = this->Internals->view->uiManager();
  if (uiManager == nullptr)
  {
    std::cerr << "qtAssociationWidget: Could not find UI Manager!\n";
    return;
  }

  if (uiManager->highlightOnHover())
  {
    QObject::connect(this->Internals->CurrentList, SIGNAL(entered(const QModelIndex&)), this,
      SLOT(hoverRow(const QModelIndex&)));
    QObject::connect(this->Internals->AvailableList, SIGNAL(entered(const QModelIndex&)), this,
      SLOT(hoverRow(const QModelIndex&)));
  }
  else
  {
    QObject::disconnect(this->Internals->CurrentList, SIGNAL(entered(const QModelIndex&)), this,
      SLOT(hoverRow(const QModelIndex&)));
    QObject::disconnect(this->Internals->AvailableList, SIGNAL(entered(const QModelIndex&)), this,
      SLOT(hoverRow(const QModelIndex&)));
    this->resetHover();
  }

  auto opManager = uiManager->operationManager();
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
    std::cerr << "qtAssociationWidget: Could not find Resource Manager!\n";
  }
  QObject::connect(this->Internals->view, SIGNAL(aboutToDestroy()), this, SLOT(removeObservers()));

  this->Internals->lastHighlightedItem = nullptr;
}

qtAssociationWidget::~qtAssociationWidget()
{
  this->removeObservers();
  delete this->Internals;
}

void qtAssociationWidget::initWidget()
{
  // Set up icons on buttons
  QString arrowRight(":/icons/attribute/arrowRight.png");
  QString arrowLeft(":/icons/attribute/arrowLeft.png");
  this->Internals->MoveToRight->setIcon(QIcon(arrowRight));
  this->Internals->MoveToLeft->setIcon(QIcon(arrowLeft));

  // signals/slots
  QObject::connect(this->Internals->MoveToRight, SIGNAL(clicked()), this, SLOT(onRemoveAssigned()));
  QObject::connect(this->Internals->MoveToLeft, SIGNAL(clicked()), this, SLOT(onAddAvailable()));
  this->Internals->CurrentList->setMouseTracking(true);   // Needed to receive hover events.
  this->Internals->AvailableList->setMouseTracking(true); // Needed to receive hover events.
}

bool qtAssociationWidget::hasSelectedItem()
{
  return this->Internals->AvailableList->selectedItems().isEmpty() ? false : true;
}

void qtAssociationWidget::showEntityAssociation(smtk::attribute::AttributePtr theAtt)
{
  this->Internals->currentAtt = theAtt;
  this->refreshAssociations();

  // Lets store the normal backgound color of an item - we will
  // need to use this when dealing with hovering
  if (this->Internals->CurrentList->count())
  {
    this->Internals->normalBackground = this->Internals->CurrentList->item(0)->background();
  }
  else if (this->Internals->AvailableList->count())
  {
    this->Internals->normalBackground = this->Internals->AvailableList->item(0)->background();
  }
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

  ResourcePtr attResource = attDef->resource();
  auto objects = this->associatableObjects();
  smtk::attribute::DefinitionPtr preDef;
  smtk::attribute::AttributePtr conAtt;
  // Now lets see if the objects are associated with this attribute or can be
  for (auto obj : objects)
  {
    if (theAttribute->isObjectAssociated(obj))
    {
      this->addObjectAssociationListItem(this->Internals->CurrentList, obj, false, true);
    }
    else
    {
      auto result = attDef->canBeAssociated(obj, conAtt, preDef);
      if (result == smtk::attribute::Definition::AssociationResultType::Valid)
      {
        this->addObjectAssociationListItem(this->Internals->AvailableList, obj, false);
      }
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
    if (resManager == nullptr)
    {
      return result;
    }

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
  if (item == nullptr)
  {
    smtk::resource::PersistentObjectPtr obj;
    return obj;
  }

  return item->data(Qt::UserRole).value<smtk::resource::PersistentObjectPtr>();
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
  vdata.setValue(object);
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
      AttributePtr probAtt;
      if (att->disassociate(currentItem, probAtt))
      {
        this->removeItem(theList, item);
        selItem = this->addObjectAssociationListItem(this->Internals->AvailableList, currentItem);
      }
      else
      {
        std::string s("Could not disassociate from ");
        s.append(currentItem->name())
          .append(" - due to attribute ")
          .append(probAtt->name())
          .append(" using this as a prerequisite");
        QMessageBox::warning(this, "Can't disassociate attribute", s.c_str());
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

void qtAssociationWidget::removeObservers()
{
  if (m_operationObserverKey.assigned() && this->Internals->view)
  {
    auto opManager = this->Internals->view->uiManager()->operationManager();
    if (opManager != nullptr)
    {
      opManager->observers().erase(m_operationObserverKey);
    }
  }
  if (m_resourceObserverKey.assigned() && this->Internals->view)
  {
    auto resManager = this->Internals->view->uiManager()->resourceManager();
    if (resManager != nullptr)
    {
      resManager->observers().erase(m_resourceObserverKey);
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
  (void)resource;
  if (event == smtk::resource::EventType::REMOVED)
  {
    // The simplest solution is just to refresh the widget
    this->refreshAssociations();
  }
}

void qtAssociationWidget::leaveEvent(QEvent* evt)
{
  this->resetHover();
  // Now let the superclass do what it wants:
  QWidget::leaveEvent(evt);
}

void qtAssociationWidget::hoverRow(const QModelIndex& idx)
{
  auto uiManager = this->Internals->view->uiManager();
  if (uiManager == nullptr)
  {
    return;
  }

  auto selection = uiManager->selection();
  if (selection == nullptr)
  {
    return;
  }
  int row = idx.row();

  // Lets create the hover background color;
  // If there was a previously highlighted item and it is still using the
  // hover color then reset its background;
  if (this->Internals->lastHighlightedItem != nullptr)
  {
    this->Internals->lastHighlightedItem->setBackground(this->Internals->normalBackground);
  }

  // Discover what is currently hovered
  auto obj = idx.data(Qt::UserRole).value<smtk::resource::PersistentObjectPtr>();
  if (obj == nullptr)
  {
    return;
  }

  auto item = this->Internals->CurrentList->item(row);
  if (item != nullptr)
  {
    auto iobj = item->data(Qt::UserRole).value<smtk::resource::PersistentObjectPtr>();
    if (iobj != obj)
    {
      item = this->Internals->AvailableList->item(row);
    }
  }
  else
  {
    item = this->Internals->AvailableList->item(row);
  }

  if (item)
  {
    item->setBackground(
      QBrush(this->Internals->CurrentList->palette().highlight().color().lighter(125)));
    this->Internals->lastHighlightedItem = item;
  }
  // Add new hover state
  auto hoverMask = uiManager->hoverBit();
  const auto& selnMap = selection->currentSelection();
  auto cvit = selnMap.find(obj);
  int sv = (cvit == selnMap.end() ? 0 : cvit->second) | hoverMask;
  smtk::resource::PersistentObjectSet objs;
  objs.insert(obj);
  selection->modifySelection(
    objs, m_selectionSourceName, sv, smtk::view::SelectionAction::UNFILTERED_REPLACE, true);
}

void qtAssociationWidget::resetHover()
{
  auto uiManager = this->Internals->view->uiManager();
  if (uiManager == nullptr)
  {
    return;
  }

  if (this->Internals->lastHighlightedItem != nullptr)
  {
    this->Internals->lastHighlightedItem->setBackground(this->Internals->normalBackground);
  }
  auto selection = uiManager->selection();
  if (selection == nullptr)
  {
    return;
  }
  selection->resetSelectionBits(m_selectionSourceName, uiManager->hoverBit());
}
