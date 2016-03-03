//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Item.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_Item_h
#define __smtk_attribute_Item_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include <map>
#include <queue>
#include <string>
#include <vector>


namespace smtk
{
  namespace attribute
  {
    class ItemDefinition;
    class GroupItem;
    class GroupItemDefinition;
    class ValueItemDefinition;
    class Attribute;

    class SMTKCORE_EXPORT Item
    {
      friend class Definition;
      friend class GroupItemDefinition;
      friend class ValueItemDefinition;
    public:
      smtkTypeMacro(Item);
      enum Type
      {
        ATTRIBUTE_REF,
        DOUBLE,
        GROUP,
        INT,
        STRING,
        VOID,
        FILE,
        DIRECTORY,
        COLOR,
        MODEL_ENTITY,
        MESH_SELECTION,
        MESH_ENTITY,
        NUMBER_OF_TYPES
      };

      enum AssignmentOptions
      {
        IGNORE_EXPRESSIONS         = 0x001, //!< Don't assign source value item's expressions
        IGNORE_MODEL_ENTITIES      = 0x002, //!< Don't assign source model entity items
        IGNORE_ATTRIBUTE_REF_ITEMS = 0x004, //!< Don't assign source attribute reference items
        COPY_MODEL_ASSOCIATIONS    = 0x008  //!< If creating attributes, copy their model associations
      };
      
     virtual ~Item();
     std::string name() const;
     std::string label() const;
     virtual Item::Type type() const = 0;
     smtk::attribute::ConstItemDefinitionPtr definition() const
     {return this->m_definition;}

     // Return the attribute that owns this item
     smtk::attribute::AttributePtr attribute() const;
     smtk::attribute::ItemPtr owningItem() const
     {return (this->m_owningItem ? this->m_owningItem->pointer() :
              smtk::attribute::ItemPtr());}
     //Position is the item's location w/r to the owning item if not null
     // or the owning attribute. Currently the only items that can own other items are
     // GroupItem and ValueItem (for expressions)
     int position() const
     {return this->m_position;}

     int subGroupPosition() const
     {return this->m_subGroupPosition;}

     // Returns the shared pointer of the item - if the item is no longer
     // owned by either an attribute or by another item it will return
     // an empty shared pointer
     smtk::attribute::ItemPtr pointer() const;
     // Returns true if the item is optional
     bool isOptional() const;
     
     // isEnabled only matters for optional items.  All non-optional
     // items will return true for isEnabled regardless of the value
     // of m_isEnabled
     bool isEnabled() const;
     void setIsEnabled(bool isEnabledValue)
     {this->m_isEnabled = isEnabledValue;}

     bool isMemberOf(const std::string &category) const;
     bool isMemberOf(const std::vector<std::string> &categories) const;

     //Get the item 's advance level:
     //if mode is 1 then the write access level is returned;
     //else the read access level is returned
     //NOTE: if the advance level was not explicitly set then the item's
     //definition's advance level is returned
     int advanceLevel(int mode=0) const;
     void setAdvanceLevel(int mode, int level);
     // unsetAdvanceLevel causes the item to return its
     // definition advance level information for the specified mode when calling
     // the advanceLevel(mode) method
     void unsetAdvanceLevel(int mode=0);
     // Returns true if the item is returning its Definition's
     // advance level information
     bool usingDefinitionAdvanceLevel(int mode=0) const
      {return (mode==1 ? this->m_usingDefAdvanceLevelInfo[1] : this->m_usingDefAdvanceLevelInfo[0]);}

     void setUserData(const std::string &key, smtk::simulation::UserDataPtr value)
       {this->m_userData[key] = value;}
     smtk::simulation::UserDataPtr userData(const std::string &key) const;
     void clearUserData(const std::string &key)
     {this->m_userData.erase(key);}
     void clearAllUserData()
     {this->m_userData.clear();}

     virtual void reset();

     //This should be used only by attributes
     void detachOwningAttribute()
     {this->m_attribute = NULL;}
     //This should only be called by the item that owns
     // this one
     void detachOwningItem()
     {this->m_owningItem = NULL;}

     // Assigns this item to be equivalent to another.  Options are processed by derived item classes
     // Returns true if success and false if a problem occured
     virtual bool assign(smtk::attribute::ConstItemPtr &sourceItem, unsigned int options = 0);
 
     static std::string type2String(Item::Type t);
     static Item::Type string2Type(const std::string &s);

    protected:
     Item(Attribute *owningAttribute, int itemPosition);
     Item(Item *owningItem, int myPosition, int mySubGroupPOsition);
     virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def);
     Attribute *m_attribute;
     Item *m_owningItem;
     int m_position;
     int m_subGroupPosition;
     bool m_isEnabled;
     mutable std::string m_tempString;
     smtk::attribute::ConstItemDefinitionPtr m_definition;
     std::map<std::string, smtk::simulation::UserDataPtr > m_userData;
    private:
     bool m_usingDefAdvanceLevelInfo[2];
     int m_advanceLevel[2];
    };
//----------------------------------------------------------------------------
    inline smtk::simulation::UserDataPtr Item::userData(const std::string &key) const
    {
      std::map<std::string, smtk::simulation::UserDataPtr >::const_iterator it =
        this->m_userData.find(key);
      return ((it == this->m_userData.end()) ? smtk::simulation::UserDataPtr() : it->second);
    }
  }
}

#endif /* __smtk_attribute_Item_h */
