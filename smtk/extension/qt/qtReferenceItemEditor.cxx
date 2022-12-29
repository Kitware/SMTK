//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtReferenceItemEditor.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtSMTKUtilities.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/utility/Queries.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/resource/Manager.h"

#include "smtk/view/Selection.h"

#include <QCheckBox>
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

#define DEBUG_REFERENCEITEM 0

using namespace smtk::attribute;
using namespace smtk::extension;

class qtReferenceItemEditorInternals
{
public:
  QPointer<QComboBox> m_comboBox;
  QPointer<QLabel> m_theLabel;
  QPointer<QVBoxLayout> m_mainLayout;

  // For managing active children
  QPointer<QFrame> m_childrenFrame;
  QList<smtk::extension::qtItem*> m_childItems;
  int m_hintChildWidth;
  int m_hintChildHeight;
  std::map<std::string, qtAttributeItemInfo> m_itemViewMap;

  smtk::resource::WeakManagerPtr m_resourceManager;
  smtk::operation::WeakManagerPtr m_operationManager;

  ~qtReferenceItemEditorInternals() { this->clearChildItems(); }

  // Removes all of the qItems representing active children
  void clearChildItems()
  {
    for (int i = 0; i < m_childItems.count(); i++)
    {
      delete m_childItems.value(i);
    }
    m_childItems.clear();
  }
};

qtItem* qtReferenceItemEditor::createItemWidget(const qtAttributeItemInfo& info)
{
  // So we support this type of item?
  if (info.itemAs<smtk::attribute::ReferenceItem>() == nullptr)
  {
    return nullptr;
  }
  auto* qi = new qtReferenceItemEditor(info);
  return qi;
}

qtReferenceItemEditor::qtReferenceItemEditor(const qtAttributeItemInfo& info)
  : qtItem(info)
  , m_okToCreate(false)
  , m_useAssociations(false)
{
  m_internals = new qtReferenceItemEditorInternals;
  // Set up the item view map for the active children
  m_itemInfo.createNewDictionary(m_internals->m_itemViewMap);

  // See if we are suppose to limit selection only to this associated
  // with this item's attribute
  m_useAssociations = m_itemInfo.component().attributeAsBool("UseAssociations");
  if (m_useAssociations)
  {
    QObject::connect(
      m_itemInfo.baseView(),
      &qtBaseAttributeView::modified,
      this,
      &qtReferenceItemEditor::itemChanged);
  }

  std::ostringstream receiverSource;
  receiverSource << "qtReferenceItemEditor_" << this;
  m_selectionSourceName = receiverSource.str();
  auto* uiManager = this->uiManager();
  if (uiManager == nullptr)
  {
#if !defined(NDEBUG)
    std::cerr << "qtReferenceItemEditor: Could not find UI Manager!\n";
#endif
    return;
  }

  QObject::connect(
    uiManager,
    &qtUIManager::highlightOnHoverChanged,
    this,
    &qtReferenceItemEditor::highlightOnHoverChanged);

  auto opManager = uiManager->operationManager();
  if (opManager != nullptr)
  {
    QPointer<qtReferenceItemEditor> guardedObject(this);
    m_operationObserverKey = opManager->observers().insert(
      [guardedObject](
        const smtk::operation::Operation& oper,
        smtk::operation::EventType event,
        smtk::operation::Operation::Result result) -> int {
        if (guardedObject)
        {
          return guardedObject->handleOperationEvent(oper, event, result);
        }
        return 0;
      },
      "qtReferenceItemEditor: Update if an operation adds or removes entries.");
    m_internals->m_operationManager = opManager;
  }
  else
  {
#if !defined(NDEBUG) && DEBUG_REFERENCEITEM
    std::cerr << "qtReferenceItemEditor: Could not find Operation Manager!\n";
#endif
  }
  auto resManager = uiManager->resourceManager();
  if (resManager != nullptr)
  {
    m_resourceObserverKey = resManager->observers().insert(
      [this](const smtk::resource::Resource& resource, smtk::resource::EventType event) {
        this->handleResourceEvent(resource, event);
      },
      "qtReferenceItemEditor: Update if a resource is added or removed.");
    m_internals->m_resourceManager = resManager;
  }
  else
  {
#if !defined(NDEBUG) && DEBUG_REFERENCEITEM
    std::cerr << "qtReferenceItemEditor: Could not find Resource Manager!\n";
#endif
  }
  this->createWidget();
  this->highlightOnHoverChanged(uiManager->highlightOnHover());
}

