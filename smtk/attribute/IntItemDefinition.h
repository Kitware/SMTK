//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME IntItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_IntItemDefinition_h
#define __smtk_attribute_IntItemDefinition_h

#include "smtk/attribute/ValueItemDefinitionTemplate.h"

namespace smtk
{
  namespace attribute
  {
    class SMTKCORE_EXPORT IntItemDefinition :
      public ValueItemDefinitionTemplate<int>
    {
    public:
      smtkTypeMacro(IntItemDefinition);
      static smtk::attribute::IntItemDefinitionPtr New(const std::string &myName)
      { return smtk::attribute::IntItemDefinitionPtr(new IntItemDefinition(myName));}

      virtual ~IntItemDefinition();
      virtual Item::Type type() const;
      virtual smtk::attribute::ItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::attribute::ItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;

      virtual smtk::attribute::ItemDefinitionPtr
        createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const;
    protected:
      IntItemDefinition(const std::string &myName);

    private:

    };
  }
}

#endif /* __smtk_attribute_DoubleItemDefinition_h */
