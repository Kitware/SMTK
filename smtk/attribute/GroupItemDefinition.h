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
      static smtk::attribute::GroupItemDefinitionPtr New(const std::string &myName)
      { return smtk::attribute::GroupItemDefinitionPtr(new GroupItemDefinition(myName));}

      virtual ~GroupItemDefinition();
      virtual Item::Type type() const;
      std::size_t numberOfItemDefinitions() const
      {return this->m_itemDefs.size();}
      smtk::attribute::ItemDefinitionPtr itemDefinition(int ith) const
      {
        return (ith < 0) ? smtk::attribute::ItemDefinitionPtr() :
          (static_cast<unsigned int>(ith) >= this->m_itemDefs.size() ?
           smtk::attribute::ItemDefinitionPtr() : this->m_itemDefs[ith]);
      }
      bool addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef);
      template<typename T>
        typename smtk::internal::shared_ptr_type<T>::SharedPointerType
        addItemDefinition(const std::string &inName)
      {
        typedef smtk::internal::shared_ptr_type<T> SharedTypes;
        typename SharedTypes::SharedPointerType item;

        // First see if there is a item by the same name
        if (this->findItemPosition(inName) < 0)
          {
          std::size_t n = this->m_itemDefs.size();
          item = SharedTypes::RawPointerType::New(inName);
          this->m_itemDefs.push_back(item);
          this->m_itemDefPositions[inName] = static_cast<int>(n);
          }
        return item;
      }

      int findItemPosition(const std::string &name) const;

      size_t numberOfRequiredGroups() const
      {return this->m_numberOfRequiredGroups;}
      void setNumberOfRequiredGroups(size_t gsize)
      {this->m_numberOfRequiredGroups = gsize;}
      bool hasSubGroupLabels() const
      {return !this->m_labels.empty();}

      void setSubGroupLabel(size_t element, const std::string &elabel);
      void setCommonSubGroupLabel(const std::string &elabel);
      bool usingCommonSubGroupLabel() const
      {return this->m_useCommonLabel;}
      std::string subGroupLabel(size_t element) const;

      virtual smtk::attribute::ItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::attribute::ItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;
      void buildGroup(smtk::attribute::GroupItem *group, int subGroupPosition) const;
      virtual void addCategory(const std::string &category);
      virtual void removeCategory(const std::string &category);

    protected:
      GroupItemDefinition(const std::string &myname);
      virtual void updateCategories();
      std::vector<smtk::attribute::ItemDefinitionPtr> m_itemDefs;
      std::map<std::string, int> m_itemDefPositions;
      std::vector<std::string> m_labels;
      size_t m_numberOfRequiredGroups;
      bool m_useCommonLabel;
    private:
    };
//----------------------------------------------------------------------------
    inline int GroupItemDefinition::
    findItemPosition(const std::string &inName) const
    {
      std::map<std::string, int>::const_iterator it;
      it = this->m_itemDefPositions.find(inName);
      if (it == this->m_itemDefPositions.end())
        {
        return -1; // named item doesn't exist
        }
      return it->second;
    }
  }
}


#endif /* __smtk_attribute_GroupItemDefinition_h */
