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
#include "smtk/extension/qt/qtReferenceItemData.h"

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/ComponentPhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/VisibilityContent.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"

#include "smtk/model/Resource.h"

namespace smtk
{
namespace extension
{

qtItem* qtComponentItem::createItemWidget(const AttributeItemInfo& info)
{
  // So we support this type of item?
  if (info.itemAs<smtk::attribute::ComponentItem>() == nullptr)
  {
    return nullptr;
  }
  return new qtComponentItem(info);
}

qtComponentItem::qtComponentItem(const AttributeItemInfo& info)
  : qtReferenceItem(info)
{
  this->createWidget();
}

qtComponentItem::~qtComponentItem()
{
}
}
}
