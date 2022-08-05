//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtDiscreteValueEditor.h"

#include "smtk/extension/qt/qtInputsItem.h"

#include <QAction>
#include <QComboBox>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QPointer>

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtUIManager.h"

using namespace smtk::extension;

class qtDiscreteValueEditorInternals
{
public:
  qtDiscreteValueEditorInternals(qtInputsItem* item, int elementIdx, QLayout* parentLayout)
    : m_inputItem(item)
    , m_elementIndex(elementIdx)
    , m_parentLayout(parentLayout)
  {
    m_hintChildWidth = 0;
    m_hintChildHeight = 0;
  }

  QPointer<qtInputsItem> m_inputItem;
  int m_elementIndex;
  QPointer<QComboBox> m_combo;
  QPointer<QFrame> m_childrenFrame;

  QList<smtk::extension::qtItem*> m_childItems;
  QPointer<QLayout> m_parentLayout;
  int m_hintChildWidth;
  int m_hintChildHeight;
  std::map<std::string, qtAttributeItemInfo> m_itemViewMap;

  void clearChildItems()
  {
    for (int i = 0; i < m_childItems.count(); i++)
    {
      delete m_childItems.value(i);
    }
    m_childItems.clear();
  }
};

qtDiscreteValueEditor::qtDiscreteValueEditor(
  qtInputsItem* item,
  int elementIdx,
  QLayout* parentLayout)
  : QWidget(item->widget())
  , m_useSelectionManager(false)
{
  this->Internals = new qtDiscreteValueEditorInternals(item, elementIdx, parentLayout);
  if (item != nullptr)
  {
    item->m_itemInfo.createNewDictionary(this->Internals->m_itemViewMap);
  }

  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  this->createWidget();
}

qtDiscreteValueEditor::~qtDiscreteValueEditor()
{
  this->Internals->clearChildItems();
  delete this->Internals;
}

void qtDiscreteValueEditor::createWidget()
{
  smtk::attribute::ResourcePtr attResource = this->Internals->m_inputItem->attributeResource();
  auto* uiManager = this->Internals->m_inputItem->uiManager();

  smtk::attribute::ValueItemPtr item = this->Internals->m_inputItem->itemAs<attribute::ValueItem>();
  if (!item)
  {
    return;
  }
  this->Internals->clearChildItems();
  QBoxLayout* wlayout = new QVBoxLayout(this);
  wlayout->setMargin(0);
  wlayout->setAlignment(Qt::AlignTop);
  if (!item || !item->isDiscrete())
  {
    return;
  }

  std::size_t n = item->numberOfValues();
  if (!n)
  {
    return;
  }

  auto itemDef = item->definitionAs<attribute::ValueItemDefinition>();
  QString tooltip;
  QComboBox* combo = new QComboBox(this);
  // When building the combobox using the following guidelines:
  // * All entries should set UserRole data to indicate the corresponding
  //   discrete index that value corresponds to.
  // * The first entry should be "Please Select" and should have an UserRole
  //   value of -1 (this indicates that the item's value is not set)
  // * Skip all enums that don't pass the category and advance level checks
  // * If the current value of the item is not present in the combobox
  //   due to advancelevel/category filtering then insert it into to the
  //   combobox but mark it using UserRole+1 so that we can color it red
  //   to indicate that the value violates the current category/advance level
  //   settings
  combo->addItem("Please Select", -1);
  combo->setItemData(0, QColor(Qt::red), Qt::ForegroundRole);
  std::string defaultEnum;
  if (itemDef->hasDefault())
  {
    defaultEnum = itemDef->discreteEnum(itemDef->defaultDiscreteIndex());
  }

  std::vector<std::string> validEnums = item->relevantEnums(true, true, uiManager->advanceLevel());
  std::size_t enumIndex;
  for (size_t i = 0; i < validEnums.size(); i++)
  {
    if ((!defaultEnum.empty()) && (validEnums[i] == defaultEnum))
    {
      tooltip = "Default: " + QString(validEnums[i].c_str());
      validEnums[i] += " (Default)";
    }
    itemDef->getEnumIndex(validEnums[i], enumIndex);
    combo->addItem(validEnums[i].c_str(), static_cast<int>(enumIndex));
  }

  if (!tooltip.isEmpty())
  {
    combo->setToolTip(tooltip);
  }
  QPointer<qtDiscreteValueEditor> guardedObject(this);
  QObject::connect(
    combo,
    (void (QComboBox::*)(int)) & QComboBox::currentIndexChanged,
    this,
    [guardedObject]() {
      if (guardedObject)
      {
        guardedObject->onInputValueChanged();
      }
    },
    Qt::QueuedConnection);
  // Add a context menu
  auto* resetDefault = new QAction("Reset to Default", combo);
  if (item->hasDefault())
  {
    QObject::connect(resetDefault, &QAction::triggered, this, [guardedObject, combo]() {
      if (!guardedObject)
      {
        return;
      }
      auto item = guardedObject->Internals->m_inputItem->itemAs<attribute::ValueItem>();
      if (item)
      {
        int elementIdx = guardedObject->Internals->m_elementIndex;
        item->setToDefault(elementIdx);
        guardedObject->updateItemData();
        guardedObject->updateContents();
      }
    });
  }
  else
  {
    resetDefault->setEnabled(false);
  }
  combo->addAction(resetDefault);
  combo->setContextMenuPolicy(Qt::ActionsContextMenu);

  wlayout->addWidget(combo);
  this->Internals->m_combo = combo;
  this->updateItemData();
  this->updateContents();
}

