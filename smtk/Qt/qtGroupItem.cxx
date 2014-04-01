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

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtGroupItemInternals
{
public:
  QPointer<QFrame> ChildrensFrame;
};

//----------------------------------------------------------------------------
qtGroupItem::qtGroupItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p, qtBaseView* bview) :
   qtItem(dataObj, p, bview)
{
  this->Internals = new qtGroupItemInternals;
  this->IsLeafItem = true;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtGroupItem::~qtGroupItem()
{
  delete this->Internals;
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
  if(!item || !item->numberOfGroups())
    {
    return;
    }

  QGroupBox* groupBox = new QGroupBox(item->label().c_str(),
    this->parentWidget());
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
    this->baseView()->valueChanged(this);
    }
}

//----------------------------------------------------------------------------
void qtGroupItem::updateItemData()
{
  smtk::attribute::GroupItemPtr item =dynamic_pointer_cast<GroupItem>(this->getObject());
  if(!item || !item->numberOfGroups())
    {
    return;
    }

  std::size_t i, n = item->numberOfGroups();
  std::size_t j, m = item->numberOfItemsPerGroup();   
  for(i = 0; i < n; i++)
    {
    for (j = 0; j < m; j++)
      {
      qtItem* childItem = qtAttribute::createItem(item->item(static_cast<int>(i),
        static_cast<int>(j)), this->Widget, this->baseView());
      if(childItem)
        {
        this->Internals->ChildrensFrame->layout()->addWidget(childItem->widget());
        }
      }
    }
}
