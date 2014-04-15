/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "smtk/Qt/qtDiscreteValueItem.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPointer>
#include <QGridLayout>
#include <QCheckBox>

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/Qt/qtAttribute.h"
#include "smtk/Qt/qtBaseView.h"
#include "smtk/Qt/qtUIManager.h"
#include "smtk/view/Root.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtDiscreteValueItemInternals
{
public:
  qtDiscreteValueItemInternals()
  {
  }
  QList< QPointer<QComboBox> > ChildCombos;
  QPointer<QFrame> CurrentChildFrame;
  QPointer<QFrame> EntryFrame;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;
};

//----------------------------------------------------------------------------
qtDiscreteValueItem::qtDiscreteValueItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p,
   qtBaseView* bview, Qt::Orientation enVectorItemOrient) :
   qtItem(dataObj, p, bview)
{
  this->Internals = new qtDiscreteValueItemInternals();
  this->IsLeafItem = true;
  this->Internals->VectorItemOrient = enVectorItemOrient;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtDiscreteValueItem::~qtDiscreteValueItem()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtDiscreteValueItem::createWidget()
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!item || !item->isDiscrete())
    {
    return;
    }
  std::size_t n = item->numberOfValues();
  if (!n)
    {
    return;
    }

  this->clearChildItems();
  this->Internals->ChildCombos.clear();
  if(this->Internals->EntryFrame)
    {
    this->Widget->layout()->removeWidget(this->Internals->EntryFrame);
    delete this->Internals->EntryFrame;
    }

  this->Widget = new QFrame(this->parentWidget());
  QGridLayout* layout = new QGridLayout(this->Widget);
  layout->setMargin(3);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  this->Internals->EntryFrame = new QFrame(this->parentWidget());
  this->Internals->EntryFrame->setObjectName("DiscreteValueInputFrame");
  QBoxLayout* entryLayout;
  if(this->Internals->VectorItemOrient == Qt::Vertical)
    {
    entryLayout = new QVBoxLayout(this->Internals->EntryFrame);
    }
  else
    {
    entryLayout = new QHBoxLayout(this->Internals->EntryFrame);
    }

  entryLayout->setMargin(0);
  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if(item->isOptional())
    {
    QCheckBox* optionalCheck = new QCheckBox(this->parentWidget());
    optionalCheck->setChecked(item->isEnabled());
    optionalCheck->setText("");
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = optionalCheck->iconSize().width() + 6; // 6 is for layout spacing
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)),
      this, SLOT(setOutputOptional(int)));
    this->Internals->EntryFrame->setEnabled(item->isEnabled());
    labelLayout->addWidget(optionalCheck);
    }

  const ValueItemDefinition *itemDef =
    dynamic_cast<const ValueItemDefinition*>(item->definition().get());
  QString labelText;
  if(!item->label().empty())
    {
    labelText = item->label().c_str();
    }
  else
    {
    labelText = item->name().c_str();
    }
  QLabel* label = new QLabel(labelText, this->Widget);
  label->setSizePolicy(sizeFixedPolicy);
  smtk::view::RootPtr rs = this->baseView()->uiManager()->attManager()->rootView();
  label->setFixedWidth(rs->maxValueLabelLength() - padding);
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignTop);

  // add in BriefDescription as tooltip if available
  const std::string strBriefDescription = itemDef->briefDescription();
  if(!strBriefDescription.empty())
    {
    label->setToolTip(strBriefDescription.c_str());
    }

  if(!itemDef->units().empty())
    {
    QString unitText=label->text();
    unitText.append(" (").append(itemDef->units().c_str()).append(")");
    label->setText(unitText);
    }
  if(itemDef->advanceLevel())
    {
    label->setFont(this->baseView()->uiManager()->advancedFont());
    }
  labelLayout->addWidget(label);
  this->Internals->theLabel = label;

  QList<QString> discreteVals;
  QString tooltip;
  for (size_t i = 0; i < itemDef->numberOfDiscreteValues(); i++)
    {
    std::string enumText = itemDef->discreteEnum(static_cast<int>(i));
    if(itemDef->hasDefault() &&
      static_cast<size_t>(itemDef->defaultDiscreteIndex()) == i)
      {
      tooltip = "Default: " + QString(enumText.c_str());
      }
    discreteVals.push_back(enumText.c_str());
    }

