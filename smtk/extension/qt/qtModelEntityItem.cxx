//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtModelEntityItem.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtCheckItemComboBox.h"

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
#include <QStandardItemModel>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtModelEntityItemInternals
{
public:

  QPointer<QGridLayout> EntryLayout;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;

  // for extensible items
  QMap<QToolButton*, QPair<QLayout*, QWidget*> > ExtensibleMap;
  QList<QToolButton*> MinusButtonIndices;
  QPointer<QToolButton> AddItemButton;

};

//----------------------------------------------------------------------------
qtModelEntityItem::qtModelEntityItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p, qtBaseView* bview,
   Qt::Orientation enVectorItemOrient) : qtItem(dataObj, p, bview)
{
  this->Internals = new qtModelEntityItemInternals;
  this->IsLeafItem = true;
  this->Internals->VectorItemOrient = enVectorItemOrient;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtModelEntityItem::~qtModelEntityItem()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtModelEntityItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
}

//----------------------------------------------------------------------------
void qtModelEntityItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if(!dataObj || !this->passAdvancedCheck() || (this->baseView() &&
    !this->baseView()->uiManager()->passItemCategoryCheck(
      dataObj->definition())))
    {
    return;
    }

  this->clearChildWidgets();
  this->updateItemData();
}

//----------------------------------------------------------------------------
void qtModelEntityItem::updateItemData()
{
  this->updateUI();
  this->qtItem::updateItemData();
}

//----------------------------------------------------------------------------
void qtModelEntityItem::addEntityAssociationWidget()
{
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
  if(!item)
    {
    return;
    }

  int n = static_cast<int>(item->numberOfValues());
  if (!n)
    {
    return;
    }

  qtModelEntityItemCombo* editBox = new qtModelEntityItemCombo(
    this->getObject(), this->Widget, "Entities");
  editBox->setToolTip("Associate model entities");
  editBox->setModel(new QStandardItemModel());
  editBox->setItemDelegate(
    new qtCheckableComboItemDelegate(editBox));

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QBoxLayout* editorLayout = new QHBoxLayout;
  editorLayout->setMargin(0);
  editorLayout->setSpacing(3);
/*
  const ModelEntityItemDefinition *itemDef =
    dynamic_cast<const ModelEntityItemDefinition*>(item->definition().get());
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

  if(n!=1 && itemDef->hasValueLabels())
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
*/

  editorLayout->addWidget(editBox);
  this->Internals->EntryLayout->addLayout(editorLayout, 0, 1);
  editBox->init();

/*
  // always going vertical for extensible items
  if(this->Internals->VectorItemOrient == Qt::Vertical ||
     item->isExtensible())
    {
    int row = i;
    // The "Add New Value" button is in first row, so take that into account
    row = item->isExtensible() ? row+1 : row;
    this->Internals->EntryLayout->addLayout(editorLayout, row, 1);
    }
  else // going horizontal
    {
    this->Internals->EntryLayout->addLayout(editorLayout, 0, i+1);
    }

  //this->updateExtensibleState();
*/
}

//----------------------------------------------------------------------------
void qtModelEntityItem::loadAssociatedEntities()
{
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
  if(!item)
    {
    return;
    }
/*
  int n = static_cast<int>(item->numberOfValues());
  if (!n && !item->isExtensible())
    {
    return;
    }
*/
  this->addEntityAssociationWidget();
/*
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
    this->addEntityAssociationWidget(i);
    }
*/
}

//----------------------------------------------------------------------------
void qtModelEntityItem::updateUI()
{
  //smtk::attribute::ItemPtr dataObj = this->getObject();
  smtk::attribute::ModelEntityItemPtr dataObj =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
  if(!dataObj || !this->passAdvancedCheck() || (this->baseView() &&
    !this->baseView()->uiManager()->passItemCategoryCheck(
      dataObj->definition())))
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
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(dataObj);
  const ModelEntityItemDefinition *itemDef =
    dynamic_cast<const ModelEntityItemDefinition*>(dataObj->definition().get());

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
  if(this->baseView())
    {
    label->setFixedWidth(this->baseView()->fixedLabelWidth() - padding);
    }
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

//  qtOverlayFilter *filter = new qtOverlayFilter(this);
//  label->installEventFilter(filter);

  // add in BriefDescription as tooltip if available
  const std::string strBriefDescription = itemDef->briefDescription();
  if(!strBriefDescription.empty())
    {
    label->setToolTip(strBriefDescription.c_str());
    }

  if(itemDef->advanceLevel() && this->baseView())
    {
    label->setFont(this->baseView()->uiManager()->advancedFont());
    }
  labelLayout->addWidget(label);
  this->Internals->theLabel = label;

  this->loadAssociatedEntities();

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
void qtModelEntityItem::setOutputOptional(int state)
{
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
  if(!item)
    {
    return;
    }
  bool enable = state ? true : false;
/*
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
*/
//  this->Internals->EntryFrame->setEnabled(enable);
  if(enable != this->getObject()->isEnabled())
    {
    this->getObject()->setIsEnabled(enable);
    if(this->baseView())
      this->baseView()->valueChanged(this->getObject());
    }
}

//----------------------------------------------------------------------------
void qtModelEntityItem::onAddNewValue()
{
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
  if(!item)
    {
    return;
    }
  if(item->setNumberOfValues(item->numberOfValues() + 1))
    {
//    QBoxLayout* entryLayout = qobject_cast<QBoxLayout*>(
//      this->Internals->EntryFrame->layout());
//    this->addEntityAssociationWidget(static_cast<int>(item->numberOfValues()) - 1);
    }
}

//----------------------------------------------------------------------------
void qtModelEntityItem::onRemoveValue()
{
  QToolButton* const minusButton = qobject_cast<QToolButton*>(
    QObject::sender());
  if(!minusButton || !this->Internals->ExtensibleMap.contains(minusButton))
    {
    return;
    }

  int gIdx = this->Internals->MinusButtonIndices.indexOf(minusButton);
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
  if(!item || gIdx < 0 || gIdx >= static_cast<int>(item->numberOfValues()))
    {
    return;
    }

  QWidget* childwidget = this->Internals->ExtensibleMap.value(minusButton).second;
  delete childwidget;
  delete this->Internals->ExtensibleMap.value(minusButton).first;
  this->Internals->ExtensibleMap.remove(minusButton);
  this->Internals->MinusButtonIndices.removeOne(minusButton);
  delete minusButton;
/*
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
*/
}

//----------------------------------------------------------------------------
void qtModelEntityItem::updateExtensibleState()
{
/*
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
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
*/
}

//----------------------------------------------------------------------------
void qtModelEntityItem::clearChildWidgets()
{
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
  if(!item)
    {
    return;
    }
/*
  if(item->isExtensible())
    {
    //clear mapping
    foreach(QToolButton* tButton, this->Internals->ExtensibleMap.keys())
      {
      delete this->Internals->ExtensibleMap.value(tButton).first;
      delete tButton;
      }
    this->Internals->ExtensibleMap.clear();
    this->Internals->MinusButtonIndices.clear();
    }
*/
}
