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

#include "qtInputsItem.h"

#include "qtUIManager.h"

#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QVariant>
#include <QSizePolicy>
#include <QPointer>
#include <QTextEdit>

#include "attribute/DoubleItem.h"
#include "attribute/DoubleItemDefinition.h"
#include "attribute/IntItem.h"
#include "attribute/IntItemDefinition.h"
#include "attribute/StringItem.h"
#include "attribute/StringItemDefinition.h"
#include "attribute/ValueItem.h"
#include "attribute/ValueItemDefinition.h"

using namespace slctk::attribute;

//----------------------------------------------------------------------------
class qtInputsItemInternals
{
public:

  QPointer<QFrame> EntryFrame;
};


//----------------------------------------------------------------------------
qtInputsItem::qtInputsItem(
  slctk::AttributeItemPtr dataObj, QWidget* p) : qtItem(dataObj, p)
{
  this->Internals = new qtInputsItemInternals;
  this->IsLeafItem = true;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtInputsItem::~qtInputsItem()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtInputsItem::createWidget()
{
  slctk::AttributeItemPtr dataObj = this->getObject();
  if(!dataObj || !this->passAdvancedCheck(
    dataObj->definition()->advanceLevel()))
    {
    return;
    }

  this->clearChildItems();
  this->Widget = new QFrame(this->parentWidget());
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  this->Internals->EntryFrame = new QFrame(this->parentWidget());
  this->Internals->EntryFrame->setObjectName("CheckAndEntryInputFrame");
  QHBoxLayout* entryLayout = new QHBoxLayout(this->Internals->EntryFrame);
  entryLayout->setMargin(0);
  this->loadInputValues(entryLayout);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setAlignment(Qt::AlignLeft);
  if(dataObj->definition()->isOptional())
    {
    QCheckBox* optionalCheck = new QCheckBox(this->parentWidget());
    optionalCheck->setChecked(dataObj->definition()->isEnabledByDefault());
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)),
      this, SLOT(setOutputOptional(int)));
    this->Internals->EntryFrame->setEnabled(
      dataObj->definition()->isEnabledByDefault());
    labelLayout->addWidget(optionalCheck);
    }
  QLabel* label = new QLabel(dataObj->definition()->label().c_str(),
    this->parentWidget());
  label->setSizePolicy(sizeFixedPolicy);

  const ValueItemDefinition *itemDef = 
    dynamic_cast<const ValueItemDefinition*>(dataObj->definition().get());
  
  if(!itemDef->units().empty())
    {
    QString unitText=label->text();
    unitText.append(" (").append(itemDef->units().c_str()).append(")");
    label->setText(unitText);
    }
  if(dataObj->definition()->advanceLevel())
    {
    label->setFont(qtUIManager::instance()->advancedFont());
    }

  labelLayout->addWidget(label);

  entryLayout->setAlignment(Qt::AlignLeft);
  layout->addLayout(labelLayout);
  layout->addWidget(this->Internals->EntryFrame);
  layout->setAlignment(Qt::AlignTop);

  this->parentWidget()->layout()->addWidget(this->Widget);
}

//----------------------------------------------------------------------------
void qtInputsItem::loadInputValues(QBoxLayout* layout)
{
  slctk::ValueItemPtr item =dynamicCastPointer<ValueItem>(this->getObject());
  if(!item || item->isDiscrete())
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
    
  for(i = 0; i < n; i++)
    {
    QWidget* editBox = this->createInputWidget((int)i);
    layout->addWidget(editBox);
    }
}
//----------------------------------------------------------------------------
void qtInputsItem::updateUI()
{
  if(this->Internals->EntryFrame)
    {
    this->Widget->layout()->removeWidget(this->Internals->EntryFrame);
    delete this->Internals->EntryFrame;
    }

  this->Internals->EntryFrame = new QFrame(this->parentWidget());
  this->Internals->EntryFrame->setObjectName("CheckAndEntryInputFrame");
  QHBoxLayout* entryLayout = new QHBoxLayout(this->Internals->EntryFrame);
  entryLayout->setMargin(0);

  this->loadInputValues(entryLayout);
  this->Widget->layout()->addWidget(this->Internals->EntryFrame);
}

//----------------------------------------------------------------------------
void qtInputsItem::setOutputOptional(int state)
{
  this->getObject()->setIsEnabled(state ? true : false);
  this->Internals->EntryFrame->setEnabled(state);
}

