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
// .NAME Item.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_Item_h
#define __smtk_attribute_Item_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <map>
#include <string>
#include <vector>


namespace smtk
{
  namespace attribute
  {
    class ItemDefinition;
    class GroupItem;
    class Attribute;

    class SMTKCORE_EXPORT Item
    {
    public:
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
       NUMBER_OF_TYPES
     };

     Item(Attribute *owningAttribute, int itemPosition);
     Item(Item *owningItem, int myPosition, int mySubGroupPOsition);
     virtual ~Item();
     std::string name() const;
     std::string label() const;
     virtual Item::Type type() const = 0;
     virtual bool setDefinition(smtk::ConstAttributeItemDefinitionPtr def);
     smtk::ConstAttributeItemDefinitionPtr definition() const
     {return this->m_definition;}

     // Return the attribute that owns this item
     smtk::AttributePtr attribute() const;
     smtk::AttributeItemPtr owningItem() const
     {return (this->m_owningItem ? this->m_owningItem->pointer() :
              smtk::AttributeItemPtr());}
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
     smtk::AttributeItemPtr pointer() const;
     bool isOptional() const;

     // isEnabled only matters for optional items.  All non-optional
     // items will return true for isEnabled regardless of the value
     // of m_isEnabled
     bool isEnabled() const;
     void setIsEnabled(bool isEnabledValue)
     {this->m_isEnabled = isEnabledValue;}

     bool isMemberOf(const std::string &category) const;
     bool isMemberOf(const std::vector<std::string> &categories) const;

     // void setUserData(const std::string &key, void *value)
     //   {this->m_userData[key] = value;}
     // void *userData(const std::string &key) const;
     // void clearUserData(const std::string &key)
     // {this->m_userData.erase(key);}
     // void clearAllUserData()
     // {this->m_userData.clear();}

     virtual void reset();

     //This should be used only by attributes
     void detachOwningAttribute()
     {this->m_attribute = NULL;}
     //This should only be called by the item that owns
     // this one
     void detachOwningItem()
     {this->m_owningItem = NULL;}

     static std::string type2String(Item::Type t);
     static Item::Type string2Type(const std::string &s);

     protected:
      Attribute *m_attribute;
      Item *m_owningItem;
      int m_position;
      int m_subGroupPosition;
      bool m_isEnabled;
      mutable std::string m_tempString;
      smtk::ConstAttributeItemDefinitionPtr m_definition;
      // std::map<std::string, void *> m_userData;
    private:

    };
//----------------------------------------------------------------------------
    // inline void *Item::userData(const std::string &key) const
    // {
    //   std::map<std::string, void *>::const_iterator it =
    //     this->m_userData.find(key);
    //   return ((it == this->m_userData.end()) ? NULL : it->second);
    // }
  };
};

#endif /* __smtk_attribute_Item_h */