qtReferenceItemEditor::~qtReferenceItemEditor()
{
  this->removeObservers();
  delete m_internals;
}

void qtReferenceItemEditor::markForDeletion()
{
  this->removeObservers();
  qtItem::markForDeletion();
}

void qtReferenceItemEditor::createWidget()
{
  auto item = m_itemInfo.itemAs<attribute::ReferenceItem>();
  if (item == nullptr)
  {
    return;
  }

  // Lets setup the main widget and layout
  m_widget = new QFrame(m_itemInfo.parentWidget());
  m_widget->setObjectName(item->name().c_str());
  if (this->isReadOnly())
  {
    m_widget->setEnabled(false);
  }
  m_internals->m_mainLayout = new QVBoxLayout(m_widget);
  m_internals->m_mainLayout->setMargin(0);
  m_internals->m_mainLayout->setSpacing(0);
  m_internals->m_mainLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QFrame* comboFrame = new QFrame(m_widget);
  comboFrame->setObjectName("Combo Frame");
  QHBoxLayout* entryLayout = new QHBoxLayout(comboFrame);
  entryLayout->setMargin(0);
  entryLayout->setSpacing(0);
  entryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if (item->isOptional())
  {
    QCheckBox* optionalCheck = new QCheckBox(comboFrame);
    optionalCheck->setObjectName("OptionalCheck");
    optionalCheck->setChecked(item->localEnabledState());
    optionalCheck->setText(" ");
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = optionalCheck->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(
      optionalCheck, &QCheckBox::stateChanged, this, &qtReferenceItemEditor::setOutputOptional);
    labelLayout->addWidget(optionalCheck);
  }
  auto itemDef = item->definitionAs<attribute::ReferenceItemDefinition>();

  QString labelText = item->label().c_str();
  QLabel* label = new QLabel(labelText, comboFrame);
  label->setObjectName("ReferenceItemLabel");
  label->setSizePolicy(sizeFixedPolicy);
  auto* iview = m_itemInfo.baseView();
  if (iview)
  {
    label->setFixedWidth(iview->fixedLabelWidth() - padding);
  }
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  //  qtOverlayFilter *filter = new qtOverlayFilter(this);
  //  label->installEventFilter(filter);

  // add in BriefDescription as tooltip if available
  const std::string strBriefDescription = itemDef->briefDescription();
  if (!strBriefDescription.empty())
  {
    label->setToolTip(strBriefDescription.c_str());
  }

  if (itemDef->advanceLevel() && m_itemInfo.baseView())
  {
    label->setFont(m_itemInfo.uiManager()->advancedFont());
  }
  labelLayout->addWidget(label);

  if (label->text().trimmed().isEmpty())
  {
    label->setVisible(false);
  }

  m_internals->m_theLabel = label;
  m_internals->m_comboBox = new QComboBox(comboFrame);
  m_internals->m_comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  m_internals->m_comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_internals->m_comboBox->setObjectName("Ref ComboBox");
  this->updateItemData();
  // signals/slots
  QObject::connect(
    m_internals->m_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectItem(int)));

  entryLayout->addLayout(labelLayout);
  entryLayout->addWidget(m_internals->m_comboBox);
  m_internals->m_mainLayout->addWidget(comboFrame);
  if (m_itemInfo.parentWidget() && m_itemInfo.parentWidget()->layout())
  {
    m_itemInfo.parentWidget()->layout()->addWidget(m_widget);
  }
  if (item->isOptional())
  {
    this->setOutputOptional(item->localEnabledState() ? 1 : 0);
  }
  this->updateContents();
}

void qtReferenceItemEditor::updateItemData()
{
  this->updateChoices();
  // Make sure the children have been updated
  int n = m_internals->m_childItems.size();
  for (int i = 0; i < n; i++)
  {
    m_internals->m_childItems.at(i)->updateItemData();
  }
}