void qtDiscreteValueEditor::updateItemData()
{
  smtk::attribute::ValueItemPtr item = this->Internals->m_inputItem->itemAs<attribute::ValueItem>();
  if (!item || !item->isDiscrete())
  {
    return;
  }

  std::size_t n = item->numberOfValues();
  if (!n)
  {
    return;
  }
  QComboBox* combo = this->Internals->m_combo;
  if (!combo)
  {
    return;
  }
  auto itemDef = item->definitionAs<attribute::ValueItemDefinition>();

  int setIndex = -1, elementIdx = this->Internals->m_elementIndex;
  if (item->isSet(elementIdx))
  {
    setIndex = item->discreteIndex(elementIdx);
  }
  // If combo's current value matches the state of the item then we just need
  // to tell our children items to update their data
  if ((setIndex >= 0) && (setIndex == combo->currentData().toInt()))
  {
    int n = Internals->m_childItems.size();
    for (int i = 0; i < n; i++)
    {
      Internals->m_childItems.at(i)->updateItemData();
    }
    return;
  }
  // We need to find the correct Item in the combo box
  int i, numItems = combo->count();
  for (i = 0; i < numItems; i++)
  {
    if (combo->itemData(i).toInt() == setIndex)
    {
      combo->setCurrentIndex(i);
      break;
    }
  }

  // The item's current value was not found in the combobox
  // due to advance level/category filtering so lets add it
  // but make its text red and mark it using UserRole+1 to indicate
  // it does not currently pass advance level/category filtering
  if (i == numItems)
  {
    combo->addItem(itemDef->discreteEnum(setIndex).c_str(), setIndex);
    combo->setItemData(numItems, QColor(Qt::red), Qt::ForegroundRole);
    combo->setItemData(numItems, 1, Qt::UserRole + 1);
    combo->setCurrentIndex(numItems);
  }

  if ((i == 0) || (i == numItems))
  {
    // On Macs to change the button text color you need to set QPalette::Text
    // For Linux you need to set QPalette::ButtonText
    QPalette comboboxPalette = combo->palette();
    comboboxPalette.setColor(QPalette::ButtonText, Qt::red);
    comboboxPalette.setColor(QPalette::Text, Qt::red);
    combo->setPalette(comboboxPalette);
  }
  else
  {
    combo->setPalette(combo->parentWidget()->palette());
  }
}

