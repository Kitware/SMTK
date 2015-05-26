//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME VoidItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_VoidItem_h
#define __smtk_attribute_VoidItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <vector>

namespace smtk
{
  namespace attribute
  {
    class VoidItemDefinition;
    class SMTKCORE_EXPORT VoidItem : public Item
    {
    friend class VoidItemDefinition;
    public:
      smtkTypeMacro(VoidItem);
      virtual ~VoidItem();
      virtual Item::Type type() const;

    protected:
      VoidItem(Attribute *owningAttribute, int itemPosition);
      VoidItem(Item *owningItem, int myPosition, int mySubGroupPosition);
      virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def);
    private:
    };
  }
}


#endif /* __smtk_attribute_VoidItem_h */
