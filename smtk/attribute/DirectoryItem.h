//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DirectoryItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_DirectoryItem_h
#define __smtk_attribute_DirectoryItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <string>
#include <vector>

namespace smtk
{
  namespace attribute
  {
    class DirectoryItemDefinition;
    class SMTKCORE_EXPORT DirectoryItem : public Item
    {
    friend class DirectoryItemDefinition;
    public:
      smtkTypeMacro(DirectoryItem);
      virtual ~DirectoryItem();
      virtual Item::Type type() const;
      bool shouldBeRelative() const;
      bool shouldExist() const;
      std::size_t numberOfValues() const
      {return this->m_values.size();}
      bool  setNumberOfValues(std::size_t newSize);
      std::size_t numberOfRequiredValues() const;
      std::string value(std::size_t element=0) const
      {return this->m_values[element];}
      bool setValue(const std::string &val)
      {return this->setValue(0, val);}
      bool setValue(std::size_t element, const std::string &val);
      bool appendValue(const std::string &val);
      bool removeValue(int element);
      virtual void reset();
      virtual std::string valueAsString(const std::string &format="") const
      {return this->valueAsString(0, format);}
      virtual std::string valueAsString(std::size_t element, const std::string &format="") const;
      virtual bool isSet(std::size_t element=0) const
      {return this->m_isSet[element];}
      virtual void unset(std::size_t element=0)
      {this->m_isSet[element] = false;}
      virtual void copyFrom(const smtk::attribute::ItemPtr sourceItem,
                            smtk::attribute::Item::CopyInfo& info);

    protected:
      DirectoryItem(Attribute *owningAttribute, int itemPosition);
      DirectoryItem(Item *owningItem, int position, int subGroupPosition);
      virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef);
      std::vector<std::string>m_values;
      std::vector<bool> m_isSet;
    private:
    };
  }
}


#endif /* __smtk_attribute_DirectoryItem_h */
