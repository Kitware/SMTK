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
#include "smtk/extension/qt/qtReferenceItem.h"
#include "smtk/extension/qt/qtReferenceItemComboBox.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"

namespace smtk
{
namespace extension
{

qtItem* qtComponentItem::createItemWidget(const qtAttributeItemInfo& info)
{
  // So we support this type of item?
  auto item = info.itemAs<smtk::attribute::ComponentItem>();
  if (item == nullptr)
  {
    return nullptr;
  }

  // If we are dealing with a non-extensible item with only 1 required value lets
  // use a simple combobox UI else we will use the more advance UI.
  auto itemDef = item->definitionAs<smtk::attribute::ReferenceItemDefinition>();
  if ((itemDef->numberOfRequiredValues() == 1) && !itemDef->isExtensible())
  {
    return new qtReferenceItemComboBox(info);
  }
  return new qtReferenceItem(info);
}
}
}
