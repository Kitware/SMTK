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
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPointer>
#include <QFontMetrics>
#include <QFont>

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtUIManager.h"

using namespace smtk::extension;

//----------------------------------------------------------------------------
class qtDiscreteValueEditorInternals
{
public:
  qtDiscreteValueEditorInternals(qtInputsItem *item, int elementIdx, QLayout* childLayout) :
    m_inputItem(item), m_elementIndex(elementIdx), m_childrenLayout(childLayout)
  {
    this->m_hintChildWidth = 0;
    this->m_hintChildHeight = 0;
  }

  QPointer<qtInputsItem> m_inputItem;
  int m_elementIndex;
  QPointer<QComboBox> m_combo;
  QPointer<QFrame> m_childrenFrame;

  QList<smtk::extension::qtItem*> m_childItems;
  QPointer<QLayout> m_childrenLayout;
  int m_hintChildWidth;
  int m_hintChildHeight;

  void clearChildItems()
  {
    for(int i=0; i < this->m_childItems.count(); i++)
      {
      delete this->m_childItems.value(i);
      }
    this->m_childItems.clear();
  }

};

//----------------------------------------------------------------------------
qtDiscreteValueEditor::qtDiscreteValueEditor(
  qtInputsItem *item, int elementIdx, QLayout* childLayout) :
   QWidget(item->widget())
{
  this->Internals = new qtDiscreteValueEditorInternals(
    item, elementIdx, childLayout);
  this->createWidget();
}

