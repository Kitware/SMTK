//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME MeshEntityItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_MeshEntityItemDefinition_h
#define __smtk_attribute_MeshEntityItemDefinition_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ItemDefinition.h"

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class SMTKCORE_EXPORT MeshEntityItemDefinition:
      public ItemDefinition
    {
    public:
      smtkTypeMacro(MeshEntityItemDefinition);
      static smtk::attribute::MeshEntityItemDefinitionPtr New(const std::string &myName)
      { return smtk::attribute::MeshEntityItemDefinitionPtr(new MeshEntityItemDefinition(myName));}

      virtual ~MeshEntityItemDefinition();

      virtual Item::Type type() const;
      bool isValueValid(const int &val) const;
      std::string refModelEntityName() const
      { return m_RefModelEntityDefName; }
      void setRefModelEntityName(const std::string& defName)
      { m_RefModelEntityDefName = defName; }

      virtual smtk::attribute::ItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::attribute::ItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;

      virtual smtk::attribute::ItemDefinitionPtr
        createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const;
    protected:
      MeshEntityItemDefinition(const std::string &myName);

      std::string m_RefModelEntityDefName;
    };
  }
}

#endif /* __smtk_attribute_MeshEntityItemDefinition_h */
