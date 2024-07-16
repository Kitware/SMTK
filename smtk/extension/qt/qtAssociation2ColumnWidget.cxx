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
#include "smtk/attribute/utility/Queries.h"

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
} // namespace
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
  WeakDefinitionPtr currentDef;
  QPointer<qtBaseView> view;
  QListWidgetItem* lastHighlightedItem{ nullptr };
  QBrush normalBackground;
};

qtAssociation2ColumnWidget::qtAssociation2ColumnWidget(QWidget* _p, qtBaseView* bview)
  : qtAssociationWidget(_p, bview)
{
  m_isValid = true;
  m_allAssociatedWarning =
    "There are still components not associated to any of the above attributes.";
  m_internals = new qtAssociation2ColumnWidgetInternals;
  m_internals->setupUi(this);
  this->initWidget();

  // Are there any customizations?
  const smtk::view::Configuration::Component& config = m_view->configuration()->details();
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
  auto* uiManager = m_view->uiManager();
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
      [guardedObject](
        const smtk::operation::Operation& oper,
        smtk::operation::EventType event,
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
  QObject::connect(
    m_internals->CurrentList,
    SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
    this,
    SLOT(onCurrentItemChanged(QListWidgetItem*, QListWidgetItem*)),
    Qt::QueuedConnection);

  QObject::connect(
    m_internals->AvailableList,
    SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
    this,
    SLOT(onCurrentItemChanged(QListWidgetItem*, QListWidgetItem*)),
    Qt::QueuedConnection);

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
  this->setIsValid(true);

  // signals/slots
  QObject::connect(
    m_internals->MoveToRight,
    SIGNAL(clicked()),
    this,
    SLOT(onRemoveAssigned()),
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
  m_internals->currentDef.reset();
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

void qtAssociation2ColumnWidget::showEntityAssociation(smtk::attribute::DefinitionPtr theDef)
{
  m_internals->currentAtt.reset();
  m_internals->currentDef = theDef;
  this->refreshAssociations();

  // Lets store the normal background color of an item - we will
  // need to use this when dealing with hovering
  if (m_internals->AvailableList->count())
  {
    m_internals->normalBackground = m_internals->AvailableList->item(0)->background();
  }
}

void qtAssociation2ColumnWidget::setIsValid(bool val)
{
  m_isValid = val;
  m_internals->AlertLabel->setVisible(!val);
}

bool qtAssociation2ColumnWidget::isValid() const
{
  return m_isValid;
}

void qtAssociation2ColumnWidget::updateAssociationStatus(const Attribute* attribute)
{
  QString reason;
  if (attribute)
  {
    auto assocItem = attribute->associatedObjects();
    bool assocValid = assocItem->isValid();
    if (assocValid && !(m_allAssociatedMode && m_internals->AvailableList->count()))
    {
      this->setIsValid(true);
      return;
    }
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
          reason = "Attribute requires no more than ";
          QString count;
          count.setNum(assocItem->maxNumberOfValues());
          reason.append(count).append(" objects associated to it");
        }
      }
      else
      {
        reason = "Attribute requires ";
        QString count;
        count.setNum(assocItem->numberOfRequiredValues());
        reason.append(count).append(" objects associated to it");
      }
    }
  }
  else if (!(m_allAssociatedMode && m_internals->AvailableList->count()))
  {
    this->setIsValid(true);
    return;
  }

  // If we are here there is a problems
  this->setIsValid(false);

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
  attribute::DefinitionPtr attDef;

  // If we are dealing with an attribute and the UI Manager is not read-only,
  // we need to turn on the buttons that change association info - else they need to be off
  if (theAttribute && !m_view->uiManager()->isReadOnly())
  {
    attDef = theAttribute->definition();
    m_internals->MoveToRight->setEnabled(true);
    m_internals->MoveToLeft->setEnabled(true);
  }
  else
  {
    attDef = m_internals->currentDef.lock();
    m_internals->MoveToRight->setEnabled(false);
    m_internals->MoveToLeft->setEnabled(false);
  }

  // If there is no attribute definition we just return
  if (!attDef)
  {
    m_internals->CurrentList->blockSignals(false);
    m_internals->AvailableList->blockSignals(false);
    this->setIsValid(true);
    return;
  }

  ResourcePtr attResource = attDef->attributeResource();
  // If this resource is marked for removal there is nothing to be done
  if (attResource->isMarkedForRemoval())
  {
    m_internals->CurrentList->blockSignals(false);
    m_internals->AvailableList->blockSignals(false);
    return;
  }
  auto resManager = m_view->uiManager()->resourceManager();
  // Lets get the objects that can possibly be associated with the attribute/definition
  if (theAttribute)
  {
    auto associationItem = theAttribute->associatedObjects();
    // Lets make sure the association item is clear of invalid objects
    associationItem->removeInvalidValues();
    auto objects =
      attribute::utility::associatableObjects(associationItem, resManager, false, ignoreResource);

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
  }
  else // We are dealing with potential associations based on a definition only
  {
    auto associationItemDef = attDef->associationRule();
    smtk::attribute::DefinitionPtr preDef;
    smtk::attribute::AttributePtr conAtt;
    auto objects = attribute::utility::associatableObjects(
      associationItemDef, attResource, resManager, ignoreResource);
    // Now lets see if the objects can be associated with this type of attribute
    for (const auto& obj : objects)
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

QListWidgetItem* qtAssociation2ColumnWidget::addObjectAssociationListItem(
  QListWidget* theList,
  const smtk::resource::PersistentObjectPtr& object,
  bool sort,
  bool appendResourceName)
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
  QListWidget* theList,
  smtk::attribute::AttributePtr att,
  bool sort)
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
  Q_FOREACH (QListWidgetItem* item, selItems)
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
    Q_EMIT this->attAssociationChanged();
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
  Q_FOREACH (QListWidgetItem* item, selItems)
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
    Q_EMIT this->attAssociationChanged();
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
  QList<QListWidgetItem*> selItems,
  QListWidget* list)
{
  list->blockSignals(true);
  Q_FOREACH (QListWidgetItem* item, selItems)
  {
    QList<QListWidgetItem*> findList = list->findItems(item->text(), Qt::MatchExactly);
    Q_FOREACH (QListWidgetItem* findItem, findList)
    {
      findItem->setSelected(true);
    }
  }
  list->blockSignals(false);
}

