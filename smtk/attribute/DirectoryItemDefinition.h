//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DirectoryItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_DirectoryItemDefinition_h
#define __smtk_attribute_DirectoryItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/FileSystemItemDefinition.h"

namespace smtk
{
  namespace attribute
  {
    class SMTKCORE_EXPORT DirectoryItemDefinition:
      public FileSystemItemDefinition
    {
    public:
      smtkTypeMacro(DirectoryItemDefinition);
      static smtk::attribute::DirectoryItemDefinitionPtr New(const std::string &myName)
      { return smtk::attribute::DirectoryItemDefinitionPtr(new DirectoryItemDefinition(myName));}

      virtual ~DirectoryItemDefinition();

      virtual Item::Type type() const;

      virtual smtk::attribute::ItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::attribute::ItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;

      virtual smtk::attribute::ItemDefinitionPtr
        createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const;
    protected:
      DirectoryItemDefinition(const std::string &myName);

    private:

    };
  }
}

#endif /* __smtk_attribute_DirectoryItemDefinition_h */
