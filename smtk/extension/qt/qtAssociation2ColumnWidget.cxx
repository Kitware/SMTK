//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAssociation2ColumnWidget.h"
#include "smtk/extension/qt/qtActiveObjects.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtSMTKUtilities.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Resource.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

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

namespace
{
// There seems to be a bug in Qt that causes a crash when
// setting the background color of a list widget item when a
// QLineEdit widget is firing a EditingCompleted signal.
// This constant temporarily turns this capability off.

const bool ALLOW_LIST_HIGHLIGHTING = false;
}
namespace Ui
{
class qtAttributeAssociation;
}

using namespace smtk::attribute;
using namespace smtk::extension;

class qtAssociation2ColumnWidgetInternals : public Ui::qtAttributeAssociation
{
public:
  qtAssociation2ColumnWidgetInternals() = default;
  WeakAttributePtr currentAtt;
  QPointer<qtBaseView> view;
  QListWidgetItem* lastHighlightedItem{ nullptr };
  QBrush normalBackground;
};

qtAssociation2ColumnWidget::qtAssociation2ColumnWidget(QWidget* _p, qtBaseView* bview)
  : qtAssociationWidget(_p, bview)
{
  m_allAssociatedWarning =
    "There are still components not associated to any of the above attributes.";
  m_internals = new qtAssociation2ColumnWidgetInternals;
  m_internals->setupUi(this);
  this->initWidget();

  // Are there any customizations?
  const smtk::view::Configuration::Component& config = m_view->getObject()->details();
  m_allAssociatedMode = config.attributeAsBool("RequireAllAssociated");
  std::string val;
  if (config.attribute("AvailableLabel", val))
  {
    this->setAvailableLabel(val);
  }
  if (config.attribute("CurrentLabel", val))
  {
    this->setCurrentLabel(val);
  }
  if (config.attribute("AssociationTitle", val))
  {
    this->setTitleLabel(val);
  }

  std::ostringstream receiverSource;
  receiverSource << "qtAssociation2ColumnWidget_" << this;
  m_selectionSourceName = receiverSource.str();
  auto uiManager = m_view->uiManager();
  if (uiManager == nullptr)
  {
    std::cerr << "qtAssociation2ColumnWidget: Could not find UI Manager!\n";
    return;
  }

  this->highlightOnHoverChanged(uiManager->highlightOnHover());
  QObject::connect(
    uiManager, SIGNAL(highlightOnHoverChanged(bool)), this, SLOT(highlightOnHoverChanged(bool)));

  auto opManager = uiManager->operationManager();
  QPointer<qtAssociation2ColumnWidget> guardedObject(this);
  if (opManager != nullptr)
  {
    m_operationObserverKey = opManager->observers().insert(
      [guardedObject](const smtk::operation::Operation& oper, smtk::operation::EventType event,
        smtk::operation::Operation::Result result) -> int {
        if (guardedObject == nullptr)
        {
          return 0;
        }
        return guardedObject->handleOperationEvent(oper, event, result);
      },
      "qtAssociation2ColumnWidget: Refresh widget when resources are modified.");
  }
  else
  {
    std::cerr << "qtAssociation2ColumnWidget: Could not find Operation Manager!\n";
  }
  auto resManager = m_view->uiManager()->resourceManager();
  if (resManager != nullptr)
  {
    m_resourceObserverKey = resManager->observers().insert(
      [guardedObject](const smtk::resource::Resource& resource, smtk::resource::EventType event) {
        if (guardedObject == nullptr)
        {
          return 0;
        }
        return guardedObject->handleResourceEvent(resource, event);
      },
      "qtAssociation2ColumnWidget: Refresh widget when resources are removed.");
  }
  else
  {
    std::cerr << "qtAssociation2ColumnWidget: Could not find Resource Manager!\n";
  }
  QObject::connect(m_view, SIGNAL(aboutToDestroy()), this, SLOT(removeObservers()));
  QObject::connect(m_internals->CurrentList,
    SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this,
    SLOT(onCurrentItemChanged(QListWidgetItem*, QListWidgetItem*)), Qt::QueuedConnection);

  QObject::connect(m_internals->AvailableList,
    SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this,
    SLOT(onCurrentItemChanged(QListWidgetItem*, QListWidgetItem*)), Qt::QueuedConnection);

  m_internals->lastHighlightedItem = nullptr;
  m_internals->AvailableLabel->setWordWrap(true);
  m_internals->CurrentLabel->setWordWrap(true);
  m_internals->TitleLabel->setWordWrap(true);
}