//----------------------------------------------------------------------------
qtDiscreteValueEditor::~qtDiscreteValueEditor()
{
  this->Internals->clearChildItems();
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtDiscreteValueEditor::createWidget()
{
  smtk::attribute::ValueItemPtr item = this->Internals->m_inputItem->valueItem();
  if(!item)
    {
    return;
    }
  this->Internals->clearChildItems();
  QBoxLayout* wlayout = new QVBoxLayout(this);
  wlayout->setMargin(0);

  if(!item || !item->isDiscrete())
    {
    return;
    }

  std::size_t n = item->numberOfValues();
  if (!n)
    {
    return;
    }

  const attribute::ValueItemDefinition *itemDef =
    dynamic_cast<const attribute::ValueItemDefinition*>(item->definition().get());
  QList<QString> discreteVals;
  QString tooltip;
  for (size_t i = 0; i < itemDef->numberOfDiscreteValues(); i++)
    {
    std::string enumText = itemDef->discreteEnum(static_cast<int>(i));
    if(itemDef->hasDefault() &&
      static_cast<size_t>(itemDef->defaultDiscreteIndex()) == i)
      {
      tooltip = "Default: " + QString(enumText.c_str());
      enumText+= " (Default)";
      }
    discreteVals.push_back(enumText.c_str());
    }

  QComboBox* combo = new QComboBox(this);
  if(!tooltip.isEmpty())
    {
    combo->setToolTip(tooltip);
    }
  combo->addItems(discreteVals);
  combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QObject::connect(combo,  SIGNAL(currentIndexChanged(int)),
    this, SLOT(onInputValueChanged()), Qt::QueuedConnection);
  wlayout->addWidget(combo);
  this->Internals->m_combo = combo;
  this->updateItemData();
  this->onInputValueChanged();
}

//----------------------------------------------------------------------------
void qtDiscreteValueEditor::updateItemData()
{
  smtk::attribute::ValueItemPtr item = this->Internals->m_inputItem->valueItem();
  if(!item || !item->isDiscrete())
    {
    return;
    }

  std::size_t n = item->numberOfValues();
  if (!n)
    {
    return;
    }
  QComboBox* combo = this->Internals->m_combo;
  if(!combo)
    {
    return;
    }
  const attribute::ValueItemDefinition *itemDef =
    dynamic_cast<const attribute::ValueItemDefinition*>(item->definition().get());

  int setIndex = -1, elementIdx = this->Internals->m_elementIndex;
  if (item->isSet(elementIdx))
    {
    setIndex = item->discreteIndex(elementIdx);
    }
  if(setIndex < 0 && itemDef->hasDefault() &&
    itemDef->defaultDiscreteIndex() < combo->count())
    {
    setIndex = itemDef->defaultDiscreteIndex();
    }
  combo->setCurrentIndex(setIndex);
}

//----------------------------------------------------------------------------
void qtDiscreteValueEditor::onInputValueChanged()
{
  if(!this->Internals->m_inputItem->baseView()->uiManager())
    return;
  QComboBox* const comboBox = this->Internals->m_combo;
  if(!comboBox)
    {
    return;
    }
  this->Internals->clearChildItems();

  int curIdx = comboBox->currentIndex();
  int elementIdx = this->Internals->m_elementIndex;
  smtk::attribute::ValueItemPtr item = this->Internals->m_inputItem->valueItem();
  bool refresh = false;
  const attribute::ValueItemDefinition *itemDef =
    dynamic_cast<const attribute::ValueItemDefinition*>(item->definition().get());
  if (!item->isDiscreteIndexValid(curIdx))
    {
    if (item->isSet(this->Internals->m_elementIndex))
      {
      this->Internals->m_inputItem->unsetValue(this->Internals->m_elementIndex);
      refresh = true;
      }
    }
  else
    {
    // We are dealing with a valid value
    refresh = 
      this->Internals->m_inputItem->setDiscreteValue(this->Internals->m_elementIndex, curIdx);
    }
  if (!refresh)
    {
    return;
    }
  // update children frame if necessary
  this->Internals->m_hintChildWidth = 0;
  this->Internals->m_hintChildHeight = 0;
  if(this->Internals->m_childrenFrame)
    {
    if(this->Internals->m_childrenLayout)
      {
      this->Internals->m_childrenLayout->removeWidget(this->Internals->m_childrenFrame);
      }
    else
      {
      this->layout()->removeWidget(this->Internals->m_childrenFrame);
      }
    delete this->Internals->m_childrenFrame;
    }

  if(item->numberOfActiveChildrenItems() > 0)
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
    for(i = 0; i < m; i++)
      {
      smtk::attribute::ConstItemDefinitionPtr itDef = item->activeChildItem(
            static_cast<int>(i))->definition();
      std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator it =
        itemDef->childrenItemDefinitions().find(itDef->name());
      if(it != itemDef->childrenItemDefinitions().end())
        {
        activeChildDefs.push_back(it->second);
        }
      }

    int currentLen = this->Internals->m_inputItem->baseView()->fixedLabelWidth();
    if(this->Internals->m_inputItem->baseView()->uiManager())
      {
      int tmpLen = this->Internals->m_inputItem->baseView()->uiManager()->getWidthOfItemsMaxLabel(
        activeChildDefs, this->Internals->m_inputItem->baseView()->uiManager()->advancedFont());
      this->Internals->m_inputItem->baseView()->setFixedLabelWidth(tmpLen);
      }

    for(i = 0; i < m; i++)
      {
      qtItem* childItem = qtAttribute::createItem(
        item->activeChildItem(static_cast<int>(i)),
        this->Internals->m_childrenFrame, this->Internals->m_inputItem->baseView());
      if(childItem)
        {
        clayout->addWidget(childItem->widget());
        this->Internals->m_childItems.push_back(childItem);
        connect(childItem, SIGNAL(modified()), this->Internals->m_inputItem, SLOT(onChildItemModified()));
        }
      }
    this->Internals->m_inputItem->baseView()->setFixedLabelWidth(currentLen);
    this->Internals->m_hintChildWidth = this->Internals->m_childrenFrame->width();
    this->Internals->m_hintChildHeight = this->Internals->m_childrenFrame->height();
    if(this->Internals->m_childrenLayout)
      {
      this->Internals->m_childrenLayout->addWidget(this->Internals->m_childrenFrame);
      }
    else
      {
      this->layout()->addWidget(this->Internals->m_childrenFrame);
      }
    }
  this->Internals->m_inputItem->baseView()->childrenResized();
}

//-----------------------------------------------------------------------------
QSize qtDiscreteValueEditor::sizeHint() const
{
  return QSize(this->Internals->m_combo->width(),
    this->Internals->m_combo->height() + this->Internals->m_hintChildHeight);
}
