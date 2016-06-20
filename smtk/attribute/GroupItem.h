//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME GroupItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_GroupItem_h
#define __smtk_attribute_GroupItem_h

#include "smtk/CoreExports.h"
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
      smtkTypeMacro(GroupItem);
      virtual ~GroupItem();
      virtual Item::Type type() const;
      virtual bool isValid() const;
      std::size_t numberOfRequiredGroups() const;
      std::size_t maxNumberOfGroups() const;

      bool isExtensible() const;

      std::size_t numberOfGroups() const
      {return this->m_items.size();}
      bool  setNumberOfGroups(std::size_t newSize);
      std::size_t numberOfItemsPerGroup() const;
      bool appendGroup();
      bool removeGroup(std::size_t element);

      smtk::attribute::ItemPtr item(std::size_t ith) const
      {return this->item(0, ith);}
      smtk::attribute::ItemPtr item(std::size_t element, std::size_t ith) const
        {return this->m_items[element][ith];}

      smtk::attribute::ItemPtr find(const std::string &inName)
        {return this->find(0, inName);}
      smtk::attribute::ItemPtr find(std::size_t element, const std::string &name) ;
      smtk::attribute::ConstItemPtr find(const std::string &inName) const
        {return this->find(0, inName);}
      smtk::attribute::ConstItemPtr find(std::size_t element, const std::string &name) const;

      template<typename T>
      typename T::Ptr findAs(const std::string& name);

      template<typename T>
      typename T::ConstPtr findAs(const std::string& name) const;

      virtual void reset();
      // Assigns this item to be equivalent to another.  Options are processed by derived item classes
      // Returns true if success and false if a problem occured - options are use when copying sub-items.
      // See Items.h for a description of these options.
      virtual bool assign(smtk::attribute::ConstItemPtr &sourceItem, unsigned int options = 0);
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
//----------------------------------------------------------------------------
    template<typename T>
    typename T::Ptr GroupItem::findAs(const std::string& iname)
    {
    return smtk::dynamic_pointer_cast<T>(this->find(iname));
    }
//----------------------------------------------------------------------------
    template<typename T>
    typename T::ConstPtr GroupItem::findAs(const std::string& iname) const
    {
    return smtk::dynamic_pointer_cast<const T>(this->find(iname));
    }
//----------------------------------------------------------------------------

  }
}


#endif /* __GroupItem_h */
