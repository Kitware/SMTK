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
#include "smtk/Qt/qtAttributeView.h"
#include "smtk/Qt/qtNewAttributeWidget.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/view/Attribute.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPointer>
#include <QToolButton>
#include <QCheckBox>
#include <QDialogButtonBox>

using namespace smtk::attribute;

inline void init_Att_Names_and_NEW(QList<QString>& attNames,
  const RefItemDefinition *itemDef)
{
  smtk::attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
  if(!attDef)
    {
    return;
    }
  attNames.push_back("None");
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

  RefItemPtr refitem =
    smtk::dynamic_pointer_cast<RefItem>(this->m_RefItem.lock());
  int elementIdx = this->property("ElementIndex").toInt();
  int setIndex = 0; // None
  if (refitem->isSet(elementIdx))
    {
    setIndex = attNames.indexOf(refitem->valueAsString(elementIdx).c_str());
    }
  this->clear();
  this->addItems(attNames);
  this->setCurrentIndex(setIndex);
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
  QPointer<qtAttribute> CurrentRefAtt;
  QHBoxLayout* RefComboLayout;
  QPointer<QToolButton> EditButton;
  QPointer<QToolButton> CollapseButton;
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
  if(this->Internals->CurrentRefAtt)
    {
    delete this->Internals->CurrentRefAtt->widget();
    delete this->Internals->CurrentRefAtt;
    }
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::setAttributeEditorVisible(bool visible)
{
  this->Internals->EditButton->setVisible(visible);
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::setAttributeWidgetVisible(bool visible)
{
  this->Internals->CollapseButton->setVisible(visible);
  if(this->Internals->CurrentRefAtt &&
     this->Internals->CurrentRefAtt->widget()->isVisible() != visible)
    {
    this->onToggleAttributeWidgetVisibility();
    }
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::onToggleAttributeWidgetVisibility()
{
  if(this->Internals->CurrentRefAtt)
    {
    bool bVisible = this->Internals->CurrentRefAtt->widget()->isVisible();
    this->Internals->CurrentRefAtt->widget()->setVisible(!bVisible);
    this->Internals->CollapseButton->setArrowType(bVisible ? Qt::UpArrow : Qt::DownArrow);
    //QString exapndDownName = bVisible ? ":/icons/attribute/expand-down.png" :
    //  ":/icons/attribute/expand-up.png";
    //this->Internals->CollapseButton->setIcon(QIcon(exapndDownName));
    }
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::onLaunchAttributeView()
{
  if(!this->getObject())
    {
    return;
    }
  smtk::view::AttributePtr newAttView(new smtk::view::Attribute("Attribute View"));
  smtk::attribute::RefItemPtr item =
    smtk::dynamic_pointer_cast<RefItem>(this->getObject());
  const RefItemDefinition *itemDef =
    dynamic_cast<const RefItemDefinition*>(item->definition().get());
  attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
  newAttView->addDefinition(attDef);

  QDialog attViewDlg;
  attViewDlg.setWindowTitle(attDef->label().empty() ?
                            attDef->type().c_str() : attDef->label().c_str());
  QVBoxLayout* layout = new QVBoxLayout(&attViewDlg);

  qtAttributeView attView(newAttView, &attViewDlg, this->baseView()->uiManager());
  //layout->addWidget(attView.widget())
  QDialogButtonBox* buttonBox=new QDialogButtonBox( &attViewDlg );
  buttonBox->setStandardButtons(QDialogButtonBox::Ok);
  layout->addWidget(buttonBox);
  attViewDlg.setModal(true);
  QObject::connect(buttonBox, SIGNAL(accepted()), &attViewDlg, SLOT(accept()));
  attViewDlg.exec();

  this->updateItemData();
  this->updateAttWidgetState();
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

  this->Internals->CollapseButton = new QToolButton(this->Widget);
  //QString exapndDownName(":/icons/attribute/expand-down.png");
  this->Internals->CollapseButton->setFixedSize(QSize(16, 16));
  this->Internals->CollapseButton->setArrowType(Qt::DownArrow);
  //this->Internals->CollapseButton->setIcon(QIcon(exapndDownName));
  this->Internals->CollapseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  connect(this->Internals->CollapseButton, SIGNAL(clicked()),
    this, SLOT(onToggleAttributeWidgetVisibility()));

  this->Internals->EditButton = new QToolButton(this->Widget);
  QString resourceName(":/icons/attribute/edit.png");
  this->Internals->EditButton->setFixedSize(QSize(16, 16));
  this->Internals->EditButton->setIcon(QIcon(resourceName));
  this->Internals->EditButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  connect(this->Internals->EditButton, SIGNAL(clicked()),
    this, SLOT(onLaunchAttributeView()));

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

//  this->Internals->RefComboLayout->addWidget(this->Internals->EditButton);
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
  layout->addWidget(this->Internals->EditButton);
  layout->addWidget(this->Internals->CollapseButton);
  thisLayout->addLayout(layout);
  this->updateItemData();
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::updateAttWidgetState()
{
  if(this->Internals->CurrentRefAtt)
    {
    bool bVisible = ( this->Internals->CollapseButton->isVisible() &&
      this->Internals->CollapseButton->arrowType() == Qt::DownArrow );
    this->Internals->CurrentRefAtt->widget()->setVisible(bVisible);
    }
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
    int setIndex = 0; // None
    if (item->isSet(elementIdx))
      {
      setIndex = attNames.indexOf(item->valueAsString(elementIdx).c_str());
      setIndex = setIndex < 0 ? 0 : setIndex;
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
  this->Internals->CollapseButton->setEnabled(enable);
  foreach(QComboBox* combo, this->Internals->comboBoxes)
    {
    combo->setEnabled(enable);
    }
  if(this->Internals->CurrentRefAtt)
    {
    this->Internals->CurrentRefAtt->widget()->setEnabled(enable);
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
  this->updateAttWidgetState();
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::refreshUI(QComboBox* comboBox)
{
  int curIdx = comboBox->currentIndex();
  int elementIdx = comboBox->property("ElementIndex").toInt();

  smtk::attribute::RefItemPtr item =dynamic_pointer_cast<RefItem>(this->getObject());
  AttributePtr attPtr;
  bool valChanged = true;
  if(curIdx>0) // index 0 is None
    {
    const RefItemDefinition *itemDef =
      dynamic_cast<const RefItemDefinition*>(item->definition().get());
    attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
    Manager *attManager = attDef->manager();
    if(curIdx == comboBox->count() - 1) // create New attribute
      {
      attPtr = attManager->createAttribute(attDef->type());

      qtNewAttributeWidget attDialog(comboBox);
      attDialog.setBaseWidget(comboBox);
      if(attDialog.showWidget(attPtr->name().c_str()) == QDialog::Accepted &&
        !attDialog.attributeName().isEmpty() &&
         attDialog.attributeName().toStdString() != attPtr->name())
        {
        attManager->rename(attPtr, attDialog.attributeName().toStdString());
        }

      comboBox->blockSignals(true);
      comboBox->insertItem(1, attPtr->name().c_str());
      comboBox->setCurrentIndex(1);
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
    if(this->Internals->CurrentRefAtt && this->Internals->CurrentRefAtt->getObject() != attPtr)
      {
      delete this->Internals->CurrentRefAtt->widget();
      delete this->Internals->CurrentRefAtt;
      this->Internals->CurrentRefAtt = NULL;
      }
    if(!this->Internals->CurrentRefAtt)
      {
      this->Internals->CurrentRefAtt = new qtAttribute(attPtr, this->Widget, this->baseView());
      QFrame* attFrame = qobject_cast<QFrame*>(this->Internals->CurrentRefAtt->widget());
      if(attFrame)
        {
        attFrame->setFrameShape(QFrame::Box);
        }
      this->Widget->layout()->addWidget(this->Internals->CurrentRefAtt->widget());
      }
    }
  else if(this->Internals->CurrentRefAtt)
    {
    delete this->Internals->CurrentRefAtt->widget();
    delete this->Internals->CurrentRefAtt;
    this->Internals->CurrentRefAtt = NULL;
    }

  if(valChanged)
    {
    this->baseView()->valueChanged(this->getObject());
    }
}
