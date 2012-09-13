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

#ifndef __slctk_attribute_GroupItemDefinition_h
#define __slctk_attribute_GroupItemDefinition_h

#include "attribute/ItemDefinition.h"
#include <map>
#include <string>
#include <vector>

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT GroupItemDefinition :
      public ItemDefinition
    {
    public:
      GroupItemDefinition(const std::string &myname);
      virtual ~GroupItemDefinition();
      virtual Item::Type type() const;
      std::size_t numberOfItemDefinitions() const
      {return this->m_itemDefs.size();}
      slctk::AttributeItemDefinitionPtr itemDefinition(int ith) const
      {
        return (ith < 0) ? slctk::AttributeItemDefinitionPtr() : 
          (ith >= this->m_itemDefs.size() ? 
           slctk::AttributeItemDefinitionPtr() : this->m_itemDefs[ith]);
      }
      bool addItemDefinition(slctk::AttributeItemDefinitionPtr cdef);
      template<typename T>
        typename slctk::internal::shared_ptr_type<T>::SharedPointerType
        addItemDefinition(const std::string &name)
      {
        typedef slctk::internal::shared_ptr_type<T> SharedTypes;
        typename SharedTypes::SharedPointerType 
          item(new typename SharedTypes::RawPointerType(name));
        this->m_itemDefs.push_back(item);
        return item;
      }

      int findItemPosition(const std::string &name) const;

      int numberOfGroups() const
      {return this->m_numberOfGroups;}
      void setNumberOfGroups(int gsize)
      {this->m_numberOfGroups = gsize;}
      bool hasSubGroupLabels() const
      {return this->m_labels.size();}

      void setSubGroupLabel(int element, const std::string &elabel);
      void setCommonSubGroupLabel(const std::string &elabel);
      bool usingCommonSubGroupLabel() const
      {return this->m_useCommonLabel;}
      std::string subGroupLabel(int element) const;

      virtual slctk::AttributeItemPtr buildItem() const;
      void buildGroup(std::vector<slctk::AttributeItemPtr> &group) const;
      virtual void addCategory(const std::string &category);
      virtual void removeCategory(const std::string &category);
      
    protected:
      virtual void updateCategories();
      std::vector<slctk::AttributeItemDefinitionPtr> m_itemDefs;
      std::map<std::string, int> m_itemDefPositions;
      std::vector<std::string> m_labels;
      int m_numberOfGroups;
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
  };
};


#endif /* __slctk_attribute_GroupItemDefinition_h */
