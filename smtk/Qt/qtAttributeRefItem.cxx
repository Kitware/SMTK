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

#include "smtk/Qt/qtAttribute.h"
#include "smtk/Qt/qtUIManager.h"
#include "smtk/Qt/qtBaseView.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPointer>
#include <QToolButton>
#include <QCheckBox>

using namespace smtk::attribute;

inline void init_Att_Names_and_NEW(QList<QString>& attNames,
  const RefItemDefinition *itemDef)
{
  smtk::attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
  if(!attDef)
    {
    return;
    }
  std::vector<smtk::attribute::AttributePtr> result;
  Manager *attManager = attDef->manager();
  attManager->findAttributes(attDef, result);
  std::vector<smtk::attribute::AttributePtr>::iterator it;
  for (it=result.begin(); it!=result.end(); ++it)
    {
    attNames.push_back((*it)->name().c_str());
    }
  attNames.push_back("New");
}

//-----------------------------------------------------------------------------
qtAttRefCombo::qtAttRefCombo(smtk::attribute::ItemPtr refitem, QWidget * inParent)
: QComboBox(inParent), m_RefItem(refitem)
{
}

//-----------------------------------------------------------------------------
void qtAttRefCombo::showPopup()
{
  QList<QString> attNames;
  if(!this->m_RefItem.lock())
    {
    this->QComboBox::showPopup();
    return;
    }
  // need to update the list, since it may be changed
  const RefItemDefinition *itemDef =
    dynamic_cast<const RefItemDefinition*>(
    this->m_RefItem.lock()->definition().get());
  init_Att_Names_and_NEW(attNames, itemDef);
  this->blockSignals(true);
  int currentIndex = attNames.indexOf(this->currentText());
  this->clear();
  this->addItems(attNames);
  this->setCurrentIndex(currentIndex);
  this->blockSignals(false);

  this->QComboBox::showPopup();
}

//----------------------------------------------------------------------------
class qtAttributeRefItemInternals
{
public:
  QList<qtAttRefCombo*> comboBoxes;
  QPointer<QCheckBox> optionalCheck;
  QPointer<QLabel> theLabel;
  QPointer<qtAttribute> CurretRefAtt;
  QHBoxLayout* RefComboLayout;
  QPointer<QToolButton> EditButton;
};