int qtAssociation2ColumnWidget::handleOperationEvent(
  const smtk::operation::Operation& /*unused*/,
  smtk::operation::EventType event,
  smtk::operation::Operation::Result result)
{
  if (event != smtk::operation::EventType::DID_OPERATE)
  {
    return 0;
  }

  auto theAttribute = m_internals->currentAtt.lock();
  attribute::DefinitionPtr attDef;

  if (theAttribute)
  {
    attDef = theAttribute->definition();
  }
  else
  {
    attDef = m_internals->currentDef.lock();
  }

  // If we have no current attribute definition then there is nothing we
  // need to update
  if (attDef == nullptr)
  {
    return 0;
  }
  // Lets get the definition's association rule
  auto associationRuleDef = attDef->associationRule();
  if (associationRuleDef == nullptr)
  {
    return 0;
  }

  bool needToRefresh = false;

  smtk::attribute::ComponentItemPtr compItem;
  std::size_t i, n;
  // Lets first go through the modified list.  If the component modified is either
  // the current attribute or a component that can be associated with the current attribute
  // then we need to refresh

  compItem = result->findComponent("modified");
  n = compItem->numberOfValues();
  for (i = 0; i < n; i++)
  {
    if (!compItem->isSet(i))
    {
      continue;
    }

    if (
      (dynamic_pointer_cast<smtk::attribute::Attribute>(compItem->value(i)) == theAttribute) ||
      (associationRuleDef->isValueValid(compItem->value(i))))
    {
      needToRefresh = true;
      break;
    }
  }

  // Lets see if we need to refresh due to expunged components
  if (!needToRefresh)
  {
    // In the case of expunged components, we only need to see if any can be associated
    compItem = result->findComponent("expunged");
    n = compItem->numberOfValues();
    for (i = 0; i < n; i++)
    {
      if (!compItem->isSet(i))
      {
        continue;
      }

      if (associationRuleDef->isValueValid(compItem->value(i)))
      {
        needToRefresh = true;
        break;
      }
    }
  }

  // Lets see if we need to refresh due to created components
  if (!needToRefresh)
  {
    // In the case of created components, we only need to see if any can be associated
    compItem = result->findComponent("created");
    n = compItem->numberOfValues();
    for (i = 0; i < n; i++)
    {
      if (!compItem->isSet(i))
      {
        continue;
      }

      if (associationRuleDef->isValueValid(compItem->value(i)))
      {
        needToRefresh = true;
        break;
      }
    }
  }

  if (needToRefresh)
  {
    this->refreshAssociations();
    Q_EMIT this->availableChanged();
  }
  return 0;
}

int qtAssociation2ColumnWidget::handleResourceEvent(
  const smtk::resource::Resource& /*resource*/,
  smtk::resource::EventType event)
{
  if (event == smtk::resource::EventType::REMOVED)
  {
    // The simplest solution is just to refresh the widget
    this->refreshAssociations();
    Q_EMIT this->availableChanged();
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

  auto* uiManager = m_view->uiManager();
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
  auto* uiManager = m_view->uiManager();
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
    QObject::connect(
      m_internals->CurrentList,
      SIGNAL(entered(const QModelIndex&)),
      this,
      SLOT(hoverRow(const QModelIndex&)),
      Qt::QueuedConnection);
    QObject::connect(
      m_internals->AvailableList,
      SIGNAL(entered(const QModelIndex&)),
      this,
      SLOT(hoverRow(const QModelIndex&)),
      Qt::QueuedConnection);
  }
  else
  {
    QObject::disconnect(
      m_internals->CurrentList,
      SIGNAL(entered(const QModelIndex&)),
      this,
      SLOT(hoverRow(const QModelIndex&)));
    QObject::disconnect(
      m_internals->AvailableList,
      SIGNAL(entered(const QModelIndex&)),
      this,
      SLOT(hoverRow(const QModelIndex&)));
    this->resetHover();
  }
}

void qtAssociation2ColumnWidget::onCurrentItemChanged(
  QListWidgetItem* item,
  QListWidgetItem* prevItem)
{
  // When something is selected we need to make sure that the
  // previous selected item is no longer using the hover background color
  // Also we need to make sure that the last highlighted item is set
  // to null so the next time hover row is called it knows there is
  // no item needing its background cleared.
  if (item == m_internals->lastHighlightedItem)
  {
    auto* uiManager = m_view->uiManager();
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
