//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME FileSystemItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_FileSystemItem_h
#define __smtk_attribute_FileSystemItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <string>
#include <vector>

namespace smtk
{
  namespace attribute
  {
    class FileSystemItemDefinition;
    class SMTKCORE_EXPORT FileSystemItem : public Item
    {
      friend class FileItemDefinition;
    public:
      smtkTypeMacro(FileSystemItem);
      virtual ~FileSystemItem();
      virtual Item::Type type() const = 0;
      virtual bool isValid() const;
      bool shouldBeRelative() const;
      bool shouldExist() const;
      std::size_t numberOfValues() const
      {return this->m_values.size();}
      bool  setNumberOfValues(std::size_t newSize);
      std::size_t numberOfRequiredValues() const;
      bool isExtensible() const;
      std::size_t maxNumberOfValues() const;
      std::string value(std::size_t element=0) const
      {return this->m_values[element];}
      bool setValue(const std::string &val)
      {return this->setValue(0, val);}
      bool setValue(std::size_t element, const std::string &val);
      bool appendValue(const std::string &val);
      bool removeValue(int element);
      virtual void reset();
      virtual bool setToDefault(std::size_t elementIndex=0);
      // Returns true if there is a default defined and the item is curently set to it
      virtual bool isUsingDefault(std::size_t elementIndex) const;
      // This method tests all of the values of the items w/r the default value
      virtual bool isUsingDefault() const;
      // Does this item have a default value?
      bool hasDefault() const;
      std::string defaultValue() const;
     virtual std::string valueAsString(const std::string &format="") const
      {return this->valueAsString(0, format);}
      virtual std::string valueAsString(std::size_t element, const std::string &format="") const;
      virtual bool isSet(std::size_t element=0) const
      {return this->m_isSet[element];}
      virtual void unset(std::size_t element=0)
      {this->m_isSet[element] = false;}

      // Assigns this item to be equivalent to another.  Options are processed by derived item classes
      // Returns true if success and false if a problem occured.  Does not use options.
      virtual bool assign(smtk::attribute::ConstItemPtr &sourceItem, unsigned int options = 0);

    protected:
      FileSystemItem(Attribute *owningAttribute, int itemPosition);
      FileSystemItem(Item *owningItem, int position, int subGroupPosition);
      virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef);
      std::vector<std::string>m_values;
      std::vector<bool> m_isSet;
    private:
    };
  }
}


#endif /* __smtk_attribute_FileSystemItem_h */
