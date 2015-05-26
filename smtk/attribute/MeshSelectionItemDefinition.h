//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME MeshSelectionItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_MeshSelectionItemDefinition_h
#define __smtk_attribute_MeshSelectionItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ItemDefinition.h"
#include "smtk/model/EntityTypeBits.h"

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class SMTKCORE_EXPORT MeshSelectionItemDefinition:
      public ItemDefinition
    {
    public:
      smtkTypeMacro(MeshSelectionItemDefinition);
      static smtk::attribute::MeshSelectionItemDefinitionPtr New(const std::string &myName)
      { return smtk::attribute::MeshSelectionItemDefinitionPtr(new MeshSelectionItemDefinition(myName));}

      virtual ~MeshSelectionItemDefinition();

      virtual Item::Type type() const;
      smtk::model::BitFlags membershipMask() const;
      void setMembershipMask(smtk::model::BitFlags entMask);
      std::string refModelEntityName() const
        { return m_RefModelEntityDefName; }
      void setRefModelEntityName(const std::string& defName)
        { m_RefModelEntityDefName = defName; }
      void setModifyMode(const std::string& mode)
        { this->m_modifyMode = mode; }
      std::string modifyMode() const
        {return this->m_modifyMode;}

      bool isValueValid(const int &val) const;

      virtual smtk::attribute::ItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::attribute::ItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;

      virtual smtk::attribute::ItemDefinitionPtr
        createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const;
    protected:
      MeshSelectionItemDefinition(const std::string &myName);

      smtk::model::BitFlags m_membershipMask;
      std::string m_RefModelEntityDefName;
      std::string m_modifyMode;
    };
  }
}

#endif /* __smtk_attribute_MeshSelectionItemDefinition_h */
