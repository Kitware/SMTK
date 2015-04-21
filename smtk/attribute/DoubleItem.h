//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DoubleItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_DoubleItem_h
#define __smtk_attribute_DoubleItem_h

#include "smtk/attribute/ValueItemTemplate.h"
#include "smtk/CoreExports.h"

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class DoubleItemDefinition;
    class SMTKCORE_EXPORT DoubleItem :
      public ValueItemTemplate<double>
    {
      friend class DoubleItemDefinition;
    public:
      smtkTypeMacro(DoubleItem);
      virtual ~DoubleItem();
      virtual Item::Type type() const;
      virtual void copyFrom(const smtk::attribute::ItemPtr sourceItem,
                            smtk::attribute::Item::CopyInfo& info);
    protected:
      DoubleItem(Attribute *owningAttribute, int itemPosition);
      DoubleItem(Item *owningItem, int myPosition, int mySubGroupPosition);

    private:

    };
  }
}

#endif /* __smtk_attribute_DoubleItem_h */
