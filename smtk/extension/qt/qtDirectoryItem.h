//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtDirectoryItem - UI components for attribute DirectoryItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_extension_qtDirectoryItem_h
#define __smtk_extension_qtDirectoryItem_h

#include "smtk/extension/qt/qtFileItem.h"

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtDirectoryItem
{
public:
  static qtItem* createItemWidget(const AttributeItemInfo& info)
  {
    return qtFileItem::createItemWidget(info);
  }
};

}; // namespace extension
}; // namespace smtk

#endif
