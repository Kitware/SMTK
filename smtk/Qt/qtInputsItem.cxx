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
#include "smtk/Qt/qtBaseView.h"

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
#include <QToolButton>

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
#include "smtk/attribute/ValueItemTemplate.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtInputsItemInternals
{
public:

  QPointer<QGridLayout> EntryLayout;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;

  // for discrete items that with potential child widget
  // <Enum-Combo, QPair<child-layout, child-Widget> >
  QMap<QWidget*, QPair<QLayout*, QWidget*> >ChildrenMap;

  // for extensible items
  QMap<QToolButton*, QPair<QLayout*, QWidget*> > ExtensibleMap;
  QList<QToolButton*> MinusButtonIndices;
  QPointer<QToolButton> AddItemButton;
};

//----------------------------------------------------------------------------
qtInputsItem::qtInputsItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p, qtBaseView* bview,
   Qt::Orientation enVectorItemOrient) : qtItem(dataObj, p, bview)
{
  this->Internals = new qtInputsItemInternals;
  this->IsLeafItem = true;
  this->Internals->VectorItemOrient = enVectorItemOrient;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtInputsItem::~qtInputsItem()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtInputsItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
}

//----------------------------------------------------------------------------
void qtInputsItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if(!dataObj || !this->passAdvancedCheck() ||
    !this->baseView()->uiManager()->passItemCategoryCheck(
      dataObj->definition()))
    {
    return;
    }

  this->clearChildWidgets();
  this->updateUI();
}
//----------------------------------------------------------------------------
void qtInputsItem::addInputEditor(int i)
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
  QBoxLayout* childLayout = NULL;
  if(item->isDiscrete())
    {
    childLayout = new QVBoxLayout;
    childLayout->setContentsMargins(12, 3, 3, 0);
    childLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    }

  QWidget* editBox = this->baseView()->uiManager()->createInputWidget(
    item, i, this->Widget, this->baseView(), childLayout);
  if(!editBox)
    {
    return;
    }

  const ValueItemDefinition *itemDef = 
    dynamic_cast<const ValueItemDefinition*>(item->definition().get());
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QBoxLayout* editorLayout = new QHBoxLayout;
  editorLayout->setMargin(0);
  editorLayout->setSpacing(3);
  if(item->isExtensible())
    {
    QToolButton* minusButton = new QToolButton(this->Widget);
    QString iconName(":/icons/attribute/minus.png");
    minusButton->setFixedSize(QSize(12, 12));
    minusButton->setIcon(QIcon(iconName));
    minusButton->setSizePolicy(sizeFixedPolicy);
    minusButton->setToolTip("Remove value");
    editorLayout->addWidget(minusButton);
    connect(minusButton, SIGNAL(clicked()),
      this, SLOT(onRemoveValue()));
    QPair<QLayout*, QWidget*> pair;
    pair.first = editorLayout;
    pair.second = editBox;
    this->Internals->ExtensibleMap[minusButton] = pair;
    this->Internals->MinusButtonIndices.push_back(minusButton);
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
      editorLayout->addWidget(label);
      }
    }
  editorLayout->addWidget(editBox);

  // always going vertical for discrete and extensible items
  if(this->Internals->VectorItemOrient == Qt::Vertical ||
     item->isDiscrete() || item->isExtensible())
    {
    int row = item->isDiscrete() ? 2*i : i;
    // The "Add New Value" button is in first row, so take that into account
    row = item->isExtensible() ? row+1 : row;
    this->Internals->EntryLayout->addLayout(editorLayout, row, 1);

    // there could be conditional children, so we need another layout
    // so that the combobox will stay TOP-left when there are multiple
    // combo boxes.
    if(item->isDiscrete() && childLayout)
      {
      this->Internals->EntryLayout->addLayout(childLayout, row+1, 0, 1, 2);
      }
    }
  else // going horizontal
    {
    this->Internals->EntryLayout->addLayout(editorLayout, 0, i+1);
    }

  QPair<QLayout*, QWidget*> pair;
  pair.first = childLayout;
  pair.second = (childLayout && childLayout->count()>0) ?
    childLayout->itemAt(0)->widget() : NULL;
  this->Internals->ChildrenMap[editBox] = pair;
  this->updateExtensibleState();
}

