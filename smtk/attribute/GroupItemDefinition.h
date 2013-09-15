/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME GroupItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_GroupItemDefinition_h
#define __smtk_attribute_GroupItemDefinition_h

#include "smtk/attribute/ItemDefinition.h"
#include <map>
#include <string>
#include <vector>

namespace smtk
{
  namespace attribute
  {
    class GroupItem;
    class SMTKCORE_EXPORT GroupItemDefinition :
      public ItemDefinition
    {
    public:
      static smtk::GroupItemDefinitionPtr New(const std::string &myName)
      { return smtk::GroupItemDefinitionPtr(new GroupItemDefinition(myName));}

      // This method is for wrapping code.  C++ developers should use smtk::dynamicCastPointer
      static smtk::GroupItemDefinitionPtr CastTo(const smtk::AttributeItemDefinitionPtr &p)
      {return smtk::dynamic_pointer_cast<GroupItemDefinition>(p);}

      virtual ~GroupItemDefinition();
      virtual Item::Type type() const;
      std::size_t numberOfItemDefinitions() const
      {return this->m_itemDefs.size();}
      smtk::AttributeItemDefinitionPtr itemDefinition(int ith) const
      {
        return (ith < 0) ? smtk::AttributeItemDefinitionPtr() :
          (ith >= this->m_itemDefs.size() ?
           smtk::AttributeItemDefinitionPtr() : this->m_itemDefs[ith]);
      }
      bool addItemDefinition(smtk::AttributeItemDefinitionPtr cdef);
      template<typename T>
        typename smtk::internal::shared_ptr_type<T>::SharedPointerType
        addItemDefinition(const std::string &name)
      {
        typedef smtk::internal::shared_ptr_type<T> SharedTypes;
        typename SharedTypes::SharedPointerType item;

        // First see if there is a item by the same name
        if (this->findItemPosition(name) < 0)
          {
          std::size_t n = this->m_itemDefs.size();
          item = SharedTypes::RawPointerType::New(name);
          this->m_itemDefs.push_back(item);
          this->m_itemDefPositions[name] = n;
          }
        return item;
      }

      int findItemPosition(const std::string &name) const;

      int numberOfRequiredGroups() const
      {return this->m_numberOfRequiredGroups;}
      void setNumberOfRequiredGroups(int gsize)
      {this->m_numberOfRequiredGroups = gsize;}
      bool hasSubGroupLabels() const
      {return this->m_labels.size();}

      void setSubGroupLabel(int element, const std::string &elabel);
      void setCommonSubGroupLabel(const std::string &elabel);
      bool usingCommonSubGroupLabel() const
      {return this->m_useCommonLabel;}
      std::string subGroupLabel(int element) const;

      virtual smtk::AttributeItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::AttributeItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;
      void buildGroup(smtk::attribute::GroupItem *group, int subGroupPosition) const;
      virtual void addCategory(const std::string &category);
      virtual void removeCategory(const std::string &category);

    protected:
      GroupItemDefinition(const std::string &myname);
      virtual void updateCategories();
      std::vector<smtk::AttributeItemDefinitionPtr> m_itemDefs;
      std::map<std::string, int> m_itemDefPositions;
      std::vector<std::string> m_labels;
      int m_numberOfRequiredGroups;
      bool m_useCommonLabel;
    private:
    };
//----------------------------------------------------------------------------
    inline int GroupItemDefinition::
    findItemPosition(const std::string &name) const
    {
      std::map<std::string, int>::const_iterator it;
      it = this->m_itemDefPositions.find(name);
      if (it == this->m_itemDefPositions.end())
        {
        return -1; // named item doesn't exist
        }
      return it->second;
    }
  }
}


#endif /* __smtk_attribute_GroupItemDefinition_h */
