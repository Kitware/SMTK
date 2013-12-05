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

#include "smtk/Qt/qtInputsItem.h"

#include "smtk/Qt/qtUIManager.h"

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
#include <QComboBox>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtInputsItemInternals
{
public:

  QPointer<QFrame> EntryFrame;
};

//----------------------------------------------------------------------------
qtInputsItem::qtInputsItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p) : qtItem(dataObj, p)
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
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if(!dataObj || !this->passAdvancedCheck() ||
    !qtUIManager::instance()->passItemCategoryCheck(
      dataObj->definition()))
    {
    return;
    }

  this->clearChildItems();
  this->updateUI();
}

//----------------------------------------------------------------------------
void qtInputsItem::loadInputValues(
  QBoxLayout* labellayout, QBoxLayout* entrylayout)
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!item)
    {
    return;
    }

  int n = static_cast<int>(item->numberOfValues());
  if (!n)
    {
    return;
    }

  const ValueItemDefinition *itemDef = 
    dynamic_cast<const ValueItemDefinition*>(item->definition().get());

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  for(int i = 0; i < n; i++)
    {
    QWidget* editBox = qtUIManager::instance()->createInputWidget(
      item, i, this->Widget);
    if(!editBox)
      {
      continue;
      }

    if(n!=1)
      {
      std::string componentLabel = itemDef->valueLabel(i);
      if(!componentLabel.empty())
        {
        // acbauer -- this should probably be improved to look nicer
        QString labelText = componentLabel.c_str();
        QLabel* label = new QLabel(labelText, editBox);
        label->setSizePolicy(sizeFixedPolicy);
        entrylayout->addWidget(label);
        }
      }
    entrylayout->addWidget(editBox);
    }
}

//----------------------------------------------------------------------------
void qtInputsItem::updateUI()
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if(!dataObj || !this->passAdvancedCheck() ||
    !qtUIManager::instance()->passItemCategoryCheck(
      dataObj->definition()))
    {
    return;
    }

  if(this->Internals->EntryFrame)
    {
    this->Widget->layout()->removeWidget(this->Internals->EntryFrame);
    delete this->Internals->EntryFrame;
    }

  this->Widget = new QFrame(this->parentWidget());
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  this->Internals->EntryFrame = new QFrame(this->parentWidget());
  this->Internals->EntryFrame->setObjectName("CheckAndEntryInputFrame");
  QHBoxLayout* entryLayout = new QHBoxLayout(this->Internals->EntryFrame);
  entryLayout->setMargin(0);
  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setAlignment(Qt::AlignLeft);

  if(dataObj->isOptional())
    {
    QCheckBox* optionalCheck = new QCheckBox(this->parentWidget());
    optionalCheck->setChecked(dataObj->isEnabled());
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)),
      this, SLOT(setOutputOptional(int)));
    this->Internals->EntryFrame->setEnabled(
      dataObj->definition()->isEnabledByDefault());
    labelLayout->addWidget(optionalCheck);
    }
  smtk::attribute::ValueItemPtr item = dynamic_pointer_cast<ValueItem>(dataObj);
  const ValueItemDefinition *itemDef = 
    dynamic_cast<const ValueItemDefinition*>(dataObj->definition().get());

  QString labelText;
  if(!item->label().empty())
    {
    labelText = item->label().c_str();
    }
  QLabel* label = new QLabel(labelText, this->Widget);
  label->setSizePolicy(sizeFixedPolicy);

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
    label->setFont(qtUIManager::instance()->advancedFont());
    }
  labelLayout->addWidget(label);

  this->loadInputValues(labelLayout, entryLayout);

  entryLayout->setAlignment(Qt::AlignLeft);
  layout->addLayout(labelLayout);
  layout->addWidget(this->Internals->EntryFrame);
  layout->setAlignment(Qt::AlignTop);
  if(this->parentWidget()->layout())
    {
    this->parentWidget()->layout()->addWidget(this->Widget);
    }
}

//----------------------------------------------------------------------------
void qtInputsItem::setOutputOptional(int state)
{
  this->getObject()->setIsEnabled(state ? true : false);
  this->Internals->EntryFrame->setEnabled(state);
}