//  this->Internals->RefComboLayout->addWidget(this->Internals->EditButton);
  for(std::size_t i = 0; i < n; i++)
    {
    QComboBox* combo = new QComboBox(this->Internals->EntryFrame);
    QVariant vdata(static_cast<int>(i));
    combo->setProperty("ElementIndex", vdata);
    this->Internals->ChildCombos.push_back(combo);
    combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    if(!tooltip.isEmpty())
      {
      combo->setToolTip(tooltip);
      }
    combo->addItems(discreteVals);
    entryLayout->addWidget(combo);
    QObject::connect(combo,  SIGNAL(currentIndexChanged(int)),
      this, SLOT(onInputValueChanged()), Qt::QueuedConnection);
    }

  entryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  layout->addLayout(labelLayout, 0, 0);
  layout->addWidget(this->Internals->EntryFrame, 0, 1);
  layout->setAlignment(Qt::AlignTop);

  this->updateItemData();
  if(this->Internals->ChildCombos.count() > 0)
    {
    this->refreshUI(this->Internals->ChildCombos[0]);
    }
}

//----------------------------------------------------------------------------
void qtDiscreteValueItem::updateItemData()
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!item || !item->isDiscrete())
    {
    return;
    }

  std::size_t n = item->numberOfValues();
  if (!n)
    {
    return;
    }
  const ValueItemDefinition *itemDef =
    dynamic_cast<const ValueItemDefinition*>(item->definition().get());
  foreach(QComboBox* combo, this->Internals->ChildCombos)
    {
    combo->blockSignals(true);
    int elementIdx = combo->property("ElementIndex").toInt();
    int setIndex = -1;
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
    combo->blockSignals(false);
    }
}

//----------------------------------------------------------------------------
void qtDiscreteValueItem::onInputValueChanged()
{
  QComboBox* const comboBox = qobject_cast<QComboBox*>(
    QObject::sender());
  if(!comboBox)
    {
    return;
    }
  this->refreshUI(comboBox);
}

//----------------------------------------------------------------------------
void qtDiscreteValueItem::refreshUI(QComboBox* comboBox)
{
  int curIdx = comboBox->currentIndex();
  int elementIdx = comboBox->property("ElementIndex").toInt();
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  bool refresh = false;
  const ValueItemDefinition *itemDef =
    dynamic_cast<const ValueItemDefinition*>(item->definition().get());
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
      this->baseView()->valueChanged(this->getObject());
      refresh = true;
      }
    else if(item->isSet(elementIdx))
      {
      item->unset(elementIdx);
      this->baseView()->valueChanged(this->getObject());
      refresh = true;
      }
    }

  // update children frame if necessary
  if(this->Internals->CurrentChildFrame)
    {
    this->Widget->layout()->removeWidget(this->Internals->CurrentChildFrame);
    delete this->Internals->CurrentChildFrame;
    this->Internals->CurrentChildFrame = NULL;
    }

  if(refresh && item->numberOfActiveChildrenItems() > 0)
    {
    this->Internals->CurrentChildFrame = new QFrame(this->Widget);
    this->Internals->CurrentChildFrame->setObjectName("ChildItemsFrame");
    QSizePolicy sizeFixedPolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    QVBoxLayout* layout = new QVBoxLayout(this->Internals->CurrentChildFrame);
    layout->setMargin(3);
    this->Internals->CurrentChildFrame->setSizePolicy(sizeFixedPolicy);
    this->Internals->CurrentChildFrame->setFrameShape(QFrame::Box);
    std::size_t i, m = item->numberOfActiveChildrenItems();
    for(i = 0; i < m; i++)
      {
      qtItem* childItem = qtAttribute::createItem(
        item->activeChildItem(static_cast<int>(i)),
        this->Internals->CurrentChildFrame, this->baseView());
      if(childItem)
        {
        layout->addWidget(childItem->widget());
        }
      }
    QGridLayout* parentGrid = static_cast<QGridLayout*>(this->Widget->layout());
    parentGrid->addWidget(this->Internals->CurrentChildFrame, 1, 0, 1, 2);
    this->Internals->CurrentChildFrame->setEnabled(item->isEnabled());
    }
  emit this->widgetResized();
}

//----------------------------------------------------------------------------
void qtDiscreteValueItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
}

//----------------------------------------------------------------------------
void qtDiscreteValueItem::setOutputOptional(int state)
{
  bool enable = state ? true : false;
  this->Internals->EntryFrame->setEnabled(enable);
  if(this->Internals->CurrentChildFrame)
    {
    this->Internals->CurrentChildFrame->setEnabled(enable);
    }
  if(enable != this->getObject()->isEnabled())
    {
    this->getObject()->setIsEnabled(enable);
    this->baseView()->valueChanged(this->getObject());
    }
}
