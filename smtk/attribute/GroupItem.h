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

#ifndef __smtk_attribute_GroupItem_h
#define __smtk_attribute_GroupItem_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/attribute/Item.h"
#include <vector>
namespace smtk
{
  namespace attribute
  {
    class GroupItemDefinition;
    class SMTKCORE_EXPORT GroupItem : public Item
    {
      friend class GroupItemDefinition;
    public:
      // This method is for wrapping code.  C++ developers should use smtk::dynamic_pointer_cast
      static smtk::attribute::GroupItemPtr CastTo(const smtk::attribute::ItemPtr &p)
      {return smtk::dynamic_pointer_cast<GroupItem>(p);}

      virtual ~GroupItem();
      virtual Item::Type type() const;
      std::size_t numberOfRequiredGroups() const;
      std::size_t numberOfGroups() const
      {return this->m_items.size();}
      bool  setNumberOfGroups(std::size_t newSize);
      std::size_t numberOfItemsPerGroup() const;
      bool appendGroup();
      bool removeGroup(int element);

      smtk::attribute::ItemPtr item(int ith) const
      {return this->item(0, ith);}
      smtk::attribute::ItemPtr item(int element, int ith) const
        {return this->m_items[element][ith];}

      smtk::attribute::ItemPtr find(const std::string &name)
        {return this->find(0, name);}
      smtk::attribute::ItemPtr find(int element, const std::string &name) ;
      smtk::attribute::ConstItemPtr find(const std::string &name) const
        {return this->find(0, name);}
      smtk::attribute::ConstItemPtr find(int element, const std::string &name) const;

      virtual void reset();
    protected:
      GroupItem(Attribute *owningAttribute, int itemPosition);
      GroupItem(Item *owningItem, int myPosition, int mySubGroupPosition);
      virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def);
      // This method will detach all of the items directly owned by
      // this group
      void detachAllItems();
      std::vector<std::vector<smtk::attribute::ItemPtr> >m_items;

    private:
    };
  }
}


#endif /* __GroupItem_h */
