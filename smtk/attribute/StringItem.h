//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME StringItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_StringItem_h
#define __smtk_attribute_StringItem_h

#include "smtk/attribute/ValueItemTemplate.h"
#include "smtk/CoreExports.h"

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class StringItemDefinition;
    class SMTKCORE_EXPORT StringItem :
      public ValueItemTemplate<std::string>
    {
      friend class StringItemDefinition;
    public:
      smtkTypeMacro(StringItem);
      virtual ~StringItem();
      virtual Item::Type type() const;
      virtual void copyFrom(const smtk::attribute::ItemPtr sourceItem,
                            smtk::attribute::Item::CopyInfo& info);
    protected:
      StringItem(Attribute *owningAttribute, int itemPosition);
      StringItem(Item *owningItem, int myPosition, int mySubGroupPosition);

    private:

    };
  }
}

#endif /* __smtk_attribute_StringItem_h */
