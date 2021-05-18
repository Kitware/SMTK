//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtComponentItem_h
#define smtk_extension_qt_qtComponentItem_h

#include "smtk/extension/qt/qtReferenceItem.h"

#include "smtk/extension/qt/Exports.h"
namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtComponentItem : public qtReferenceItem
{
  Q_OBJECT

public:
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);
  qtComponentItem(const qtAttributeItemInfo& info);
  ~qtComponentItem() override;
};
} // namespace extension
} // namespace smtk
#endif
