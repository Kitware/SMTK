//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtReferenceItem.h"
#include "smtk/extension/qt/qtReferenceItemData.h"

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtOverlay.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

using namespace smtk::extension;
using namespace smtk::attribute;

qtReferenceItemData::qtReferenceItemData()
{
}

qtReferenceItemData::~qtReferenceItemData()
{
}

qtReferenceItem::qtReferenceItem(smtk::attribute::ItemPtr item, QWidget* parent, qtBaseView* bview)
  : Superclass(item, parent, bview)
  , m_p(new qtReferenceItemData)
{
}

qtReferenceItem::~qtReferenceItem()
{
  delete m_p;
  m_p = nullptr;
}

void qtReferenceItem::selectionLinkToggled(bool linked)
{
  (void)linked;
}

void qtReferenceItem::createWidget()
{
}

void qtReferenceItem::updateUI()
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  // smtk::attribute::ValueItemPtr dataObj = this->valueItem();
  if (!dataObj || !this->passAdvancedCheck() ||
    (this->baseView() &&
      !this->baseView()->uiManager()->passItemCategoryCheck(dataObj->definition())))
  {
    return;
  }
}
