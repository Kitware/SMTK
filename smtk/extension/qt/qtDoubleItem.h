//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtDoubleItem - UI components for attribute DoubleItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef smtk_extension_qtDoubleItem_h
#define smtk_extension_qtDoubleItem_h

#include "smtk/extension/qt/qtInputsItem.h"

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtDoubleItem
{
public:
  static qtItem* createItemWidget(const qtAttributeItemInfo& info)
  {
    return qtInputsItem::createItemWidget(info);
  }
};

}; // namespace extension
}; // namespace smtk

#endif
