//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME IntItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_IntItem_h
#define __smtk_attribute_IntItem_h

#include "smtk/attribute/ValueItemTemplate.h"
#include "smtk/CoreExports.h"

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class IntItemDefinition;
    class SMTKCORE_EXPORT IntItem :
      public ValueItemTemplate<int>
    {
      friend class IntItemDefinition;
    public:
      smtkTypeMacro(IntItem);
      virtual ~IntItem();
      virtual Item::Type type() const;
      virtual void copyFrom(const smtk::attribute::ItemPtr sourceItem,
                            smtk::attribute::Item::CopyInfo& info);
    protected:
      IntItem(Attribute *owningAttribute, int itemPosition);
      IntItem(Item *owningItem, int myPosition, int mySubGroupPosition);

    private:

    };
  }
}

#endif /* __smtk_attribute_IntItem_h */
