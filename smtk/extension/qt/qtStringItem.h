//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtStringItem - UI components for attribute StringItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef smtk_extension_qtStringItem_h
#define smtk_extension_qtStringItem_h

#include "smtk/extension/qt/qtInputsItem.h"

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtStringItem
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
