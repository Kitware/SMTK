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

#ifndef __smtk_attribute_DirectoryItem_h
#define __smtk_attribute_DirectoryItem_h

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
      smtkTypeMacro(DirectoryItem);
      virtual ~DirectoryItem();
      virtual Item::Type type() const;

    protected:
      DirectoryItem(Attribute *owningAttribute, int itemPosition);
      DirectoryItem(Item *owningItem, int position, int subGroupPosition);

    private:
    };
  }
}


#endif /* __smtk_attribute_DirectoryItem_h */