//----------------------------------------------------------------------------
void qtInputsItem::loadInputValues()
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!item)
    {
    return;
    }

  int n = static_cast<int>(item->numberOfValues());
  if (!n && !item->isExtensible())
    {
    return;
    }

  if(item->isExtensible())
    {
    if(!this->Internals->AddItemButton)
      {
      QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      this->Internals->AddItemButton = new QToolButton(this->Widget);
      QString iconName(":/icons/attribute/plus.png");
      this->Internals->AddItemButton->setText("Add New Value");
      this->Internals->AddItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

//      this->Internals->AddItemButton->setFixedSize(QSize(12, 12));
      this->Internals->AddItemButton->setIcon(QIcon(iconName));
      this->Internals->AddItemButton->setSizePolicy(sizeFixedPolicy);
      connect(this->Internals->AddItemButton, SIGNAL(clicked()),
        this, SLOT(onAddNewValue()));
      this->Internals->EntryLayout->addWidget(this->Internals->AddItemButton, 0, 1);
      }
    }

  for(int i = 0; i < n; i++)
    {
    this->addInputEditor(i);
    }
}

//----------------------------------------------------------------------------
void qtInputsItem::updateUI()
{
  //smtk::attribute::ItemPtr dataObj = this->getObject();
  smtk::attribute::ValueItemPtr dataObj =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!dataObj || !this->passAdvancedCheck() ||
    !this->baseView()->uiManager()->passItemCategoryCheck(
      dataObj->definition()))
    {
    return;
    }

  this->Widget = new QFrame(this->parentWidget());
  this->Internals->EntryLayout = new QGridLayout(this->Widget);
  this->Internals->EntryLayout->setMargin(0);
  this->Internals->EntryLayout->setSpacing(0);
  this->Internals->EntryLayout->setAlignment( Qt::AlignLeft | Qt::AlignTop );

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if(dataObj->isOptional())
    {
    QCheckBox* optionalCheck = new QCheckBox(this->parentWidget());
    optionalCheck->setChecked(dataObj->isEnabled());
    optionalCheck->setText(" ");
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = optionalCheck->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)),
      this, SLOT(setOutputOptional(int)));
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
  else
    {
    labelText = item->name().c_str();
    }
  QLabel* label = new QLabel(labelText, this->Widget);
  label->setSizePolicy(sizeFixedPolicy);
  label->setFixedWidth(this->baseView()->fixedLabelWidth() - padding);
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

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

  this->loadInputValues();

  // we need this layout so that for items with conditionan children,
  // the label will line up at Top-left against the chilren's widgets.
//  QVBoxLayout* vTLlayout = new QVBoxLayout;
//  vTLlayout->setMargin(0);
//  vTLlayout->setSpacing(0);
//  vTLlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
//  vTLlayout->addLayout(labelLayout);
  this->Internals->EntryLayout->addLayout(labelLayout, 0, 0);
//  layout->addWidget(this->Internals->EntryFrame, 0, 1);
  if(this->parentWidget() && this->parentWidget()->layout())
    {
    this->parentWidget()->layout()->addWidget(this->Widget);
    }
  if(dataObj->isOptional())
    {
    this->setOutputOptional(dataObj->isEnabled() ? 1 : 0);
    }
}

//----------------------------------------------------------------------------
void qtInputsItem::setOutputOptional(int state)
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!item)
    {
    return;
    }
  bool enable = state ? true : false;
  if(item->isExtensible())
    {
    if(this->Internals->AddItemButton)
      {
      this->Internals->AddItemButton->setEnabled(enable);
      }
    foreach(QToolButton* tButton, this->Internals->ExtensibleMap.keys())
      {
      tButton->setEnabled(enable);
      }
   }

  foreach(QWidget* widget, this->Internals->ChildrenMap.keys())
    {
    if(this->Internals->ChildrenMap.value(widget).second)
      {
      this->Internals->ChildrenMap.value(widget).second->setEnabled(enable);
      }
    widget->setEnabled(enable);
    }