qtAssociation2ColumnWidget::~qtAssociation2ColumnWidget()
{
  this->removeObservers();
  delete m_internals;
}

void qtAssociation2ColumnWidget::initWidget()
{
  // Set up icons on buttons
  QString arrowRight(":/icons/attribute/arrowRight.png");
  QString arrowLeft(":/icons/attribute/arrowLeft.png");
  m_internals->MoveToRight->setIcon(QIcon(arrowRight));
  m_internals->MoveToLeft->setIcon(QIcon(arrowLeft));
  m_internals->AlertLabel->setPixmap(m_view->uiManager()->alertPixmap());
  m_internals->AlertLabel->hide();

  // signals/slots
  QObject::connect(m_internals->MoveToRight, SIGNAL(clicked()), this, SLOT(onRemoveAssigned()),
    Qt::QueuedConnection);
  QObject::connect(
    m_internals->MoveToLeft, SIGNAL(clicked()), this, SLOT(onAddAvailable()), Qt::QueuedConnection);
  m_internals->CurrentList->setMouseTracking(true);   // Needed to receive hover events.
  m_internals->AvailableList->setMouseTracking(true); // Needed to receive hover events.
}

bool qtAssociation2ColumnWidget::hasSelectedItem()
{
  return !m_internals->AvailableList->selectedItems().isEmpty();
}

void qtAssociation2ColumnWidget::showEntityAssociation(smtk::attribute::AttributePtr theAtt)
{
  m_internals->currentAtt = theAtt;
  this->refreshAssociations();

  // Lets store the normal background color of an item - we will
  // need to use this when dealing with hovering
  if (m_internals->CurrentList->count())
  {
    m_internals->normalBackground = m_internals->CurrentList->item(0)->background();
  }
  else if (m_internals->AvailableList->count())
  {
    m_internals->normalBackground = m_internals->AvailableList->item(0)->background();
  }
}

bool qtAssociation2ColumnWidget::isValid() const
{
  return !m_internals->AlertLabel->isVisible();
}

void qtAssociation2ColumnWidget::updateAssociationStatus(const Attribute* attribute)
{
  auto assocItem = attribute->associatedObjects();
  bool assocValid = assocItem->isValid();
  if (assocValid && !(m_allAssociatedMode && m_internals->AvailableList->count()))
  {
    m_internals->AlertLabel->hide();
    return;
  }
  m_internals->AlertLabel->show();
  QString reason;

  // Lets update the tool-tip to say why its not valid
  if (!assocValid)
  {
    if (assocItem->isExtensible())
    {
      // Remember that an item may have the correct number of values
      // but they may not all be set
      if (assocItem->numberOfValues() <= assocItem->numberOfRequiredValues())
      {
        reason = "Attribute requires at least ";
        QString count;
        count.setNum(assocItem->numberOfRequiredValues());
        reason.append(count).append(" objects associated to it");
      }
      else
      {
        //Must have exceeded max number
        QString reason("Attribute requires no more than ");
        QString count;
        count.setNum(assocItem->maxNumberOfValues());
        reason.append(count).append(" objects associated to it");
      }
    }
    else
    {
      QString reason("Attribute requires ");
      QString count;
      count.setNum(assocItem->numberOfRequiredValues());
      reason.append(count).append(" objects associated to it");
    }
  }
  if (m_allAssociatedMode && m_internals->AvailableList->count())
  {
    if (reason.isEmpty())
    {
      reason = m_allAssociatedWarning;
    }
    else
    {
      reason.append("\n").append(m_allAssociatedWarning);
    }
  }
  m_internals->AlertLabel->setToolTip(reason);
}
void qtAssociation2ColumnWidget::refreshAssociations(const smtk::common::UUID& ignoreResource)
{
  m_internals->CurrentList->blockSignals(true);
  m_internals->AvailableList->blockSignals(true);
  m_internals->CurrentList->clear();
  m_internals->AvailableList->clear();

  auto theAttribute = m_internals->currentAtt.lock();

  if (!theAttribute)
  {
    m_internals->CurrentList->blockSignals(false);
    m_internals->AvailableList->blockSignals(false);
    m_internals->AlertLabel->hide();
    return;
  }

  attribute::DefinitionPtr attDef = theAttribute->definition();
  ResourcePtr attResource = attDef->resource();
  auto objects = this->associatableObjects(ignoreResource);
  smtk::attribute::DefinitionPtr preDef;
  smtk::attribute::AttributePtr conAtt;
  // Now lets see if the objects are associated with this attribute or can be
  for (const auto& obj : objects)
  {
    if (theAttribute->isObjectAssociated(obj))
    {
      this->addObjectAssociationListItem(m_internals->CurrentList, obj, false, true);
    }
    else
    {
      auto result = attDef->canBeAssociated(obj, conAtt, preDef);
      if (result == smtk::attribute::Definition::AssociationResultType::Valid)
      {
        this->addObjectAssociationListItem(m_internals->AvailableList, obj, false);
      }
    }
  }

  m_internals->CurrentList->sortItems();
  m_internals->AvailableList->sortItems();
  // Lets see if the attribute's associations are currently valid
  this->updateAssociationStatus(theAttribute.get());
  m_internals->CurrentList->blockSignals(false);
  m_internals->AvailableList->blockSignals(false);
}

