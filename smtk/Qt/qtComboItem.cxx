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

#include "smtk/Qt/qtComboItem.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPointer>

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/Qt/qtAttribute.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtComboItemInternals
{
public:
  qtComboItemInternals(int elementIdx) : ElementIndex(elementIdx)
  {
  }
  int ElementIndex;
  QPointer<QComboBox> Combo;
  QPointer<QFrame> ChildrenFrame;
};

//----------------------------------------------------------------------------
qtComboItem::qtComboItem(
  smtk::attribute::ItemPtr dataObj, int elementIdx, QWidget* p) : qtItem(dataObj, p)
{
  this->Internals = new qtComboItemInternals(elementIdx);
  this->IsLeafItem = true;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtComboItem::~qtComboItem()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtComboItem::createWidget()
{
  if(!this->getObject())
    {
    return;
    }
  this->clearChildItems();
  this->Widget = new QFrame(this->parentWidget());

  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!item || !item->isDiscrete())
    {
    return;
    }

  std::size_t i, n = item->numberOfValues();
  if (!n)
    {
    return;
    }
  QBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);

  int elementIdx = this->Internals->ElementIndex;
  const ValueItemDefinition *itemDef =
    dynamic_cast<const ValueItemDefinition*>(item->definition().get());
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

  QComboBox* combo = new QComboBox(this->Widget);
  if(!tooltip.isEmpty())
    {
    combo->setToolTip(tooltip);
    }
  combo->addItems(discreteVals);

  QObject::connect(combo,  SIGNAL(currentIndexChanged(int)),
    this, SLOT(onInputValueChanged()), Qt::QueuedConnection);
  layout->addWidget(combo);
  this->Internals->Combo = combo;
  this->updateItemData();
}

//----------------------------------------------------------------------------
void qtComboItem::updateItemData()
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!item || !item->isDiscrete())
    {
    return;
    }

  std::size_t i, n = item->numberOfValues();
  if (!n)
    {
    return;
    }
  QComboBox* combo = this->Internals->Combo;
  if(!combo)
    {
    return;
    }
  const ValueItemDefinition *itemDef = 
    dynamic_cast<const ValueItemDefinition*>(item->definition().get());

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
void qtComboItem::onInputValueChanged()
{
  QComboBox* const comboBox = qobject_cast<QComboBox*>(
    QObject::sender());
  if(!comboBox)
    {
    return;
    }

  int curIdx = comboBox->currentIndex();
  int elementIdx =this->Internals->ElementIndex;
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(curIdx>=0)
    {
    item->setDiscreteIndex(elementIdx, curIdx);
    }
  else
    {
    item->unset(elementIdx);
    }

  // update children frame if necessary
  if(this->Internals->ChildrenFrame)
    {
    this->Widget->layout()->removeWidget(this->Internals->ChildrenFrame);
    delete this->Internals->ChildrenFrame;
    }

  if(item->numberOfActiveChildrenItems() > 0)
    {
    this->Internals->ChildrenFrame = new QFrame(this->Widget);
    this->Internals->ChildrenFrame->setObjectName("ChildItemsFrame");
    QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QVBoxLayout* layout = new QVBoxLayout(this->Internals->ChildrenFrame);
    layout->setMargin(0);
    this->Internals->ChildrenFrame->setSizePolicy(sizeFixedPolicy);
    std::size_t i, m = item->numberOfActiveChildrenItems();
    for(i = 0; i < m; i++)
      {
      qtItem* childItem = qtAttribute::createItem(
        item->activeChildItem(static_cast<int>(i)),
        this->Internals->ChildrenFrame);
      if(childItem)
        {
        layout->addWidget(childItem->widget());
        }
      }
    this->Widget->layout()->addWidget(this->Internals->ChildrenFrame);
    }

}