void qtReferenceItemEditor::updateChoices(const smtk::common::UUID& ignoreResource)
{
  auto item = m_itemInfo.itemAs<attribute::ReferenceItem>();
  // If there is no combox being displayed there is nothing to be done/
  // This can occur if the parent widget has been deleted (which means this
  // instance will be deleted shortly)
  if (m_internals->m_comboBox == nullptr)
  {
    return;
  }

  m_internals->m_comboBox->blockSignals(true);
  this->m_mappedObjects.clear();
  m_internals->m_comboBox->clear();
  if (!item)
  {
    m_internals->m_comboBox->blockSignals(false);
    return;
  }
  auto itemDef = item->definitionAs<attribute::ReferenceItemDefinition>();
  auto theAttribute = item->attribute();

  attribute::DefinitionPtr attDef = theAttribute->definition();

  ResourcePtr attResource = attDef->resource();
  // Lets get a set of possible candidates that could be assigned to the item
  auto resManager = this->uiManager()->resourceManager();

  auto objSet = smtk::attribute::utility::associatableObjects(
    item, resManager, m_useAssociations, ignoreResource);

  // Do we have to apply categories to this result?
  if (itemDef->enforcesCategories() && m_itemInfo.uiManager())
  {
    std::set<smtk::resource::PersistentObjectPtr> toBeRemoved;
    for (const auto& obj : objSet)
    {
      auto att = std::dynamic_pointer_cast<attribute::Attribute>(obj);
      if (att && !att->isRelevant())
      {
        toBeRemoved.insert(att);
      }
    }
    // OK - lets remove all attributes that failed the category check
    for (const auto& obj : toBeRemoved)
    {
      objSet.erase(obj);
    }
  }
  // There are cases when the item's current color value may not be currently legal due to uniqueness
  // and/or category constraints.  In the case of the uniqueness condition,
  // the componentItem's value itself may not be in the set
  // returned (since adding that component would not be legal or perhaps an operation has assigned a
  // component that would bypass the potential sources of components.  For example, the component
  // may have been assigned from a resource that was not directly associated to the attribute resource.
  // Just to be safe lets add the item's current value (if set)
  m_currentValueIsInvalid = false;
  if (item->isSet() && item->value())
  {
    auto it = objSet.find(item->value());
    if (it == objSet.end())
    {
      objSet.insert(item->value());
      m_currentValueIsInvalid = true;
    }
  }

  std::vector<smtk::resource::PersistentObjectPtr> objects(objSet.begin(), objSet.end());
  smtk::resource::PersistentObjectPtr selectObj = item->value();
  // Lets sort the list
  std::sort(
    std::begin(objects),
    std::end(objects),
    [](smtk::resource::PersistentObjectPtr a, smtk::resource::PersistentObjectPtr b) {
      return a->name() < b->name();
    });

  // Lets set the comboBox
  // First add the please select option
  m_internals->m_comboBox->addItem("Please Select");
  m_internals->m_comboBox->setItemData(0, QBrush(Qt::red), Qt::ForegroundRole);
  // Are we allowed to create an object?
  if (m_okToCreate)
  {
    m_internals->m_comboBox->addItem("Create New");
  }

  int count = 0;
  int selectedIndex = 0;
  for (auto& obj : objects)
  {
    QVariant vdata;
    vdata.setValue(count);
    if (obj == selectObj)
    {
      selectedIndex = m_internals->m_comboBox->count();
    }
    // TODO: right now the descriptive phrase stuff displays New Resource for
    // unnamed resources so we will do the same.  However, the code should be
    // re-factored so that the logic for calculating names resides in one place
    // Also - the current approach doesn't work when there are more than 1
    // unnamed resources.
    if (obj->name().empty())
    {
      m_internals->m_comboBox->addItem("New Resource", vdata);
    }
    else
    {
      m_internals->m_comboBox->addItem(obj->name().c_str(), vdata);
    }
    if (m_currentValueIsInvalid)
    {
      m_internals->m_comboBox->setItemData(selectedIndex, QBrush(Qt::red), Qt::ForegroundRole);
    }
    this->m_mappedObjects[count++] = obj;
  }
  m_internals->m_comboBox->setCurrentIndex(selectedIndex);
  if ((selectedIndex == 0) || m_currentValueIsInvalid)
  {
    QPalette comboboxPalette = m_internals->m_comboBox->palette();
    // On Macs to change the button text color you need to set QPalette::Text
    // For Linux you need to set QPalette::ButtonText
    comboboxPalette.setColor(QPalette::ButtonText, Qt::red);
    comboboxPalette.setColor(QPalette::Text, Qt::red);
    m_internals->m_comboBox->setPalette(comboboxPalette);
  }
  else
  {
    m_internals->m_comboBox->setPalette(m_internals->m_comboBox->parentWidget()->palette());
  }
  m_internals->m_comboBox->blockSignals(false);
}

