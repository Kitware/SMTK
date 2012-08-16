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
// .NAME GroupItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_GroupItem_h
#define __slctk_attribute_GroupItem_h

#include "AttributeExports.h"
#include "attribute/Item.h"
#include <vector>
namespace slctk
{
  namespace attribute
  {
    class GroupItemDefinition;
    class SLCTKATTRIBUTE_EXPORT GroupItem : public Item
    {
    public:
      GroupItem();
      virtual ~GroupItem();
      virtual Item::Type type() const;
      virtual bool setDefinition(slctk::ConstAttributeItemDefinitionPtr def);
      std::size_t numberOfItemsPerGroup() const;
      std::size_t numberOfGroups() const
      {return this->m_items.size();}
      bool appendGroup();
      bool removeGroup(int element);

      slctk::AttributeItemPtr item(int ith) const
      {return this->item(0, ith);}
      slctk::AttributeItemPtr item(int element, int ith) const
        {return this->m_items[element][ith];}

      slctk::AttributeItemPtr find(const std::string &name)
        {return this->find(0, name);}
      slctk::AttributeItemPtr find(int element, const std::string &name) ;
      slctk::ConstAttributeItemPtr find(const std::string &name) const
        {return this->find(0, name);}
      slctk::ConstAttributeItemPtr find(int element, const std::string &name) const;

    protected:
      std::vector<std::vector<slctk::AttributeItemPtr> >m_items;
  
    private:
    };
  };
};


#endif /* __GroupItem_h */
