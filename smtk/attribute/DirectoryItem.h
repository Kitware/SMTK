//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DirectoryItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_DirectoryItem_h
#define smtk_attribute_DirectoryItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/FileSystemItem.h"
#include <string>
#include <vector>

namespace smtk
{
namespace attribute
{
class DirectoryItemDefinition;
class SMTKCORE_EXPORT DirectoryItem : public FileSystemItem
{
  friend class DirectoryItemDefinition;

public:
  smtkTypeMacro(smtk::attribute::DirectoryItem);
  smtkSuperclassMacro(smtk::attribute::FileSystemItem);
  ~DirectoryItem() override;
  Item::Type type() const override;

protected:
  DirectoryItem(Attribute* owningAttribute, int itemPosition);
  DirectoryItem(Item* owningItem, int position, int subGroupPosition);

private:
};

} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_DirectoryItem_h */