smtk::attribute::AttributePtr qtAssociation2ColumnWidget::getSelectedAttribute(
  QListWidgetItem* item)
{
  return this->getAttribute(item);
}

smtk::attribute::AttributePtr qtAssociation2ColumnWidget::getAttribute(QListWidgetItem* item)
{
  Attribute* rawPtr =
    item ? static_cast<Attribute*>(item->data(Qt::UserRole).value<void*>()) : nullptr;
  return rawPtr ? rawPtr->shared_from_this() : smtk::attribute::AttributePtr();
}

smtk::resource::PersistentObjectPtr qtAssociation2ColumnWidget::selectedObject(
  QListWidgetItem* item)
{
  return this->object(item);
}

std::set<smtk::resource::PersistentObjectPtr> qtAssociation2ColumnWidget::associatableObjects(
  const smtk::common::UUID& ignoreResource) const
{
  std::set<smtk::resource::PersistentObjectPtr> result;
  smtk::resource::ResourceSet resources;
  auto theAttribute = m_internals->currentAtt.lock();
  auto attResource = theAttribute->attributeResource();
  auto associationItem = theAttribute->associatedObjects();
  auto resManager = m_view->uiManager()->resourceManager();
  if (associationItem == nullptr)
  {
    return result;
  }
  auto assocMap = associationItem->acceptableEntries();

  // First we need to determine if the attribute resource has resources associated with it
  // if not we need to go to resource manager to get the information
  if (attResource->hasAssociations() || (resManager == nullptr))
  {
    resources = attResource->associations();
    // we should always consider the attribute resource itself as well
    resources.insert(attResource);
    // Iterate over the acceptable entries
    decltype(assocMap.equal_range("")) range;
    for (auto i = assocMap.begin(); i != assocMap.end(); i = range.second)
    {
      // Get the range for the current key
      range = assocMap.equal_range(i->first);

      // Lets see if any of the resources match this type
      for (const auto& resource : resources)
      {
        if (resource->id() == ignoreResource)
        {
          continue;
        }
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
    decltype(assocMap.equal_range("")) range;
    for (auto i = assocMap.begin(); i != assocMap.end(); i = range.second)
    {
      // Get the range for the current key
      range = assocMap.equal_range(i->first);

      // As the resource manager to get all appropriate resources
      resources = resManager->find(i->first);
      // Need to process all of these resources
      for (const auto& resource : resources)
      {
        if (resource->id() == ignoreResource)
        {
          continue;
        }
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

smtk::resource::PersistentObjectPtr qtAssociation2ColumnWidget::object(QListWidgetItem* item)
{
  if (item == nullptr)
  {
    smtk::resource::PersistentObjectPtr obj;
    return obj;
  }

  return item->data(Qt::UserRole).value<smtk::resource::PersistentObjectPtr>();
}

QList<QListWidgetItem*> qtAssociation2ColumnWidget::getSelectedItems(QListWidget* theList) const
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

void qtAssociation2ColumnWidget::removeItem(QListWidget* theList, QListWidgetItem* selItem)
{
  if (theList && selItem)
  {
    theList->takeItem(theList->row(selItem));
  }
}

QListWidgetItem* qtAssociation2ColumnWidget::addObjectAssociationListItem(QListWidget* theList,
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

QListWidgetItem* qtAssociation2ColumnWidget::addAttributeAssociationItem(
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

void qtAssociation2ColumnWidget::onRemoveAssigned()
{
  auto att = m_internals->currentAtt.lock();
  if (att == nullptr)
  {
    return; // there is nothing to do
  }

  m_internals->CurrentList->blockSignals(true);
  m_internals->AvailableList->blockSignals(true);

  QListWidgetItem* selItem = nullptr;
  QListWidget* theList = m_internals->CurrentList;
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
        selItem = this->addObjectAssociationListItem(m_internals->AvailableList, currentItem);
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

  m_internals->CurrentList->blockSignals(false);
  m_internals->AvailableList->blockSignals(false);
  if (selItem)
  {
    emit this->attAssociationChanged();
    // highlight selected item in AvailableList
    this->updateListItemSelectionAfterChange(selItems, m_internals->AvailableList);
    m_internals->CurrentList->setCurrentItem(nullptr);
    m_internals->CurrentList->clearSelection();
    this->updateAssociationStatus(att.get());
  }
}

void qtAssociation2ColumnWidget::onAddAvailable()
{
  auto att = m_internals->currentAtt.lock();
  if (att == nullptr)
  {
    return; // Nothing to do
  }

  m_internals->CurrentList->blockSignals(true);
  m_internals->AvailableList->blockSignals(true);
  QListWidgetItem* selItem = nullptr;
  QListWidget* theList = m_internals->AvailableList;
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
          this->addObjectAssociationListItem(m_internals->CurrentList, currentItem, true, true);
      }
      else // failed to associate with new entity
      {
        QMessageBox::warning(
          this, tr("Associate Entities"), tr("Failed to associate with new object!"));
      }
    }
  }

  m_internals->CurrentList->blockSignals(false);
  m_internals->AvailableList->blockSignals(false);
  if (selItem)
  {
    emit this->attAssociationChanged();
    // highlight selected item in CurrentList
    this->updateListItemSelectionAfterChange(selItems, m_internals->CurrentList);
    m_internals->AvailableList->setCurrentItem(nullptr);
    m_internals->AvailableList->clearSelection();
    this->updateAssociationStatus(att.get());
  }
}

void qtAssociation2ColumnWidget::removeObservers()
{
  if (m_operationObserverKey.assigned() && m_view)
  {
    auto opManager = m_view->uiManager()->operationManager();
    if (opManager != nullptr)
    {
      opManager->observers().erase(m_operationObserverKey);
    }
  }
  if (m_resourceObserverKey.assigned() && m_view)
  {
    auto resManager = m_view->uiManager()->resourceManager();
    if (resManager != nullptr)
    {
      resManager->observers().erase(m_resourceObserverKey);
    }
  }
}

void qtAssociation2ColumnWidget::updateListItemSelectionAfterChange(
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

int qtAssociation2ColumnWidget::handleOperationEvent(const smtk::operation::Operation& /*unused*/,
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
  emit this->availableChanged();
  return 0;
}

int qtAssociation2ColumnWidget::handleResourceEvent(
  const smtk::resource::Resource& resource, smtk::resource::EventType event)
{
  if (event == smtk::resource::EventType::REMOVED)
  {
    // The simplest solution is just to refresh the widget
    this->refreshAssociations(resource.id());
    emit this->availableChanged();
  }
  return 0;
}

void qtAssociation2ColumnWidget::leaveEvent(QEvent* evt)
{
  this->resetHover();
  // Now let the superclass do what it wants:
  QWidget::leaveEvent(evt);
}

void qtAssociation2ColumnWidget::hoverRow(const QModelIndex& idx)
{
  QListWidget* const listWidget = qobject_cast<QListWidget*>(QObject::sender());
  if (!listWidget)
  {
    return;
  }

  int row = idx.row();
  QListWidgetItem *item, *curItem;
  if (listWidget == m_internals->CurrentList)
  {
    item = m_internals->CurrentList->item(row);
    curItem = m_internals->CurrentList->currentItem();
  }
  else if (listWidget == m_internals->AvailableList)
  {
    item = m_internals->AvailableList->item(row);
    curItem = m_internals->AvailableList->currentItem();
  }
  else
  {
    return;
  }

  if ((item == m_internals->lastHighlightedItem) || (item == curItem))
  {
    return;
  }

  auto uiManager = m_view->uiManager();
  if (uiManager == nullptr)
  {
    return;
  }

  auto selection = uiManager->selection();
  if (selection == nullptr)
  {
    return;
  }

  // Lets create the hover background color;
  // If there was a previously highlighted item and it is still using the
  // hover color then reset its background;

  if (ALLOW_LIST_HIGHLIGHTING && (m_internals->lastHighlightedItem != nullptr))
  {
    m_internals->lastHighlightedItem->setBackground(m_internals->normalBackground);
  }

  // Discover what is currently hovered
  auto obj = idx.data(Qt::UserRole).value<smtk::resource::PersistentObjectPtr>();
  if (obj == nullptr)
  {
    return;
  }

  if (ALLOW_LIST_HIGHLIGHTING && item)
  {
    item->setBackground(
      QBrush(m_internals->CurrentList->palette().highlight().color().lighter(125)));
    m_internals->lastHighlightedItem = item;
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

void qtAssociation2ColumnWidget::resetHover()
{
  auto uiManager = m_view->uiManager();
  if (uiManager == nullptr)
  {
    return;
  }

  if (ALLOW_LIST_HIGHLIGHTING && (m_internals->lastHighlightedItem != nullptr))
  {
    m_internals->lastHighlightedItem->setBackground(m_internals->normalBackground);
    m_internals->lastHighlightedItem = nullptr;
  }
  auto selection = uiManager->selection();
  if (selection == nullptr)
  {
    return;
  }
  selection->resetSelectionBits(m_selectionSourceName, uiManager->hoverBit());
}

void qtAssociation2ColumnWidget::highlightOnHoverChanged(bool shouldHighlight)
{
  if (shouldHighlight)
  {
    QObject::connect(m_internals->CurrentList, SIGNAL(entered(const QModelIndex&)), this,
      SLOT(hoverRow(const QModelIndex&)), Qt::QueuedConnection);
    QObject::connect(m_internals->AvailableList, SIGNAL(entered(const QModelIndex&)), this,
      SLOT(hoverRow(const QModelIndex&)), Qt::QueuedConnection);
  }
  else
  {
    QObject::disconnect(m_internals->CurrentList, SIGNAL(entered(const QModelIndex&)), this,
      SLOT(hoverRow(const QModelIndex&)));
    QObject::disconnect(m_internals->AvailableList, SIGNAL(entered(const QModelIndex&)), this,
      SLOT(hoverRow(const QModelIndex&)));
    this->resetHover();
  }
}

void qtAssociation2ColumnWidget::onCurrentItemChanged(
  QListWidgetItem* item, QListWidgetItem* prevItem)
{
  // When something is selected we need to make sure that the
  // previous selected item is no longer using the hover background color
  // Also we need to make sure that the last highlighted item is set
  // to null so the next time hover row is called it knows there is
  // no item needing its background cleared.
  if (item == m_internals->lastHighlightedItem)
  {
    auto uiManager = m_view->uiManager();
    if (uiManager == nullptr)
    {
      return;
    }
    if (ALLOW_LIST_HIGHLIGHTING && (prevItem != nullptr))
    {
      prevItem->setBackground(m_internals->normalBackground);
    }
    m_internals->lastHighlightedItem = nullptr;
    auto selection = uiManager->selection();
    if (selection == nullptr)
    {
      return;
    }
    selection->resetSelectionBits(m_selectionSourceName, uiManager->hoverBit());
  }
}

void qtAssociation2ColumnWidget::setCurrentLabel(const std::string& message)
{
  m_internals->CurrentLabel->setText(message.c_str());
}
void qtAssociation2ColumnWidget::setAvailableLabel(const std::string& message)
{
  m_internals->AvailableLabel->setText(message.c_str());
}
void qtAssociation2ColumnWidget::setTitleLabel(const std::string& message)
{
  m_internals->TitleLabel->setText(message.c_str());
}
