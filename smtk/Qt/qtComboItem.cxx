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

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtComboItemInternals
{
public:

  QList<QComboBox*> comboBoxes;
};


//----------------------------------------------------------------------------
qtComboItem::qtComboItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p) : qtItem(dataObj, p)
{
  this->Internals = new qtComboItemInternals;
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
  this->Internals->comboBoxes.clear();
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

  QBoxLayout* layout = NULL;
  if(n == 1)
    {
    layout = new QHBoxLayout(this->Widget);
    }
  else
    {
    layout = new QVBoxLayout(this->Widget);
    }
  layout->setMargin(0);
  QLabel* label = new QLabel(this->getObject()->name().c_str(),
    this->Widget);
  layout->addWidget(label);

  for(i = 0; i < n; i++)
    {
    QComboBox* combo = new QComboBox(this->Widget);
    QVariant vdata((int)i);
    combo->setProperty("ElementIndex", vdata);
    this->Internals->comboBoxes.push_back(combo);
    layout->addWidget(combo);
    QObject::connect(combo,  SIGNAL(currentIndexChanged(int)),
      this, SLOT(onInputValueChanged()), Qt::QueuedConnection);
    }
  
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
    
  const ValueItemDefinition *itemDef = 
    dynamic_cast<const ValueItemDefinition*>(item->definition().get());
    
  QList<QString> discreteVals;
  for (i = 0; i < itemDef->numberOfDiscreteValues(); i++)
    {
    discreteVals.push_back(itemDef->discreteEnum(i).c_str());
    }

  foreach(QComboBox* combo, this->Internals->comboBoxes)
    {
    combo->blockSignals(true);
    combo->clear();
    combo->addItems(discreteVals);
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
void qtComboItem::onInputValueChanged()
{
  QComboBox* const comboBox = qobject_cast<QComboBox*>(
    QObject::sender());
  if(!comboBox)
    {
    return;
    }
  int curIdx = comboBox->currentIndex();
  int elementIdx = comboBox->property("ElementIndex").toInt();
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(curIdx>=0)
    {
    item->setDiscreteIndex(elementIdx, curIdx);
    }
  else
    {
    item->unset(elementIdx);
    }
}
