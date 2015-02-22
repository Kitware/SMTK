//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME MeshEntityItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_MeshEntityItem_h
#define __smtk_attribute_MeshEntityItem_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <string>
#include <set>

namespace smtk
{
  namespace attribute
  {
    class MeshEntityItemDefinition;
    class SMTKCORE_EXPORT MeshEntityItem : public Item
    {
    friend class MeshEntityItemDefinition;
    public:
      smtkTypeMacro(MeshEntityItem);
      virtual ~MeshEntityItem();
      virtual Item::Type type() const;

      void setValues(const std::set<int>&);
      void insertValues(const std::set<int>&);
      void removeValues(const std::set<int>&);

      std::size_t numberOfValues() const
      {return this->m_values.size();}
      int value(std::size_t element=0) const;
      bool insertValue(const int &val);
      bool removeValue(const int &val);
      virtual void reset();
      virtual std::string valueAsString() const
      {return this->valueAsString(0);}
      virtual std::string valueAsString(std::size_t element) const;
      virtual void copyFrom(const smtk::attribute::ItemPtr sourceItem,
                            smtk::attribute::Item::CopyInfo& info);

      std::set<int>::const_iterator begin() const;
      std::set<int>::const_iterator end() const;

    protected:
      MeshEntityItem(Attribute *owningAttribute, int itemPosition);
      MeshEntityItem(Item *owningItem, int position, int subGroupPosition);
      virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef);
      std::set<int>m_values;
    private:
    };
  }
}


#endif /* __smtk_attribute_MeshEntityItem_h */
