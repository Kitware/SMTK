//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DirectoryItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_DirectoryItemDefinition_h
#define __smtk_attribute_DirectoryItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ItemDefinition.h"

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class SMTKCORE_EXPORT DirectoryItemDefinition:
      public ItemDefinition
    {
    public:
      smtkTypeMacro(DirectoryItemDefinition);
      static smtk::attribute::DirectoryItemDefinitionPtr New(const std::string &myName)
      { return smtk::attribute::DirectoryItemDefinitionPtr(new DirectoryItemDefinition(myName));}

      virtual ~DirectoryItemDefinition();

      virtual Item::Type type() const;
      bool isValueValid(const std::string &val) const;

      virtual smtk::attribute::ItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::attribute::ItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;
      // Returns or Sets the def's extensiblity property.  If true then items from this def
      // can have a variable number of items.  The number of items is always <= to number of
      // required items and max number of items (provided max number of items > 0)
      // Default value is false.
      bool isExtensible() const
      {return this->m_isExtensible;}
      void setIsExtensible(bool mode);

      std::size_t numberOfRequiredValues() const
      {return this->m_numberOfRequiredValues;}
      bool setNumberOfRequiredValues(std::size_t esize);

     // Returns or Sets the maximum number of items that items from this def can have.
      // if 0 is returned then there is no max limit.  Default value is 0
      // Note that this is used only when the def is extensible
      std::size_t maxNumberOfValues() const
      {return this->m_maxNumberOfValues;}
      // Returns false if the new max is less than the number of required groups
      // and is not 0
      bool setMaxNumberOfValues(std::size_t esize);

      bool hasValueLabels() const
      {return !this->m_valueLabels.empty();}

      void setValueLabel(std::size_t element, const std::string &elabel);
      void setCommonValueLabel(const std::string &elabel);
      bool usingCommonLabel() const
      {return this->m_useCommonLabel;}
      std::string valueLabel(std::size_t element) const;
      bool shouldExist() const
      {return this->m_shouldExist;}
      void setShouldExist(bool val)
      { this->m_shouldExist = val;}
      bool shouldBeRelative() const
      {return this->m_shouldBeRelative;}
      void setShouldBeRelative(bool val)
      {this->m_shouldBeRelative = val;}
      std::string defaultValue() const
      {return this->m_defaultValue;}
      void setDefaultValue(const std::string& val);
      void unsetDefaultValue()
      {this->m_hasDefault=false;}
      bool hasDefault() const
      {return m_hasDefault;}

 
      virtual smtk::attribute::ItemDefinitionPtr
        createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const;
    protected:
      DirectoryItemDefinition(const std::string &myName);
      bool m_shouldExist;
      bool m_shouldBeRelative;
      bool m_useCommonLabel;
      bool m_isExtensible;
      bool m_hasDefault;
      std::string m_defaultValue;
      std::vector<std::string> m_valueLabels;
      std::size_t m_numberOfRequiredValues;
      std::size_t m_maxNumberOfValues;
    private:

    };
  }
}

#endif /* __smtk_attribute_DirectoryItemDefinition_h */