//  this->Internals->EntryFrame->setEnabled(enable);
  if(enable != this->getObject()->isEnabled())
    {
    this->getObject()->setIsEnabled(enable);
    this->baseView()->valueChanged(this->getObject());
    }
}

//----------------------------------------------------------------------------
void qtInputsItem::onAddNewValue()
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!item)
    {
    return;
    }
  if(item->setNumberOfValues(item->numberOfValues() + 1))
    {
//    QBoxLayout* entryLayout = qobject_cast<QBoxLayout*>(
//      this->Internals->EntryFrame->layout());
    this->addInputEditor(static_cast<int>(item->numberOfValues()) - 1);
    }
}

//----------------------------------------------------------------------------
void qtInputsItem::onRemoveValue()
{
  QToolButton* const minusButton = qobject_cast<QToolButton*>(
    QObject::sender());
  if(!minusButton || !this->Internals->ExtensibleMap.contains(minusButton))
    {
    return;
    }

  int gIdx = this->Internals->MinusButtonIndices.indexOf(minusButton);//minusButton->property("SubgroupIndex").toInt();
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!item || gIdx < 0 || gIdx >= static_cast<int>(item->numberOfValues()))
    {
    return;
    }

  QWidget* widget = this->Internals->ExtensibleMap.value(minusButton).second;
  if(this->Internals->ChildrenMap.value(widget).second)
    {
    delete this->Internals->ChildrenMap.value(widget).second;
    }
  if(this->Internals->ChildrenMap.value(widget).first)
    {
    delete this->Internals->ChildrenMap.value(widget).first;
    }
  delete widget;
  delete this->Internals->ExtensibleMap.value(minusButton).first;
  this->Internals->ExtensibleMap.remove(minusButton);
  this->Internals->MinusButtonIndices.removeOne(minusButton);
  delete minusButton;

  switch (item->type())
    {
    case smtk::attribute::Item::DOUBLE:
      {
      dynamic_pointer_cast<DoubleItem>(item)->removeValue(gIdx);
      break;
      }
    case smtk::attribute::Item::INT:
      {
      dynamic_pointer_cast<IntItem>(item)->removeValue(gIdx);
      break;
      }
    case smtk::attribute::Item::STRING:
      {
      dynamic_pointer_cast<StringItem>(item)->removeValue(gIdx);
     break;
      }
    default:
      //this->m_errorStatus << "Error: Unsupported Item Type: " <<
      // smtk::attribute::Item::type2String(item->type()) << "\n";
      break;
    }
  this->updateExtensibleState();
}

//----------------------------------------------------------------------------
void qtInputsItem::updateExtensibleState()
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!item || !item->isExtensible())
    {
    return;
    }
  bool maxReached = (item->maxNumberOfValues() > 0) &&
    (item->maxNumberOfValues() == item->numberOfValues());
  this->Internals->AddItemButton->setEnabled(!maxReached);

  bool minReached = (item->numberOfRequiredValues() > 0) &&
    (item->numberOfRequiredValues() == item->numberOfValues());
  foreach(QToolButton* tButton, this->Internals->ExtensibleMap.keys())
    {
    tButton->setEnabled(!minReached);
    }
}

//----------------------------------------------------------------------------
void qtInputsItem::clearChildWidgets()
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(this->getObject());
  if(!item)
    {
    return;
    }

  if(item->isExtensible())
    {
    //clear mapping
    foreach(QToolButton* tButton, this->Internals->ExtensibleMap.keys())
      {
// will delete later from this->Internals->ChildrenMap
//      delete this->Internals->ExtensibleMap.value(tButton).second;
      delete this->Internals->ExtensibleMap.value(tButton).first;
      delete tButton;
      }
    this->Internals->ExtensibleMap.clear();
    this->Internals->MinusButtonIndices.clear();
    }

  foreach(QWidget* widget, this->Internals->ChildrenMap.keys())
    {
    if(this->Internals->ChildrenMap.value(widget).second)
      {
      delete this->Internals->ChildrenMap.value(widget).second;
      }
    if(this->Internals->ChildrenMap.value(widget).first)
      {
      delete this->Internals->ChildrenMap.value(widget).first;
      }
    delete widget;
    }
  this->Internals->ChildrenMap.clear();
}
