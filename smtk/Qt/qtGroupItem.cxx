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

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"

#include <QGroupBox>
#include <QVBoxLayout>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtGroupItemInternals
{
public:

};

//----------------------------------------------------------------------------
qtGroupItem::qtGroupItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p) : qtItem(dataObj, p)
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
  smtk::attribute::GroupItemPtr item =dynamicCastPointer<GroupItem>(this->getObject());
  if(!item || !item->numberOfGroups())
    {
    return;
    }

  QGroupBox* groupBox = new QGroupBox(item->name().c_str(),
    this->parentWidget());
  //   groupBox->setCheckable(true);
  //   groupBox->setChecked(true);
  this->Widget = groupBox;
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  if(this->parentWidget())
    {
    this->parentWidget()->layout()->setAlignment(Qt::AlignTop);
    this->parentWidget()->layout()->addWidget(this->Widget);
    }

  this->updateItemData();
}


//----------------------------------------------------------------------------
void qtGroupItem::updateItemData()
{
  smtk::attribute::GroupItemPtr item =dynamicCastPointer<GroupItem>(this->getObject());
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
      qtItem* childItem = qtAttribute::createItem(item->item(i,j), this->Widget);
      if(childItem)
        {
        this->Widget->layout()->addWidget(childItem->widget());
        }
      }
    }
}