smtk::resource::PersistentObjectPtr qtReferenceItemEditor::object(int index)
{
  // Lets get the right index
  // Are we dealing with the create new option
  if ((index <= 0) || (m_okToCreate && (index == 1)))
  {
    return nullptr; // These are entries without items associated with them
  }
  bool ok;
  int val = m_internals->m_comboBox->itemData(index).toInt(&ok);
  if (!ok)
  {
// There is a problem in that the item doesn't have a mapped value for us to look up
#if !defined(NDEBUG)
    std::cerr << "qtReferenceItemEditor::object - can't get mapped id for index = " << index
              << "\n";
#endif
    return nullptr;
  }
  auto findResult = m_mappedObjects.find(val);
  if (findResult == m_mappedObjects.end())
  {
// There is a problem in that the mapped value can't be found
#if !defined(NDEBUG)
    std::cerr << "qtReferenceItemEditor::object - can't find mapped id\n";
#endif
    return nullptr;
  }
  auto selectedObj = findResult->second.lock();
  if (selectedObj == nullptr)
  {
// There is a problem in that we can't get a persistent object from the weak pointer
#if !defined(NDEBUG)
    std::cerr << "qtReferenceItemEditor::object - can't get PersistentObject\n";
#endif
    return nullptr;
  }
  return selectedObj;
}

QWidget* qtReferenceItemEditor::lastEditor() const
{
  return m_internals->m_comboBox;
}

void qtReferenceItemEditor::setPreviousEditor(QWidget* widget)
{
  if (m_internals->m_comboBox != nullptr)
  {
    QWidget::setTabOrder(widget, m_internals->m_comboBox);
  }
}

void qtReferenceItemEditor::highlightItem(int index)
{
  // Are we dealing with the create new option
  if ((index == 0) || (m_okToCreate && (index == 1)))
  {
    return; // Nothing to highlight
  }
  // If there is no selection manager then there is nothing
  // we need to do
  auto* uiManager = this->uiManager();
  if (uiManager == nullptr)
  {
    return;
  }

  auto selection = uiManager->selection();
  if (selection == nullptr)
  {
    return;
  }
  // First lets see if this is something we can highlight
  if ((index <= 0) || ((index == 1) && m_okToCreate))
  {
    // In this case the user has selected something that corresponds
    // to a special action (unset or create) so there is nothing to color
    selection->resetSelectionBits(m_selectionSourceName, uiManager->hoverBit());
  }
  // Discover what is currently hovered
  auto selectedObject = this->object(index);
  if (selectedObject == nullptr)
  {
    // There is a problem in that we can't get a persistent object from the index
    selection->resetSelectionBits(m_selectionSourceName, uiManager->hoverBit());
#if !defined(NDEBUG)
    std::cerr << "qtReferenceItemEditor::highlightItem - can't get PersistentObject for index: "
              << index << std::endl;
#endif
    return;
  }
  // Add new hover state
  auto hoverMask = uiManager->hoverBit();
  const auto& selnMap = selection->currentSelection();
  auto cvit = selnMap.find(selectedObject);
  int sv = (cvit == selnMap.end() ? 0 : cvit->second) | hoverMask;
  smtk::resource::PersistentObjectSet objs;
  objs.insert(selectedObject);
  selection->modifySelection(
    objs, m_selectionSourceName, sv, smtk::view::SelectionAction::UNFILTERED_REPLACE, true);
}

