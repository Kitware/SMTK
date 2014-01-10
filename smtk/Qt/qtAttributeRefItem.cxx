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

#include "smtk/Qt/qtAttributeRefItem.h"

#include "smtk/Qt/qtUIManager.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"

#include <QComboBox>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtAttributeRefItemInternals
{
public:
  QList<QComboBox*> comboBoxes;
  QLabel* theLabel;
};

//----------------------------------------------------------------------------
qtAttributeRefItem::qtAttributeRefItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p) : qtItem(dataObj, p)
{
  this->Internals = new qtAttributeRefItemInternals;
  this->IsLeafItem = true;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtAttributeRefItem::~qtAttributeRefItem()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
QString qtAttributeRefItem::labelText() const
{
  return this->Internals->theLabel->text();
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::setLabelVisible(bool visible)
{
  return this->Internals->theLabel->setVisible(visible);
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::createWidget()
{
  if(!this->getObject())
    {
    return;
    }
  this->clearChildItems();
  this->Internals->comboBoxes.clear();
  this->Widget = new QFrame(this->parentWidget());

  smtk::attribute::RefItemPtr item =dynamic_pointer_cast<RefItem>(this->getObject());
  if(!item)
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
  QString lText = this->getObject()->label().c_str();
  this->Internals->theLabel = new QLabel(lText, this->Widget);
  layout->addWidget(this->Internals->theLabel);

  for(i = 0; i < n; i++)
    {
    QComboBox* combo = new QComboBox(this->Widget);
    QVariant vdata(static_cast<int>(i));
    combo->setProperty("ElementIndex", vdata);
    this->Internals->comboBoxes.push_back(combo);
    layout->addWidget(combo);
    QObject::connect(combo,  SIGNAL(currentIndexChanged(int)),
      this, SLOT(onInputValueChanged()), Qt::QueuedConnection);
    }

  this->updateItemData();
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::updateItemData()
{
  smtk::attribute::RefItemPtr item =dynamic_pointer_cast<RefItem>(this->getObject());
  if(!item)
    {
    return;
    }

  std::size_t n = item->numberOfValues();
  if (!n)
    {
    return;
    }

  const RefItemDefinition *itemDef =
    dynamic_cast<const RefItemDefinition*>(item->definition().get());
  attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
  if(!attDef)
    {
    return;
    }
  QList<QString> attNames;
  std::vector<smtk::attribute::AttributePtr> result;
  Manager *attManager = attDef->manager();
  attManager->findAttributes(attDef, result);
  std::vector<smtk::attribute::AttributePtr>::iterator it;
  for (it=result.begin(); it!=result.end(); ++it)
    {
    attNames.push_back((*it)->name().c_str());
    }

  foreach(QComboBox* combo, this->Internals->comboBoxes)
    {
    combo->blockSignals(true);
    combo->clear();
    combo->addItems(attNames);
    int elementIdx = combo->property("ElementIndex").toInt();
    int setIndex = -1;
    if (item->isSet(elementIdx))
      {
      setIndex = attNames.indexOf(item->valueAsString(elementIdx).c_str());
      }
    combo->setCurrentIndex(setIndex);
    combo->blockSignals(false);
    }
}
//----------------------------------------------------------------------------
void qtAttributeRefItem::onInputValueChanged()
{
  QComboBox* const comboBox = qobject_cast<QComboBox*>(
    QObject::sender());
  if(!comboBox)
    {
    return;
    }
  int curIdx = comboBox->currentIndex();
  int elementIdx = comboBox->property("ElementIndex").toInt();

  smtk::attribute::RefItemPtr item =dynamic_pointer_cast<RefItem>(this->getObject());

  if(curIdx>=0)
    {
    const RefItemDefinition *itemDef =
      dynamic_cast<const RefItemDefinition*>(item->definition().get());
    attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
    Manager *attManager = attDef->manager();
    AttributePtr attPtr = attManager->findAttribute(comboBox->currentText().toStdString());
    if(attPtr)
      {
      item->setValue(elementIdx, attPtr);
      }
    else
      {
      item->unset(elementIdx);
      }
    }
  else
    {
    item->unset(elementIdx);
    }
}
