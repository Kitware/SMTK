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

#include <QComboBox>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
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
  qtDiscreteValueEditorInternals(qtInputsItem* item, int elementIdx, QLayout* childLayout)
    : m_inputItem(item)
    , m_elementIndex(elementIdx)
    , m_childrenLayout(childLayout)
  {
    m_hintChildWidth = 0;
    m_hintChildHeight = 0;
  }

  QPointer<qtInputsItem> m_inputItem;
  int m_elementIndex;
  QPointer<QComboBox> m_combo;
  QPointer<QFrame> m_childrenFrame;

  QList<smtk::extension::qtItem*> m_childItems;
  QPointer<QLayout> m_childrenLayout;
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
  qtInputsItem* item, int elementIdx, QLayout* childLayout)
  : QWidget(item->widget())
  , m_useSelectionManager(false)
{
  this->Internals = new qtDiscreteValueEditorInternals(item, elementIdx, childLayout);
  if (item != nullptr)
  {
    item->m_itemInfo.createNewDictionary(this->Internals->m_itemViewMap);
  }

  this->createWidget();
}

qtDiscreteValueEditor::~qtDiscreteValueEditor()
{
  this->Internals->clearChildItems();
  delete this->Internals;
}

void qtDiscreteValueEditor::createWidget()
{
  smtk::attribute::ValueItemPtr item = this->Internals->m_inputItem->itemAs<attribute::ValueItem>();
  if (!item)
  {
    return;
  }
  this->Internals->clearChildItems();
  QBoxLayout* wlayout = new QVBoxLayout(this);
  wlayout->setMargin(0);

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
  QList<QString> discreteVals;
  QString tooltip;
  for (size_t i = 0; i < itemDef->numberOfDiscreteValues(); i++)
  {
    std::string enumText = itemDef->discreteEnum(static_cast<int>(i));
    if (itemDef->hasDefault() && static_cast<size_t>(itemDef->defaultDiscreteIndex()) == i)
    {
      tooltip = "Default: " + QString(enumText.c_str());
      enumText += " (Default)";
    }
    discreteVals.push_back(enumText.c_str());
  }

  QComboBox* combo = new QComboBox(this);
  if (!tooltip.isEmpty())
  {
    combo->setToolTip(tooltip);
  }
  combo->addItems(discreteVals);
  combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QObject::connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(onInputValueChanged()),
    Qt::QueuedConnection);
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
  if (setIndex < 0 && itemDef->hasDefault() && itemDef->defaultDiscreteIndex() < combo->count())
  {
    setIndex = itemDef->defaultDiscreteIndex();
  }
  combo->setCurrentIndex(setIndex);
}

void qtDiscreteValueEditor::onInputValueChanged()
{
  // Do we need to update anything?
  QComboBox* const comboBox = this->Internals->m_combo;
  if (!comboBox)
  {
    return;
  }
  int curIdx = comboBox->currentIndex();
  smtk::attribute::ValueItemPtr item = this->Internals->m_inputItem->itemAs<attribute::ValueItem>();

  // If the current selection matches the current value of the item then we can just return
  if (item->isSet(this->Internals->m_elementIndex) &&
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
  auto uiManager = this->Internals->m_inputItem->uiManager();
  if (uiManager == nullptr)
    return;

  QComboBox* const comboBox = this->Internals->m_combo;
  if (!comboBox)
  {
    return;
  }
  this->Internals->clearChildItems();

  smtk::attribute::ValueItemPtr item = this->Internals->m_inputItem->itemAs<attribute::ValueItem>();
  auto itemDef = item->definitionAs<attribute::ValueItemDefinition>();
  // update children frame if necessary
  this->Internals->m_hintChildWidth = 0;
  this->Internals->m_hintChildHeight = 0;
  if (this->Internals->m_childrenFrame)
  {
    if (this->Internals->m_childrenLayout)
    {
      this->Internals->m_childrenLayout->removeWidget(this->Internals->m_childrenFrame);
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
    QSizePolicy sizeFixedPolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QVBoxLayout* clayout = new QVBoxLayout(this->Internals->m_childrenFrame);
    clayout->setMargin(3);
    this->Internals->m_childrenFrame->setSizePolicy(sizeFixedPolicy);
    this->Internals->m_childrenFrame->setFrameShape(QFrame::Box);

    QList<smtk::attribute::ItemDefinitionPtr> activeChildDefs;
    std::size_t i, m = item->numberOfActiveChildrenItems();
    for (i = 0; i < m; i++)
    {
      smtk::attribute::ConstItemDefinitionPtr itDef =
        item->activeChildItem(static_cast<int>(i))->definition();
      std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator it =
        itemDef->childrenItemDefinitions().find(itDef->name());
      if (it != itemDef->childrenItemDefinitions().end())
      {
        activeChildDefs.push_back(it->second);
      }
    }

    auto iiview = dynamic_cast<qtBaseAttributeView*>(
      this->Internals->m_inputItem->m_itemInfo.baseView().data());
    int currentLen = iiview ? iiview->fixedLabelWidth() : 0;
    if (this->Internals->m_inputItem->uiManager())
    {
      int tmpLen = this->Internals->m_inputItem->uiManager()->getWidthOfItemsMaxLabel(
        activeChildDefs, this->Internals->m_inputItem->uiManager()->advancedFont());
      if (iiview)
      {
        iiview->setFixedLabelWidth(tmpLen);
      }
    }

    for (i = 0; i < m; i++)
    {
      auto citem = item->activeChildItem(static_cast<int>(i));
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
        smtk::view::View::Component comp; // create a default view style
        qtAttributeItemInfo info(citem, comp, this->Internals->m_childrenFrame.data(),
          this->Internals->m_inputItem->m_itemInfo.baseView());
        childItem = this->Internals->m_inputItem->uiManager()->createItem(info);
      }
      if (childItem)
      {
        clayout->addWidget(childItem->widget());
        this->Internals->m_childItems.push_back(childItem);
        connect(
          childItem, SIGNAL(modified()), this->Internals->m_inputItem, SLOT(onChildItemModified()));
        connect(childItem, SIGNAL(widgetSizeChanged()), this, SIGNAL(widgetSizeChanged()));
      }
    }

    if (iiview)
    {
      iiview->setFixedLabelWidth(currentLen);
    }
    this->Internals->m_hintChildWidth = this->Internals->m_childrenFrame->width();
    this->Internals->m_hintChildHeight = this->Internals->m_childrenFrame->height();
    if (this->Internals->m_childrenLayout)
    {
      this->Internals->m_childrenLayout->addWidget(this->Internals->m_childrenFrame);
    }
    else
    {
      this->layout()->addWidget(this->Internals->m_childrenFrame);
    }
  }
  this->Internals->m_inputItem->m_itemInfo.baseView()->childrenResized();
  emit this->widgetSizeChanged();
}