void qtReferenceItemEditor::selectItem(int index)
{
  // Check to see if the new value is red or not - if it is
  // then the combo box button should be red.
  if (m_internals->m_comboBox->itemData(index, Qt::ForegroundRole) == QBrush(Qt::red))
  {
    QPalette comboboxPalette = m_internals->m_comboBox->palette();
    // On Macs to change the button text color you need to set QPalette::Text
    // For Linux you need to set QPalette::ButtonText
    comboboxPalette.setColor(QPalette::ButtonText, Qt::red);
    comboboxPalette.setColor(QPalette::Text, Qt::red);
    m_internals->m_comboBox->setPalette(comboboxPalette);
  }
  else
  {
    m_internals->m_comboBox->setPalette(m_internals->m_comboBox->parentWidget()->palette());
  }

  // Lets get the selection manager if possible since
  // we want to clear anything being highlighted

  smtk::extension::qtUIManager* uiManager = this->uiManager();
  smtk::view::SelectionPtr selection;
  if (uiManager)
  {
    selection = uiManager->selection();
  }

  auto item = m_itemInfo.itemAs<attribute::ReferenceItem>();
  if (item == nullptr)
  {
    // Clear the selection if we can
    if (selection)
    {
      selection->resetSelectionBits(m_selectionSourceName, uiManager->hoverBit());
    }
    return;
  }
  // Is the "Please Select" option chosen?  If so then unset the item if needed
  if (index <= 0)
  {
    if (item->isSet())
    {
      item->unset();
      this->updateContents();
      Q_EMIT this->modified();
    }
  }
  // Are we dealing with the create new option
  else if (m_okToCreate && (index == 1))
  {
    // ToDo:  Implemented!
  }
  else // Will this change the item?
  {
    auto selectedObject = this->object(index);
    if (selectedObject && !(item->isSet() && (item->value() == selectedObject)))
    {
      item->setValue(selectedObject);
      this->updateContents();
      Q_EMIT this->modified();
    }
  }

  // Clear the selection if we can
  if (selection)
  {
    selection->resetSelectionBits(m_selectionSourceName, uiManager->hoverBit());
  }

  // if the previous value was considered invalid lets refresh the choices
  // which should remove the invalid entry from the list of choices
  if (m_currentValueIsInvalid)
  {
    this->updateChoices();
  }
}

void qtReferenceItemEditor::removeObservers()
{
  if (m_operationObserverKey.assigned())
  {
    auto opManager = m_internals->m_operationManager.lock();
    if (opManager != nullptr)
    {
      opManager->observers().erase(m_operationObserverKey);
    }
  }
  if (m_resourceObserverKey.assigned())
  {
    auto resManager = m_internals->m_resourceManager.lock();
    if (resManager != nullptr)
    {
      resManager->observers().erase(m_resourceObserverKey);
    }
  }
}

int qtReferenceItemEditor::handleOperationEvent(
  const smtk::operation::Operation& /*unused*/,
  smtk::operation::EventType event,
  smtk::operation::Operation::Result result)
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
  this->updateChoices();
  return 0;
}

void qtReferenceItemEditor::handleResourceEvent(
  const smtk::resource::Resource& resource,
  smtk::resource::EventType event)
{

  auto item = m_itemInfo.itemAs<attribute::ReferenceItem>();
  if (item == nullptr)
  {
    return;
  }
  auto theAttribute = item->attribute();
  auto attResource = theAttribute->attributeResource();

  // If this resource is marked for removal, then we don't need to update this widget since
  // it should be deleted shortly - this will also prevent the resource's links system from
  // pulling in associated resources unnecessarily
  if (attResource->isMarkedForRemoval())
  {
    return;
  }

  if ((event == smtk::resource::EventType::REMOVED) && (attResource->id() != resource.id()))
  {
    // The simplest solution is just to refresh the widget
    this->updateChoices(resource.id());
  }
}

void qtReferenceItemEditor::resetHover()
{
  auto* uiManager = this->uiManager();
  if (uiManager == nullptr)
  {
    return;
  }

  auto selection = uiManager->selection();
  if (selection == nullptr)
  {
    return;
  }
  selection->resetSelectionBits(m_selectionSourceName, uiManager->hoverBit());
}

void qtReferenceItemEditor::setOutputOptional(int state)
{
  auto item = m_itemInfo.itemAs<smtk::attribute::ReferenceItem>();
  if (!item)
  {
    return;
  }
  bool enable = state != 0;
  m_internals->m_comboBox->setVisible(enable);
  if (enable != item->localEnabledState())
  {
    item->setIsEnabled(enable);
    auto* iview = m_itemInfo.baseView();
    if (iview)
    {
      iview->valueChanged(item);
    }
    Q_EMIT this->modified();
  }
}

void qtReferenceItemEditor::highlightOnHoverChanged(bool shouldHighlight)
{
  if (shouldHighlight)
  {
    QObject::connect(
      m_internals->m_comboBox, SIGNAL(highlighted(int)), this, SLOT(highlightItem(int)));
  }
  else
  {
    QObject::disconnect(
      m_internals->m_comboBox, SIGNAL(highlighted(int)), this, SLOT(highlightItem(int)));
    this->resetHover();
  }
}

void qtReferenceItemEditor::itemChanged(smtk::attribute::ItemPtr modifiedItem)
{
  smtk::attribute::ItemPtr item = m_itemInfo.item();
  if ((!item) || (item == modifiedItem))
  {
    return;
  }

  if (item->attribute() != modifiedItem->attribute())
  {
    return;
  }
  this->updateChoices();
}

