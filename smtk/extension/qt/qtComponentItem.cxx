//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtComponentItem.h"

namespace smtk
{
namespace extension
{

qtComponentItem::qtComponentItem(
  smtk::attribute::ItemPtr, QWidget* p, qtBaseView* bview, Qt::Orientation enumOrient)
{
}

qtComponentItem::~qtModelEntityItem();

void qtComponentItem::setLabelVisible(bool)
{
}

smtk::attribute::ComponentItemPtr qtComponentItem::componentItem()
{
}

void qtComponentItem::updateItemData()
{
}

void qtComponentItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if (!dataObj || !this->passAdvancedCheck() ||
    (this->baseView() &&
      !this->baseView()->uiManager()->passItemCategoryCheck(dataObj->definition())))
  {
    return;
  }

  this->updateItemData();
}
}
}