void qtDiscreteValueEditor::onInputValueChanged()
{
  // Do we need to update anything?
  QComboBox* const comboBox = this->Internals->m_combo;
  if (!comboBox)
  {
    return;
  }

  int curIdx = comboBox->currentData().toInt();
  // Lets set the combo pallete properly if the current index is 0 (Please Select) or
  // has been added to the combo box because the item is set to a value that is not
  // considered accessible due to the current category and advance level settings
  // (indicated by having UserRole+1 data assugned to it) then set palette to be use red
  // else set it to be the same as the combo-box's parent widget
  if ((comboBox->currentIndex() == 0) || comboBox->currentData(Qt::UserRole + 1).isValid())
  {
    // On Macs to change the button text color you need to set QPalette::Text
    // For Linux you need to set QPalette::ButtonText
    QPalette comboboxPalette = comboBox->palette();
    comboboxPalette.setColor(QPalette::ButtonText, Qt::red);
    comboboxPalette.setColor(QPalette::Text, Qt::red);
    comboBox->setPalette(comboboxPalette);

#ifdef WIN32
    // On Windows, all options in the QComboBox were being displayed in red (regardless of validity)
    //   Iterating through each individual item isn't ideal, but it fixes the problem
    QColor noProblemColor = comboBox->parentWidget()->palette().color(QPalette::WindowText);
    QColor red = QColor(Qt::red);
    comboBox->setItemData(0, red, Qt::ForegroundRole);
    if (comboBox->count() > 0)
    {
      for (int i = 1; i < comboBox->count(); i++)
      {
        comboBox->setItemData(i, noProblemColor, Qt::ForegroundRole);
      }
      if (comboBox->currentIndex() != 0)
      {
        comboBox->setItemData(comboBox->currentIndex(), red, Qt::ForegroundRole);
      }
    }
#endif
  }
  else
  {
    comboBox->setPalette(comboBox->parentWidget()->palette());
  }

  smtk::attribute::ValueItemPtr item = this->Internals->m_inputItem->itemAs<attribute::ValueItem>();

  if (item == nullptr)
  {
    return;
  }

  // If the current selection matches the current value of the item then we can just return
  if (
    item->isSet(this->Internals->m_elementIndex) &&
    (curIdx == item->discreteIndex(this->Internals->m_elementIndex)))
  {
    return; // There is nothing to update
  }

  // If the current selection is invalid and the item value is not set then we can just return
  if (!(item->isDiscreteIndexValid(curIdx) || item->isSet(this->Internals->m_elementIndex)))
  {
    return; // There is nothing to update
  }

  // Is the current selection valid - if not lets unset the item
  if (!item->isDiscreteIndexValid(curIdx))
  {
    item->unset(this->Internals->m_elementIndex);
  }
  else
  {
    item->setDiscreteIndex(this->Internals->m_elementIndex, curIdx);
  }

  this->updateContents();
  this->Internals->m_inputItem->forceUpdate();
}

