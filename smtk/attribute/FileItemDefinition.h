//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME FileItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_FileItemDefinition_h
#define __smtk_attribute_FileItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ItemDefinition.h"

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class SMTKCORE_EXPORT FileItemDefinition:
      public ItemDefinition
    {
    public:
      smtkTypeMacro(FileItemDefinition);
      static smtk::attribute::FileItemDefinitionPtr New(const std::string &myName)
      { return smtk::attribute::FileItemDefinitionPtr(new FileItemDefinition(myName));}

      virtual ~FileItemDefinition();

      virtual Item::Type type() const;
      bool isValueValid(const std::string &val) const;

      virtual smtk::attribute::ItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::attribute::ItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;
      std::size_t numberOfRequiredValues() const
      {return this->m_numberOfRequiredValues;}
      void setNumberOfRequiredValues(std::size_t esize);

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
      const std::string& getFileFilters() const
      {return this->m_fileFilters;}
      void setFileFilters(const std::string &filters)
      {this->m_fileFilters = filters;}

      std::string defaultValue() const
      {return this->m_defaultValue;}
      void setDefaultValue(const std::string& val);
      bool hasDefault() const
      {return m_hasDefault;}

      virtual smtk::attribute::ItemDefinitionPtr
        createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const;
    protected:
      FileItemDefinition(const std::string &myName);
      bool m_shouldExist;
      bool m_shouldBeRelative;
      bool m_useCommonLabel;
      std::vector<std::string> m_valueLabels;
      std::size_t m_numberOfRequiredValues;
      std::string m_fileFilters;

      std::string m_defaultValue;
      bool m_hasDefault;

     private:

    };
  }
}

#endif /* __smtk_attribute_FileItemDefinition_h */