//----------------------------------------------------------------------------
qtAttributeRefItem::qtAttributeRefItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p,  qtBaseView* view) :
   qtItem(dataObj, p, view)
{
  this->Internals = new qtAttributeRefItemInternals;
  this->IsLeafItem = true;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtAttributeRefItem::~qtAttributeRefItem()
{
  if(this->Internals->CurretRefAtt)
    {
    delete this->Internals->CurretRefAtt->widget();
    delete this->Internals->CurretRefAtt;
    }
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
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

  QVBoxLayout* thisLayout = new QVBoxLayout(this->Widget);
  QBoxLayout* layout = new QHBoxLayout();
  layout->setAlignment(Qt::AlignLeft);
  this->Internals->RefComboLayout = new QHBoxLayout();
  this->Internals->RefComboLayout->setMargin(0);

  this->Internals->EditButton = new QToolButton(this->Widget);
  QString resourceName(":/icons/attribute/edit.png");
  this->Internals->EditButton->setFixedSize(QSize(16, 16));
  this->Internals->EditButton->setIcon(QIcon(resourceName));
  this->Internals->EditButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  this->Internals->EditButton->setCheckable(true);
  this->Internals->EditButton->setChecked(true);

  connect(this->Internals->EditButton, SIGNAL(toggled(bool)),
    this, SLOT(showAttributeEditor(bool)));

  layout->setMargin(0);
  smtk::attribute::ItemPtr dataObj = this->getObject();

  if(dataObj->isOptional())
    {
    this->Internals->optionalCheck = new QCheckBox(this->Widget);
    this->Internals->optionalCheck->setChecked(dataObj->definition()->isEnabledByDefault());
    QSizePolicy sizeFixedPolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    this->Internals->optionalCheck->setSizePolicy(sizeFixedPolicy);

    if(dataObj->definition()->advanceLevel() >0)
      {
      this->Internals->optionalCheck->setFont(
        this->baseView()->uiManager()->advancedFont());
      }
    if(dataObj->definition()->briefDescription().length())
      {
      this->Internals->optionalCheck->setToolTip(
        dataObj->definition()->briefDescription().c_str());
      }

    QObject::connect(this->Internals->optionalCheck,
      SIGNAL(stateChanged(int)),
      this, SLOT(setOutputOptional(int)));
    layout->addWidget(this->Internals->optionalCheck);
    }

  this->Internals->RefComboLayout->addWidget(this->Internals->EditButton);
  for(i = 0; i < n; i++)
    {
    qtAttRefCombo* combo = new qtAttRefCombo(item, this->Widget);
    QVariant vdata(static_cast<int>(i));
    combo->setProperty("ElementIndex", vdata);
    this->Internals->comboBoxes.push_back(combo);
    this->Internals->RefComboLayout->addWidget(combo);
    QObject::connect(combo,  SIGNAL(currentIndexChanged(int)),
      this, SLOT(onInputValueChanged()), Qt::QueuedConnection);
    }
  QString lText = dataObj->label().c_str();
  this->Internals->theLabel = new QLabel(lText);
  layout->addWidget(this->Internals->theLabel);
  layout->addLayout(this->Internals->RefComboLayout);
  thisLayout->addLayout(layout);
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
  init_Att_Names_and_NEW(attNames, itemDef);

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
  if(this->Internals->comboBoxes.count())
    {
    this->refreshUI(this->Internals->comboBoxes[0]);
    }
  if(item->isOptional() && this->Internals->optionalCheck)
    {
    if(this->Internals->optionalCheck->isChecked() == item->isEnabled())
      {
      this->setOutputOptional(this->Internals->optionalCheck->checkState());
      }
    else
      {
      this->Internals->optionalCheck->setChecked(item->isEnabled());
      }
    }
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::setOutputOptional(int state)
{
  bool enable = state ? true : false;

  this->Internals->EditButton->setEnabled(enable);
  foreach(QComboBox* combo, this->Internals->comboBoxes)
    {
    combo->setEnabled(enable);
    }
  if(this->Internals->CurretRefAtt)
    {
    this->Internals->CurretRefAtt->widget()->setEnabled(enable);
    }
  if(enable != this->getObject()->isEnabled())
    {
    this->getObject()->setIsEnabled(enable);
    this->baseView()->valueChanged(this->getObject());
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
  this->refreshUI(comboBox);
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::refreshUI(QComboBox* comboBox)
{
  int curIdx = comboBox->currentIndex();
  int elementIdx = comboBox->property("ElementIndex").toInt();

  smtk::attribute::RefItemPtr item =dynamic_pointer_cast<RefItem>(this->getObject());
  AttributePtr attPtr;
  bool valChanged = true;
  if(curIdx>=0)
    {
    const RefItemDefinition *itemDef =
      dynamic_cast<const RefItemDefinition*>(item->definition().get());
    attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
    Manager *attManager = attDef->manager();
    if(curIdx == comboBox->count() - 1)
      {
      attPtr = attManager->createAttribute(attDef->type());
      comboBox->blockSignals(true);
      comboBox->insertItem(0, attPtr->name().c_str());
      comboBox->setCurrentIndex(0);
      comboBox->blockSignals(false);
      }
    else
      {
      attPtr = attManager->findAttribute(comboBox->currentText().toStdString());
      }

    if(elementIdx >=0 && static_cast<int>(item->numberOfValues()) > elementIdx &&
      item->isSet(elementIdx) && attPtr == item->value(elementIdx))
      {
      valChanged = false; // nothing to do
      }
    else
      {
      if(attPtr)
        {
        item->setValue(elementIdx, attPtr);
        }
      else
        {
        item->unset(elementIdx);
        }
      }
    }
  else
    {
    item->unset(elementIdx);
    }
  if(attPtr)
    {
    if(this->Internals->CurretRefAtt && this->Internals->CurretRefAtt->getObject() != attPtr)
      {
      delete this->Internals->CurretRefAtt->widget();
      delete this->Internals->CurretRefAtt;
      this->Internals->CurretRefAtt = NULL;
      }
    if(!this->Internals->CurretRefAtt)
      {
      this->Internals->CurretRefAtt = new qtAttribute(attPtr, this->Widget, this->baseView());
      QFrame* attFrame = qobject_cast<QFrame*>(this->Internals->CurretRefAtt->widget());
      if(attFrame)
        {
        attFrame->setFrameShape(QFrame::Box);
        }
      this->Widget->layout()->addWidget(this->Internals->CurretRefAtt->widget());
      }

    this->showAttributeEditor(this->Internals->EditButton->isChecked());
    }
  if(valChanged)
    {
    this->baseView()->valueChanged(this->getObject());
    }
}
//----------------------------------------------------------------------------
void qtAttributeRefItem::showAttributeEditor(bool showEditor)
{
  if(this->Internals->CurretRefAtt)
    {
    this->Internals->CurretRefAtt->widget()->setVisible(showEditor);
    }
}