void qtDiscreteValueEditor::updateContents()
{
  auto* uiManager = this->Internals->m_inputItem->uiManager();
  if (uiManager == nullptr)
  {
    return;
  }

  smtk::attribute::ResourcePtr attResource = this->Internals->m_inputItem->attributeResource();

  this->Internals->clearChildItems();

  smtk::attribute::ValueItemPtr item = this->Internals->m_inputItem->itemAs<attribute::ValueItem>();
  auto itemDef = item->definitionAs<attribute::ValueItemDefinition>();
  // update children frame if necessary
  this->Internals->m_hintChildWidth = 0;
  this->Internals->m_hintChildHeight = 0;
  if (this->Internals->m_childrenFrame)
  {
    if (this->Internals->m_parentLayout)
    {
      this->Internals->m_parentLayout->removeWidget(this->Internals->m_childrenFrame);
    }
    else
    {
      this->layout()->removeWidget(this->Internals->m_childrenFrame);
    }
    delete this->Internals->m_childrenFrame;
  }

  if (item->numberOfActiveChildrenItems() > 0)
  {
    this->Internals->m_childrenFrame = new QFrame(this);
    this->Internals->m_childrenFrame->setObjectName("ChildItemsFrame");
    QSizePolicy sizeFixedPolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QBoxLayout* clayout = nullptr;
    if (qobject_cast<QHBoxLayout*>(this->Internals->m_parentLayout))
    {
      clayout = new QHBoxLayout(this->Internals->m_childrenFrame);
      this->Internals->m_parentLayout->setAlignment(Qt::AlignTop);
      clayout->setAlignment(Qt::AlignTop);
      clayout->setContentsMargins(0, 0, 0, 0);
    }
    else
    {
      clayout = new QVBoxLayout(this->Internals->m_childrenFrame);
    }

    clayout->setObjectName("activeChildLayout");
    this->Internals->m_childrenFrame->setSizePolicy(sizeFixedPolicy);

    QList<smtk::attribute::ItemDefinitionPtr> activeChildDefs;
    std::size_t i, m = item->numberOfActiveChildrenItems();
    for (i = 0; i < m; i++)
    {
      smtk::attribute::ConstItemDefinitionPtr itDef =
        item->activeChildItem(static_cast<int>(i))->definition();
      std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator it =
        itemDef->childrenItemDefinitions().find(itDef->name());
      if (
        (it != itemDef->childrenItemDefinitions().end()) && attResource &&
        attResource->passActiveCategoryCheck(itemDef->categories()))
      {
        activeChildDefs.push_back(it->second);
      }
    }

    auto* iiview = this->Internals->m_inputItem->m_itemInfo.baseView();
    int currentLen = iiview ? iiview->fixedLabelWidth() : 0;
    int tmpLen = uiManager->getWidthOfItemsMaxLabel(activeChildDefs, uiManager->advancedFont());
    if (iiview)
    {
      iiview->setFixedLabelWidth(tmpLen);
    }
    bool hasVisibleChildren = false;
    for (i = 0; i < m; i++)
    {
      auto citem = item->activeChildItem(static_cast<int>(i));
      if (iiview && !iiview->displayItem(citem))
      {
        continue; // This child does not pass display checks so skip it
      }
      auto it = Internals->m_itemViewMap.find(citem->name());
      qtItem* childItem;
      if (it != Internals->m_itemViewMap.end())
      {
        auto info = it->second;
        info.setParentWidget(this->Internals->m_childrenFrame.data());
        info.setItem(citem);
        childItem = this->Internals->m_inputItem->uiManager()->createItem(info);
      }
      else
      {
        smtk::view::Configuration::Component comp; // create a default view style
        qtAttributeItemInfo info(
          citem,
          comp,
          this->Internals->m_childrenFrame.data(),
          this->Internals->m_inputItem->m_itemInfo.baseView());
        childItem = uiManager->createItem(info);
      }
      if (childItem)
      {
        clayout->addWidget(childItem->widget());
        this->Internals->m_childItems.push_back(childItem);
        connect(
          childItem, SIGNAL(modified()), this->Internals->m_inputItem, SLOT(onChildItemModified()));
        connect(childItem, SIGNAL(widgetSizeChanged()), this, SIGNAL(widgetSizeChanged()));
        hasVisibleChildren = true;
      }
    }

    if (iiview)
    {
      iiview->setFixedLabelWidth(currentLen);
    }
    this->Internals->m_hintChildWidth = this->Internals->m_childrenFrame->width();
    this->Internals->m_hintChildHeight = this->Internals->m_childrenFrame->height();
    this->Internals->m_childrenFrame->setVisible(hasVisibleChildren);
    if (this->Internals->m_parentLayout)
    {
      this->Internals->m_parentLayout->addWidget(this->Internals->m_childrenFrame);
    }
    else
    {
      this->layout()->addWidget(this->Internals->m_childrenFrame);
    }
  }
  this->Internals->m_inputItem->m_itemInfo.baseView()->childrenResized();
  Q_EMIT this->widgetSizeChanged();
}
