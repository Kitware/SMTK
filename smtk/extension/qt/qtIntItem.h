//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtIntItem - UI components for attribute IntItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_extension_qtIntItem_h
#define __smtk_extension_qtIntItem_h

#include "smtk/extension/qt/qtInputsItem.h"

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtIntItem
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
