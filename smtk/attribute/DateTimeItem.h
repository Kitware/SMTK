//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DateTimeItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_DateTimeItem_h
#define __smtk_attribute_DateTimeItem_h

#include "smtk/CoreExports.h"
#include "smtk/attribute/Item.h"
#include "smtk/common/DateTimeZonePair.h"
#include <vector>

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class DateTimeItemDefinition;
    class SMTKCORE_EXPORT DateTimeItem : public Item
    {
      friend class DateTimeItemDefinition;
    public:
      smtkTypeMacro(DateTimeItem);
      virtual ~DateTimeItem();
      virtual Item::Type type() const;
      virtual bool isValid() const;

      std::size_t numberOfValues() const
      {return this->m_values.size();}
      bool  setNumberOfValues(std::size_t newSize);
      std::size_t numberOfRequiredValues() const;
      ::smtk::common::DateTimeZonePair value(std::size_t element=0) const
      {return this->m_values[element];}
      bool setValue(const ::smtk::common::DateTimeZonePair& val)
      {return this->setValue(0, val);}
      bool setValue(std::size_t element, const ::smtk::common::DateTimeZonePair& val);
      virtual void reset();
      virtual bool setToDefault(std::size_t elementIndex=0);
      // Returns true if there is a default defined and the item is curently set to it
      virtual bool isUsingDefault(std::size_t elementIndex) const;
      // This method tests all of the values of the items w/r the default value
      virtual bool isUsingDefault() const;
      virtual bool isSet(std::size_t element=0) const
      {return this->m_isSet[element];}
      virtual void unset(std::size_t element=0)
      {this->m_isSet[element] = false;}

      // Assigns this item to be equivalent to another. Options are processed by derived item classes.
      // The options are defined in Item.h. Returns true if success and false if a problem occured.
      virtual bool assign(smtk::attribute::ConstItemPtr &sourceItem, unsigned int options = 0);
    protected:
      DateTimeItem(Attribute *owningAttribute, int itemPosition);
      DateTimeItem(Item *owningItem, int myPosition, int mySubGroupPosition);
#ifndef SHIBOKEN_SKIP
      virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def);
      ConstDateTimeItemDefinitionPtr itemDefinition() const;
#endif
      std::vector<::smtk::common::DateTimeZonePair> m_values;
      std::vector<bool> m_isSet;

    private:
    };
  }
}

#endif /* __smtk_attribute_DateTimeItem_h */