void qtReferenceItemEditor::updateContents()
{
  auto* uiManager = this->uiManager();
  if (uiManager == nullptr)
  {
    return;
  }

  smtk::attribute::ResourcePtr attResource = this->attributeResource();

  // First clear all of the current children items being displayed
  m_internals->clearChildItems();
  if (m_internals->m_childrenFrame)
  {
    m_internals->m_mainLayout->removeWidget(m_internals->m_childrenFrame);
    delete m_internals->m_childrenFrame;
  }

  auto item = this->itemAs<attribute::ReferenceItem>();
  auto itemDef = item->definitionAs<attribute::ReferenceItemDefinition>();

  if (item->numberOfActiveChildrenItems() == 0)
  {
    // Nothing else to do but tell the outside world our size may have changed
    m_itemInfo.baseView()->childrenResized();
    Q_EMIT this->widgetSizeChanged();
  }

  m_internals->m_hintChildWidth = 0;
  m_internals->m_hintChildHeight = 0;

  // Prepare the frame to hold all of the children
  m_internals->m_childrenFrame = new QFrame(m_widget);
  m_internals->m_childrenFrame->setObjectName("ChildItemsFrame");
  QSizePolicy sizeFixedPolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  QVBoxLayout* clayout = new QVBoxLayout(m_internals->m_childrenFrame);
  clayout->setMargin(3);
  m_internals->m_childrenFrame->setSizePolicy(sizeFixedPolicy);
  m_internals->m_childrenFrame->setFrameShape(QFrame::Box);

  // Determine the active children definitions that pass category checks
  // We do this to get an idea as to the max label length
  QList<smtk::attribute::ItemDefinitionPtr> activeChildDefs;
  std::size_t i, m = item->numberOfActiveChildrenItems();
  for (i = 0; i < m; i++)
  {
    smtk::attribute::ConstItemDefinitionPtr itDef = item->activeChildItem(i)->definition();
    std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator it =
      itemDef->childrenItemDefinitions().find(itDef->name());
    if (
      (it != itemDef->childrenItemDefinitions().end()) && attResource &&
      attResource->passActiveCategoryCheck(itemDef->categories()))
    {
      activeChildDefs.push_back(it->second);
    }
  }

  auto* iiview = m_itemInfo.baseView();
  int currentLen = iiview ? iiview->fixedLabelWidth() : 0;

  int tmpLen = uiManager->getWidthOfItemsMaxLabel(activeChildDefs, uiManager->advancedFont());
  if (iiview)
  {
    iiview->setFixedLabelWidth(tmpLen);
  }

  // Now process the active children items themselves
  bool hasVisibleChildren = false;
  for (i = 0; i < m; i++)
  {
    auto citem = item->activeChildItem(static_cast<int>(i));
    if (iiview && !iiview->displayItem(citem))
    {
      continue; // This child does not pass display checks so skip it
    }
    auto it = m_internals->m_itemViewMap.find(citem->name());
    qtItem* childItem;
    // Do we have custom view info for the child?
    if (it != m_internals->m_itemViewMap.end())
    {
      auto info = it->second;
      info.setParentWidget(m_internals->m_childrenFrame.data());
      info.setItem(citem);
      childItem = uiManager->createItem(info);
    }
    else
    {
      smtk::view::Configuration::Component comp; // create a default view style
      qtAttributeItemInfo info(
        citem, comp, m_internals->m_childrenFrame.data(), m_itemInfo.baseView());
      childItem = uiManager->createItem(info);
    }
    if (childItem)
    {
      clayout->addWidget(childItem->widget());
      m_internals->m_childItems.push_back(childItem);
      connect(childItem, SIGNAL(modified()), this, SIGNAL(modified()));
      connect(childItem, SIGNAL(widgetSizeChanged()), this, SIGNAL(widgetSizeChanged()));
      hasVisibleChildren = true;
    }
  }

  if (iiview)
  {
    iiview->setFixedLabelWidth(currentLen);
  }
  m_internals->m_hintChildWidth = m_internals->m_childrenFrame->width();
  m_internals->m_hintChildHeight = m_internals->m_childrenFrame->height();
  m_internals->m_childrenFrame->setVisible(hasVisibleChildren);
  m_internals->m_mainLayout->addWidget(m_internals->m_childrenFrame);

  m_itemInfo.baseView()->childrenResized();
  Q_EMIT this->widgetSizeChanged();
}