//----------------------------------------------------------------------------
QWidget* qtInputsItem::createInputWidget(int elementIdx)
{
  slctk::ValueItemPtr item =dynamicCastPointer<ValueItem>(this->getObject());
  QWidget* inputWidget = NULL;
  QVariant vdata(elementIdx);
  bool isDefault = false;
  switch (item->type())
    {
    case slctk::attribute::Item::DOUBLE:
      {
      QLineEdit* editBox = new QLineEdit(this->Widget);
      const DoubleItemDefinition *dDef = 
        dynamic_cast<const DoubleItemDefinition*>(item->definition().get());
      QDoubleValidator *validator = new QDoubleValidator(this->Widget);
      editBox->setValidator(validator);
      editBox->setFixedWidth(100);
      
      double value=slctk_DOUBLE_MIN;
      if(dDef->hasMinRange())
        {
        value = dDef->minRangeInclusive() ?
          dDef->minRange() : dDef->minRange() + slctk_DOUBLE_CONSTRAINT_PRECISION;
        validator->setBottom(value);
        }
      value=slctk_DOUBLE_MAX;
      if(dDef->hasMaxRange())
        {
        value = dDef->maxRangeInclusive() ?
          dDef->maxRange() : dDef->maxRange() - slctk_DOUBLE_CONSTRAINT_PRECISION;
        validator->setTop(value);
        }

      slctk::DoubleItemPtr ditem =dynamicCastPointer<DoubleItem>(this->getObject());
      if(item->isSet(elementIdx))
        {
        editBox->setText(item->valueAsString(elementIdx).c_str());

        isDefault = dDef->hasDefault() &&
          dDef->defaultValue()==ditem->value(elementIdx);
        }
      else if(dDef->hasDefault())
        {
        editBox->setText(QString::number(dDef->defaultValue()));
        isDefault = true;
        }
      inputWidget = editBox;
      break;
      }
    case slctk::attribute::Item::INT:
      {
      QLineEdit* editBox = new QLineEdit(this->Widget);
      const IntItemDefinition *iDef = 
        dynamic_cast<const IntItemDefinition*>(item->definition().get());
      QIntValidator *validator = new QIntValidator(this->Widget);
      editBox->setValidator(validator);
      editBox->setFixedWidth(100);

      int value=slctk_INT_MIN;
      if(iDef->hasMinRange())
        {
        value = iDef->minRangeInclusive() ?
          iDef->minRange() : iDef->minRange() + 1;
        validator->setBottom(value);
        }
      value=slctk_INT_MAX;
      if(iDef->hasMaxRange())
        {
        value = iDef->maxRangeInclusive() ?
          iDef->maxRange() : iDef->maxRange() - 1;
        validator->setTop(value);
        }

      slctk::IntItemPtr iItem =dynamicCastPointer<IntItem>(this->getObject());
      if(item->isSet(elementIdx))
        {
        editBox->setText(item->valueAsString(elementIdx).c_str());

        isDefault = iDef->hasDefault() &&
          iDef->defaultValue()==iItem->value(elementIdx);
        }
      else if(iDef->hasDefault())
        {
        editBox->setText(QString::number(iDef->defaultValue()));
        isDefault = true;
        }
      inputWidget = editBox;
      break;
      }
    case slctk::attribute::Item::STRING:
      {
      const StringItemDefinition *sDef = 
        dynamic_cast<const StringItemDefinition*>(item->definition().get());
      slctk::StringItemPtr sitem =dynamicCastPointer<StringItem>(this->getObject());
      QString valText;
      if(item->isSet(elementIdx))
        {
        valText = item->valueAsString(elementIdx).c_str();
        isDefault = sDef->hasDefault() &&
          sDef->defaultValue()==sitem->value(elementIdx);
        }
      else if(sDef->hasDefault())
        {
        valText = sDef->defaultValue().c_str();
        isDefault = true;
        }

      if(sDef->isMultiline())
        {
        QTextEdit* textEdit = new QTextEdit(this->Widget);
        textEdit->setPlainText(valText);
        QObject::connect(textEdit, SIGNAL(textChanged()),
          this, SLOT(onInputValueChanged()));
        inputWidget = textEdit;
        }
      else
        {
        QLineEdit* lineEdit = new QLineEdit(this->Widget);
        lineEdit->setText(valText);
        inputWidget = lineEdit;
        }
      inputWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      break;
      }
    default:
      //this->m_errorStatus << "Error: Unsupported Item Type: " <<
      // slctk::attribute::Item::type2String(item->type()) << "\n";
      break;
    }
  if(inputWidget)
    {
    inputWidget->setProperty("ElementIndex", vdata);
    qtUIManager::instance()->setWidgetToDefaultValueColor(inputWidget,isDefault);
    }
  if(QLineEdit* const editBox = qobject_cast<QLineEdit*>(inputWidget))
    {
    QObject::connect(editBox, SIGNAL(editingFinished()),
      this, SLOT(onInputValueChanged()), Qt::QueuedConnection);
    }

  return inputWidget;
}
//----------------------------------------------------------------------------
void qtInputsItem::onInputValueChanged()
{
  QLineEdit* const editBox = qobject_cast<QLineEdit*>(
    QObject::sender());
  QTextEdit* const textBox = qobject_cast<QTextEdit*>(
    QObject::sender());
  if(!editBox && !textBox)
    {
    return;
    }

  slctk::ValueItemPtr item =dynamicCastPointer<ValueItem>(this->getObject());
  slctk::DoubleItemPtr dItem =dynamicCastPointer<DoubleItem>(this->getObject());
  slctk::IntItemPtr iItem =dynamicCastPointer<IntItem>(this->getObject());
  slctk::StringItemPtr sItem =dynamicCastPointer<StringItem>(this->getObject());
  int elementIdx = editBox ? editBox->property("ElementIndex").toInt() :
    textBox->property("ElementIndex").toInt();

  if(editBox && !editBox->text().isEmpty())
    {
    if(dItem)
      {
      dItem->setValue(elementIdx, editBox->text().toDouble());
      }
    else if(iItem)
      {
      iItem->setValue(elementIdx, editBox->text().toInt());
      }
    else if(sItem)
      {
      sItem->setValue(elementIdx, editBox->text().toStdString());
      }
    else
      {
      item->unset(elementIdx);
      }
    }
  else if(textBox && !textBox->toPlainText().isEmpty() && sItem)
    {
    sItem->setValue(elementIdx, textBox->toPlainText().toStdString());
    }
  else
    {
    item->unset(elementIdx);
    }
}
