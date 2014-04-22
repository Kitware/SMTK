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
#include "smtk/view/Root.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtInputsItemInternals
{
public:

  QPointer<QFrame> EntryFrame;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;

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

  this->clearChildItems();
  this->updateUI();
}
//----------------------------------------------------------------------------
void qtInputsItem::addInputEditor(QBoxLayout* entrylayout, int i)
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

  QWidget* editBox = this->baseView()->uiManager()->createInputWidget(
    item, i, this->Widget, this->baseView());
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
    QToolButton* minusButton = new QToolButton(this->Internals->EntryFrame);
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
  // there could be conditional children, so we need another layout
  // so that the combobox will stay TOP-left when there are multiple
  // combo boxes.
  if(item->isDiscrete())
    {
    QVBoxLayout* childLayout = new QVBoxLayout;
    childLayout->setMargin(0);
    childLayout->setSpacing(6);

    childLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    childLayout->addLayout(editorLayout);
    entrylayout->addLayout(childLayout);
    }
  else
    {
    entrylayout->addLayout(editorLayout);
    }
  this->updateExtensibleState();
}

//----------------------------------------------------------------------------
void qtInputsItem::loadInputValues(
  QBoxLayout* /*labellayout*/, QBoxLayout* entrylayout)
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
      this->Internals->AddItemButton = new QToolButton(this->Internals->EntryFrame);
      QString iconName(":/icons/attribute/plus.png");
      this->Internals->AddItemButton->setToolTip("Add new value");
      this->Internals->AddItemButton->setFixedSize(QSize(12, 12));
      this->Internals->AddItemButton->setIcon(QIcon(iconName));
      this->Internals->AddItemButton->setSizePolicy(sizeFixedPolicy);
      connect(this->Internals->AddItemButton, SIGNAL(clicked()),
        this, SLOT(onAddNewValue()));
      entrylayout->addWidget(this->Internals->AddItemButton);
      }
    //clear mapping
    foreach(QToolButton* tButton, this->Internals->ExtensibleMap.keys())
      {
      delete this->Internals->ExtensibleMap.value(tButton).second;
      delete this->Internals->ExtensibleMap.value(tButton).first;
      delete tButton;
      }
    this->Internals->ExtensibleMap.clear();
    this->Internals->MinusButtonIndices.clear();
    }

  for(int i = 0; i < n; i++)
    {
    this->addInputEditor(entrylayout, i);
    }
}

//----------------------------------------------------------------------------
void qtInputsItem::updateUI()
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if(!dataObj || !this->passAdvancedCheck() ||
    !this->baseView()->uiManager()->passItemCategoryCheck(
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
  QGridLayout* layout = new QGridLayout(this->Widget);
  layout->setMargin(0);
  layout->setSpacing(0);
  layout->setAlignment( Qt::AlignLeft | Qt::AlignTop );

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  this->Internals->EntryFrame = new QFrame(this->parentWidget());
  this->Internals->EntryFrame->setObjectName("CheckAndEntryInputFrame");
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
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if(dataObj->isOptional())
    {
    QCheckBox* optionalCheck = new QCheckBox(this->parentWidget());
    optionalCheck->setChecked(dataObj->isEnabled());
    optionalCheck->setText(" ");
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = optionalCheck->iconSize().width() + 6; // 6 is for layout spacing
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)),
      this, SLOT(setOutputOptional(int)));
    this->Internals->EntryFrame->setEnabled(dataObj->isEnabled());
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
  smtk::view::RootPtr rs = this->baseView()->uiManager()->attManager()->rootView();
  label->setFixedWidth(rs->maxValueLabelLength() - padding);
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

  this->loadInputValues(labelLayout, entryLayout);

  entryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // we need this layout so that for items with conditionan children,
  // the label will line up at Top-left against the chilren's widgets.
  QVBoxLayout* vTLlayout = new QVBoxLayout;
  vTLlayout->setMargin(0);
  vTLlayout->setSpacing(0);
  vTLlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  vTLlayout->addLayout(labelLayout);
  layout->addLayout(vTLlayout, 0, 0);
  layout->addWidget(this->Internals->EntryFrame, 0, 1);
  if(this->parentWidget() && this->parentWidget()->layout())
    {
    this->parentWidget()->layout()->addWidget(this->Widget);
    }
}

//----------------------------------------------------------------------------
void qtInputsItem::setOutputOptional(int state)
{
  bool enable = state ? true : false;
  this->Internals->EntryFrame->setEnabled(enable);
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
    QBoxLayout* entryLayout = qobject_cast<QBoxLayout*>(
      this->Internals->EntryFrame->layout());
    this->addInputEditor(entryLayout, static_cast<int>(item->numberOfValues()) - 1);
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

  delete this->Internals->ExtensibleMap.value(minusButton).second;
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
