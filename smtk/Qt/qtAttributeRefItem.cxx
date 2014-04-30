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
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QGridLayout>

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
  this->setMinimumWidth(80);
}
/*
//-----------------------------------------------------------------------------
QSize qtAttRefCombo::sizeHint() const
{
  return QSize(150, this->height());
}
*/
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

  // <elementIdx, qtAtt>
  QMap<int, QPointer<qtAttribute> > RefAtts;
//  QPointer<qtAttribute> CurrentRefAtt;
//  QHBoxLayout* RefComboLayout;
  QPointer<QToolButton> EditButton;
  QPointer<QToolButton> CollapseButton;
  Qt::Orientation VectorItemOrient;
  bool UserSetAttVisibility;
};

//----------------------------------------------------------------------------
qtAttributeRefItem::qtAttributeRefItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p,  qtBaseView* view,
  Qt::Orientation enVectorItemOrient ) :
   qtItem(dataObj, p, view)
{
  this->Internals = new qtAttributeRefItemInternals;
  this->IsLeafItem = true;
  this->Internals->VectorItemOrient = enVectorItemOrient;
  this->Internals->UserSetAttVisibility = true;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtAttributeRefItem::~qtAttributeRefItem()
{
  foreach(int eleIdx, this->Internals->RefAtts.keys())
    {
    qtAttribute* qa = this->Internals->RefAtts[eleIdx];
    if(qa)
      {
      delete qa;
      }
    }
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::setLabelVisible(bool visible)
{
  if (this->Internals->theLabel)
    {
    this->Internals->theLabel->setVisible(visible);
    if(visible)
      {
      this->Widget->setMinimumWidth(250);
      }
    else
      {
      this->Widget->setMinimumWidth(150);
      }
    }
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
  this->setAttributesVisible(visible);
  this->Internals->UserSetAttVisibility = visible;
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::onToggleAttributeWidgetVisibility()
{
  foreach(qtAttribute* qa, this->Internals->RefAtts.values())
    {
    if(qa)
      {
      bool bVisible = qa->widget()->isVisible();
      this->setAttributesVisible(!bVisible);
      this->Internals->CollapseButton->setArrowType(bVisible ? Qt::UpArrow : Qt::DownArrow);
      break;
      }
    }
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::setAttributesVisible(bool visible)
{
  foreach(qtAttribute* qa, this->Internals->RefAtts.values())
    {
    if(qa && qa->widget()->isVisible() != visible)
      {
      qa->widget()->setVisible(visible);
      }
    }
  emit this->widgetResized();
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
  foreach(qtAttribute* qa, this->Internals->RefAtts.values())
    {
    this->updateAttWidgetState(qa);
    }
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
  this->Widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
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

  QGridLayout* thisLayout = new QGridLayout(this->Widget);
  thisLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  thisLayout->setContentsMargins(0, 0, 0, 0);
  // Setup combo layout
  QBoxLayout* comboLayout;
  if(this->Internals->VectorItemOrient == Qt::Vertical)
    {
    comboLayout = new QVBoxLayout();
    }
  else
    {
    comboLayout = new QHBoxLayout();
    }
  comboLayout->setMargin(0);
  comboLayout->setSpacing(6);
  comboLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QGridLayout* layout = new QGridLayout();
  layout->setContentsMargins(0, 5, 0, 0);
  layout->setSpacing(3);
  layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  this->Internals->CollapseButton = new QToolButton(this->Widget);
  //QString exapndDownName(":/icons/attribute/expand-down.png");
  this->Internals->CollapseButton->setFixedSize(QSize(16, 16));
  this->Internals->CollapseButton->setArrowType(Qt::DownArrow);
  //this->Internals->CollapseButton->setIcon(QIcon(exapndDownName));
  this->Internals->CollapseButton->setToolTip("Toggle referenced attribute widget");
  this->Internals->CollapseButton->setSizePolicy(sizeFixedPolicy);
  connect(this->Internals->CollapseButton, SIGNAL(clicked()),
    this, SLOT(onToggleAttributeWidgetVisibility()));

  this->Internals->EditButton = new QToolButton(this->Widget);
  QString resourceName(":/icons/attribute/edit.png");
  this->Internals->EditButton->setFixedSize(QSize(16, 16));
  this->Internals->EditButton->setIcon(QIcon(resourceName));
  this->Internals->EditButton->setSizePolicy(sizeFixedPolicy);
  this->Internals->EditButton->setToolTip("Manage referenced attributes");

  this->Widget->setMinimumWidth(250); //combobox width, edit button, collapse button, spacing.

  connect(this->Internals->EditButton, SIGNAL(clicked()),
    this, SLOT(onLaunchAttributeView()));

  smtk::attribute::ItemPtr dataObj = this->getObject();
  QString lText = dataObj->label().empty() ?
     dataObj->name().c_str() : dataObj->label().c_str();

  int padding = 0;
  int gridCol = 0;
  if(dataObj->isOptional())
    {
    this->Internals->optionalCheck = new QCheckBox(this->Widget);
    this->Internals->optionalCheck->setChecked(dataObj->definition()->isEnabledByDefault());
    this->Internals->optionalCheck->setText(" ");// some space
    this->Internals->optionalCheck->setSizePolicy(sizeFixedPolicy);

   if(dataObj->definition()->briefDescription().length())
      {
      this->Internals->optionalCheck->setToolTip(
        dataObj->definition()->briefDescription().c_str());
      }
    padding = this->Internals->optionalCheck->iconSize().width() + 3; // 3 is for layout spacing

    QObject::connect(this->Internals->optionalCheck,
      SIGNAL(stateChanged(int)),
      this, SLOT(setOutputOptional(int)));
    layout->addWidget(this->Internals->optionalCheck, 0, gridCol++);
    }

  this->Internals->theLabel = new QLabel(lText, this->Widget);

  this->Internals->theLabel->setFixedWidth(this->baseView()->fixedLabelWidth() - padding);
  this->Internals->theLabel->setSizePolicy(sizeFixedPolicy);
  this->Internals->theLabel->setWordWrap(true);
  this->Internals->theLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  if(dataObj->definition()->advanceLevel() >0)
    {
    this->Internals->theLabel->setFont(
      this->baseView()->uiManager()->advancedFont());
    }

  layout->addWidget(this->Internals->theLabel, 0, gridCol++);

//  this->Internals->RefComboLayout->addWidget(this->Internals->EditButton);
  for(i = 0; i < n; i++)
    {
    qtAttRefCombo* combo = new qtAttRefCombo(item, this->Widget);
    QVariant vdata(static_cast<int>(i));
    combo->setProperty("ElementIndex", vdata);
    QVBoxLayout* childLayout = new QVBoxLayout;
    childLayout->setMargin(0);
    childLayout->setSpacing(6);

    childLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    childLayout->addWidget(combo);
    QVariant vlayoutdata;
    vlayoutdata.setValue(static_cast<void*>(childLayout));
    combo->setProperty("MyLayout", vlayoutdata);
    this->Internals->comboBoxes.push_back(combo);
    combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    comboLayout->addLayout(childLayout);
    QObject::connect(combo,  SIGNAL(currentIndexChanged(int)),
      this, SLOT(onInputValueChanged()), Qt::QueuedConnection);
    }

//  QBoxLayout* editlayout = new QHBoxLayout();
//  editlayout->setMargin(0);
//  editlayout->addLayout(comboLayout);
//  editlayout->addWidget(this->Internals->EditButton);
//  editlayout->addWidget(this->Internals->CollapseButton);

  layout->addWidget(this->Internals->EditButton, 0, gridCol++);
  layout->addWidget(this->Internals->CollapseButton, 0, gridCol);

  QVBoxLayout* labelLayout = new QVBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  labelLayout->addLayout(layout);

  //layout->addLayout(this->Internals->RefComboLayout);
  thisLayout->addLayout(labelLayout, 0, 0);
  thisLayout->addLayout(comboLayout, 0, 1);
  this->updateItemData();
}

//----------------------------------------------------------------------------
void qtAttributeRefItem::updateAttWidgetState(qtAttribute* qa)
{
  if(qa && qa->widget())
    {
    bool bVisible = ( this->Internals->UserSetAttVisibility &&
      this->Internals->CollapseButton->arrowType() == Qt::DownArrow );
    qa->widget()->setVisible(bVisible);
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
    this->refreshUI(combo);
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
  foreach(qtAttribute* qa, this->Internals->RefAtts.values())
    {
    if(qa)
      {
      qa->widget()->setEnabled(enable);
      }
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
  if(curIdx>0) // index 0 is None
    {
    const RefItemDefinition *itemDef =
      dynamic_cast<const RefItemDefinition*>(item->definition().get());
    attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
    Manager *attManager = attDef->manager();
    if(curIdx == comboBox->count() - 1) // create New attribute
      {
      QList<smtk::attribute::DefinitionPtr> AllDefs;
      this->baseView()->getDefinitions(attDef, AllDefs);
      QList<QString> defTypes;
      QList<QString> defLabels;
      foreach (smtk::attribute::DefinitionPtr aDef, AllDefs)
        {
        if(!aDef->isAbstract())
          {
          std::string txtDef = aDef->label().empty() ?
            aDef->type() : aDef->label();
          defLabels.push_back(txtDef.c_str());
          defTypes.push_back(aDef->type().c_str());
          }
        }
      if(defTypes.count() > 0)
        {
        std::string attName = attManager->createUniqueName(defTypes[0].toStdString());
        qtNewAttributeWidget attDialog(comboBox);
        attDialog.setBaseWidget(comboBox);
        if(attDialog.showWidget(attName.c_str(), defLabels) == QDialog::Accepted &&
          !attDialog.attributeName().isEmpty())
          {
          int defIndex = defLabels.indexOf(attDialog.attributeType());
          attPtr = attManager->createAttribute(attDialog.attributeName().toStdString(),
            defTypes[defIndex].toStdString());
          comboBox->blockSignals(true);
          comboBox->insertItem(1, attDialog.attributeName());
          comboBox->setCurrentIndex(1);
          comboBox->blockSignals(false);
          }
        else
          {
          int idx = 0;
          // reset combo text
          if(item->isSet(elementIdx))
            {
            attPtr = item->value(elementIdx);
            idx = comboBox->findText(attPtr->name().c_str());
            }
          comboBox->blockSignals(true);
          comboBox->setCurrentIndex(idx<0 ? 0 : idx);
          comboBox->blockSignals(false);
          }
        }
      else
        {
        QMessageBox::warning(this->Widget, tr("Create Attribute"),
          tr("No attribute definition, or all definitions are abstract!"));
        }
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
    qtAttribute* currentAtt =
      this->Internals->RefAtts.contains(elementIdx) ?
      this->Internals->RefAtts[elementIdx] : NULL;
    if(currentAtt && currentAtt->getObject() != attPtr)
      {
      delete currentAtt->widget();
      delete currentAtt;
      currentAtt = NULL;
      }

    if(!currentAtt)
      {
      int currentLen = this->baseView()->fixedLabelWidth();
      int tmpLen = this->baseView()->uiManager()->getWidthOfAttributeMaxLabel(
        attPtr->definition(), this->baseView()->uiManager()->advancedFont());
      this->baseView()->setFixedLabelWidth(tmpLen);
      currentAtt = new qtAttribute(attPtr, this->Widget, this->baseView());
      this->baseView()->setFixedLabelWidth(currentLen);
      QFrame* attFrame = qobject_cast<QFrame*>(currentAtt->widget());
      if(attFrame)
        {
        attFrame->setFrameShape(QFrame::Box);
        }
//      QGridLayout* parentGrid = static_cast<QGridLayout*>(this->Widget->layout());
      QBoxLayout* mylayout =
        static_cast<QBoxLayout*>(comboBox->property("MyLayout").value<void *>());
      mylayout->insertWidget(1, currentAtt->widget());
      QVariant vrefdata;
      vrefdata.setValue(static_cast<void*>(currentAtt));
      comboBox->setProperty("QtRefAtt", vrefdata);
      this->Internals->RefAtts[elementIdx] = currentAtt;
      }
    this->updateAttWidgetState(currentAtt);
    }
  else // check if we need to remove anything
    {
    QVariant myRefAtt = comboBox->property("QtRefAtt");
    if(myRefAtt.isValid())
      {
      qtAttribute* qa = static_cast<qtAttribute*>(myRefAtt.value<void *>());
      if(qa)
        {
        this->Internals->RefAtts[elementIdx] = NULL;
        delete qa->widget();
        delete qa;
        }
      }
    }

  if(valChanged)
    {
    this->baseView()->valueChanged(this->getObject());
    }
}
