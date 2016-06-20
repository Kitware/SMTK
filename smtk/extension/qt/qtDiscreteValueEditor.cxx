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
  qtDiscreteValueEditorInternals(int elementIdx, QWidget* p,
    smtk::attribute::ItemPtr dataObj, qtBaseView* bview, QLayout* childLayout) :
    ElementIndex(elementIdx), ParentWidget(p), DataObject(dataObj),
    BaseView(bview), ChildrenLayout(childLayout)
  {
    this->hintChildWidth = 0;
    this->hintChildHeight = 0;
  }

  int ElementIndex;
  QPointer<QComboBox> Combo;
  QPointer<QFrame> ChildrenFrame;

  QPointer<QWidget> ParentWidget;
  smtk::attribute::WeakItemPtr DataObject;
  QList<smtk::extension::qtItem*> ChildItems;
  QPointer<qtBaseView> BaseView;
  QPointer<QLayout> ChildrenLayout;
  int hintChildWidth;
  int hintChildHeight;

  void clearChildItems()
  {
    for(int i=0; i < this->ChildItems.count(); i++)
      {
      delete this->ChildItems.value(i);
      }
    this->ChildItems.clear();
  }

};

//----------------------------------------------------------------------------
qtDiscreteValueEditor::qtDiscreteValueEditor(
  smtk::attribute::ItemPtr dataObj, int elementIdx, QWidget* p,
   qtBaseView* bview, QLayout* childLayout) :
   QWidget(p)
{
  this->Internals = new qtDiscreteValueEditorInternals(
    elementIdx, p, dataObj, bview, childLayout);
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
  if(!this->Internals->DataObject.lock())
    {
    return;
    }
  this->Internals->clearChildItems();
  QBoxLayout* wlayout = new QVBoxLayout(this);
  wlayout->setMargin(0);
//  this->Widget = new QFrame(this->Internals->ParentWidget);

  smtk::attribute::ValueItemPtr item =smtk::dynamic_pointer_cast<attribute::ValueItem>(
    this->Internals->DataObject.lock());
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
  this->Internals->Combo = combo;
  this->updateItemData();
  this->onInputValueChanged();
}

//----------------------------------------------------------------------------
void qtDiscreteValueEditor::updateItemData()
{
  smtk::attribute::ValueItemPtr item =smtk::dynamic_pointer_cast<attribute::ValueItem>(
    this->Internals->DataObject.lock());
  if(!item || !item->isDiscrete())
    {
    return;
    }

  std::size_t n = item->numberOfValues();
  if (!n)
    {
    return;
    }
  QComboBox* combo = this->Internals->Combo;
  if(!combo)
    {
    return;
    }
  const attribute::ValueItemDefinition *itemDef =
    dynamic_cast<const attribute::ValueItemDefinition*>(item->definition().get());

  int setIndex = -1, elementIdx = this->Internals->ElementIndex;
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
  if(!this->Internals->BaseView->uiManager())
    return;
  QComboBox* const comboBox = this->Internals->Combo;
  if(!comboBox)
    {
    return;
    }
  this->Internals->clearChildItems();

  int curIdx = comboBox->currentIndex();
  int elementIdx =this->Internals->ElementIndex;
  smtk::attribute::ValueItemPtr item =smtk::dynamic_pointer_cast<attribute::ValueItem>(
    this->Internals->DataObject.lock());
  bool refresh = false;
  const attribute::ValueItemDefinition *itemDef =
    dynamic_cast<const attribute::ValueItemDefinition*>(item->definition().get());
  if(itemDef->isDiscreteIndexValid(curIdx) && item->isSet(elementIdx) &&
      item->discreteIndex(elementIdx) == curIdx)
    {
    refresh = true; // nothing to do
    }
  else
    {
    if(itemDef->isDiscreteIndexValid(curIdx))
      {
      item->setDiscreteIndex(elementIdx, curIdx);
      this->Internals->BaseView->valueChanged(this->Internals->DataObject.lock());
      refresh = true;
      }
    else if(item->isSet(elementIdx))
      {
      item->unset(elementIdx);
      this->Internals->BaseView->valueChanged(this->Internals->DataObject.lock());
      refresh = true;
      }
    }

  // update children frame if necessary
  this->Internals->hintChildWidth = 0;
  this->Internals->hintChildHeight = 0;
  if(this->Internals->ChildrenFrame)
    {
    if(this->Internals->ChildrenLayout)
      {
      this->Internals->ChildrenLayout->removeWidget(this->Internals->ChildrenFrame);
      }
    else
      {
      this->layout()->removeWidget(this->Internals->ChildrenFrame);
      }
    delete this->Internals->ChildrenFrame;
    }

  if(refresh && item->numberOfActiveChildrenItems() > 0)
    {
    this->Internals->ChildrenFrame = new QFrame(this);
    this->Internals->ChildrenFrame->setObjectName("ChildItemsFrame");
    QSizePolicy sizeFixedPolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QVBoxLayout* clayout = new QVBoxLayout(this->Internals->ChildrenFrame);
    clayout->setMargin(3);
    this->Internals->ChildrenFrame->setSizePolicy(sizeFixedPolicy);
    this->Internals->ChildrenFrame->setFrameShape(QFrame::Box);

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

    int currentLen = this->Internals->BaseView->fixedLabelWidth();
    if(this->Internals->BaseView->uiManager())
      {
      int tmpLen = this->Internals->BaseView->uiManager()->getWidthOfItemsMaxLabel(
        activeChildDefs, this->Internals->BaseView->uiManager()->advancedFont());
      this->Internals->BaseView->setFixedLabelWidth(tmpLen);
      }

    for(i = 0; i < m; i++)
      {
      qtItem* childItem = qtAttribute::createItem(
        item->activeChildItem(static_cast<int>(i)),
        this->Internals->ChildrenFrame, this->Internals->BaseView);
      if(childItem)
        {
        clayout->addWidget(childItem->widget());
        this->Internals->ChildItems.push_back(childItem);
        }
      }
    this->Internals->BaseView->setFixedLabelWidth(currentLen);
    this->Internals->hintChildWidth = this->Internals->ChildrenFrame->width();
    this->Internals->hintChildHeight = this->Internals->ChildrenFrame->height();
    if(this->Internals->ChildrenLayout)
      {
      this->Internals->ChildrenLayout->addWidget(this->Internals->ChildrenFrame);
      }
    else
      {
      this->layout()->addWidget(this->Internals->ChildrenFrame);
      }
    }
  this->Internals->BaseView->valueChanged(item);
  this->Internals->BaseView->childrenResized();
}

//-----------------------------------------------------------------------------
QSize qtDiscreteValueEditor::sizeHint() const
{
  return QSize(this->Internals->Combo->width(),
    this->Internals->Combo->height() + this->Internals->hintChildHeight);
}
