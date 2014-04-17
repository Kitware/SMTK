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

#include "smtk/Qt/qtGroupItem.h"

#include "smtk/Qt/qtUIManager.h"
#include "smtk/Qt/qtAttribute.h"
#include "smtk/Qt/qtBaseView.h"

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QPointer>
#include <QMap>
#include <QToolButton>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtGroupItemInternals
{
public:
  QPointer<QFrame> ChildrensFrame;
  Qt::Orientation VectorItemOrient;
  QMap<QToolButton*, QPair<QFrame*, QList<qtItem* > > > ExtensibleMap;
  QList<QToolButton*> MinusButtonIndices;
  QPointer<QToolButton> AddItemButton;
};

//----------------------------------------------------------------------------
qtGroupItem::qtGroupItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p, qtBaseView* bview,
  Qt::Orientation enVectorItemOrient) :
   qtItem(dataObj, p, bview)
{
  this->Internals = new qtGroupItemInternals;
  this->IsLeafItem = true;
  this->Internals->VectorItemOrient = enVectorItemOrient;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtGroupItem::~qtGroupItem()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtGroupItem::setLabelVisible(bool visible)
{
  if(!this->getObject())
    {
    return;
    }
  smtk::attribute::GroupItemPtr item =dynamic_pointer_cast<GroupItem>(this->getObject());
  if(!item || !item->numberOfGroups())
    {
    return;
    }

  QGroupBox* groupBox = qobject_cast<QGroupBox*>(this->Widget);
  groupBox->setTitle(visible ?
    item->label().c_str() : "");
}

//----------------------------------------------------------------------------
void qtGroupItem::createWidget()
{
  if(!this->getObject())
    {
    return;
    }
  this->clearChildItems();
  smtk::attribute::GroupItemPtr item =dynamic_pointer_cast<GroupItem>(this->getObject());
  if(!item || (!item->numberOfGroups() && !item->isExtensible()))
    {
    return;
    }

  QString title = item->label().empty() ? item->name().c_str() : item->label().c_str();
  QGroupBox* groupBox = new QGroupBox(title, this->parentWidget());
  this->Widget = groupBox;
  // Instantiate a layout for the widget, but do *not* assign it to a variable.
  // because that would cause a compiler warning, since the layout is not
  // explicitly referenced anywhere in this scope. (There is no memory
  // leak because the layout instance is parented by the widget.)
  new QVBoxLayout(this->Widget);
  this->Widget->layout()->setMargin(0);
  this->Internals->ChildrensFrame = new QFrame(groupBox);
  new QVBoxLayout(this->Internals->ChildrensFrame);

  this->Widget->layout()->addWidget(this->Internals->ChildrensFrame);

  if(this->parentWidget())
    {
    this->parentWidget()->layout()->setAlignment(Qt::AlignTop);
    this->parentWidget()->layout()->addWidget(this->Widget);
    }
  this->updateItemData();

  // If the group is optional, we need a checkbox
  if(item->isOptional())
    {
    groupBox->setCheckable(true);
    groupBox->setChecked(item->isEnabled());
    this->Internals->ChildrensFrame->setEnabled(item->isEnabled());
    connect(groupBox, SIGNAL(toggled(bool)),
            this, SLOT(setEnabledState(bool)));
    }
}

//----------------------------------------------------------------------------
void qtGroupItem::setEnabledState(bool checked)
{
  this->Internals->ChildrensFrame->setEnabled(checked);
  if(!this->getObject())
    {
    return;
    }

  if(checked != this->getObject()->isEnabled())
    {
    this->getObject()->setIsEnabled(checked);
    this->baseView()->valueChanged(this->getObject());
    }
}

//----------------------------------------------------------------------------
void qtGroupItem::updateItemData()
{
  smtk::attribute::GroupItemPtr item =dynamic_pointer_cast<GroupItem>(this->getObject());
  if(!item || (!item->numberOfGroups() && !item->isExtensible()))
    {
    return;
    }
  if(item->isExtensible())
    {
    //clear mapping
    foreach(QToolButton* tButton, this->Internals->ExtensibleMap.keys())
      {
      foreach(qtItem* qi, this->Internals->ExtensibleMap.value(tButton).second)
        {
        delete qi->widget();
        delete qi;
        }
      delete this->Internals->ExtensibleMap.value(tButton).first;
      delete tButton;
      }
    this->Internals->ExtensibleMap.clear();
    this->Internals->MinusButtonIndices.clear();
    // The new item button
    if(!this->Internals->AddItemButton)
      {
      this->Internals->AddItemButton = new QToolButton(this->Internals->ChildrensFrame);
      this->Internals->AddItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
      QString iconName(":/icons/attribute/plus.png");
      this->Internals->AddItemButton->setText("Add Sub Group");
      this->Internals->AddItemButton->setIcon(QIcon(iconName));
      this->Internals->AddItemButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      connect(this->Internals->AddItemButton, SIGNAL(clicked()),
        this, SLOT(onAddSubGroup()));
      this->Internals->ChildrensFrame->layout()->addWidget(
        this->Internals->AddItemButton);
      }
    this->Widget->layout()->setSpacing(3);
    }

  std::size_t i, n = item->numberOfGroups();
  for(i = 0; i < n; i++)
    {
    this->addSubGroup(static_cast<int>(i));
    }
}

//----------------------------------------------------------------------------
void qtGroupItem::onAddSubGroup()
{
  smtk::attribute::GroupItemPtr item =dynamic_pointer_cast<GroupItem>(this->getObject());
  if(!item || (!item->numberOfGroups() && !item->isExtensible()))
    {
    return;
    }
  if(item->appendGroup())
    {
    this->addSubGroup(static_cast<int>(item->numberOfGroups()) - 1);
    }
}

//----------------------------------------------------------------------------
void qtGroupItem::addSubGroup(int i)
{
  smtk::attribute::GroupItemPtr item =dynamic_pointer_cast<GroupItem>(this->getObject());
  if(!item || (!item->numberOfGroups() && !item->isExtensible()))
    {
    return;
    }

  std::size_t j, m = item->numberOfItemsPerGroup();
  QBoxLayout* frameLayout = qobject_cast<QBoxLayout*>(
    this->Internals->ChildrensFrame->layout());
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QBoxLayout* subGrouplayout = new QVBoxLayout();
  subGrouplayout->setMargin(0);
  QList<qtItem*> itemList;
  for (j = 0; j < m; j++)
    {
    qtItem* childItem = qtAttribute::createItem(item->item(i,
      static_cast<int>(j)), this->Widget, this->baseView(), this->Internals->VectorItemOrient);
    if(childItem)
      {
      subGrouplayout->addWidget(childItem->widget());
      itemList.push_back(childItem);
      }
    }

  if(item->isExtensible())
    {
    QBoxLayout* layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(3);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QToolButton* minusButton = new QToolButton(this->Internals->ChildrensFrame);
    QString iconName(":/icons/attribute/minus.png");
    minusButton->setFixedSize(QSize(16, 16));
    minusButton->setIcon(QIcon(iconName));
    minusButton->setSizePolicy(sizeFixedPolicy);
    minusButton->setToolTip("Remove sub group");
    //QVariant vdata(static_cast<int>(i));
    //minusButton->setProperty("SubgroupIndex", vdata);
    connect(minusButton, SIGNAL(clicked()),
      this, SLOT(onRemoveSubGroup()));
    layout->addWidget(minusButton);
    QFrame* lineFrame = new QFrame();
    lineFrame->setFrameShape(QFrame::Panel);
    lineFrame->setFrameShadow(QFrame::Sunken);
    lineFrame->setLayout(subGrouplayout);
    layout->addWidget(lineFrame);
    //layout->addLayout(subGrouplayout);
    frameLayout->addLayout(layout);
    // frameLayout->addWidget(lineFrame);
    QPair <QFrame*, QList<qtItem*> > pair;
    pair.first = lineFrame;
    pair.second = itemList;
    this->Internals->ExtensibleMap[minusButton] = pair;
    this->Internals->MinusButtonIndices.push_back(minusButton);
    }
  else
    {
    frameLayout->addLayout(subGrouplayout);
    }
}
//----------------------------------------------------------------------------
void qtGroupItem::onRemoveSubGroup()
{
  QToolButton* const minusButton = qobject_cast<QToolButton*>(
    QObject::sender());
  if(!minusButton || !this->Internals->ExtensibleMap.contains(minusButton))
    {
    return;
    }

  int gIdx = this->Internals->MinusButtonIndices.indexOf(minusButton);//minusButton->property("SubgroupIndex").toInt();
  smtk::attribute::GroupItemPtr item =dynamic_pointer_cast<GroupItem>(this->getObject());
  if(!item || gIdx < 0 || gIdx >= static_cast<int>(item->numberOfGroups()))
    {
    return;
    }

  foreach(qtItem* qi, this->Internals->ExtensibleMap.value(minusButton).second)
    {
    delete qi->widget();
    delete qi;
    }
  delete this->Internals->ExtensibleMap.value(minusButton).first;
  this->Internals->ExtensibleMap.remove(minusButton);
  this->Internals->MinusButtonIndices.removeOne(minusButton);
  delete minusButton;

  item->removeGroup(gIdx);
}
