//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DateTimeItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_DateTimeItemDefinition_h
#define __smtk_attribute_DateTimeItemDefinition_h

#include "smtk/attribute/ValueItemDefinitionTemplate.h"
#include "smtk/attribute/DateTimeZonePair.h"
#include <string>

namespace smtk
{
  namespace attribute
  {
    class SMTKCORE_EXPORT DateTimeItemDefinition :
      public ValueItemDefinitionTemplate<DateTimeZonePair>
    {
    public:
      smtkTypeMacro(DateTimeItemDefinition);
      static smtk::attribute::DateTimeItemDefinitionPtr New(const std::string &myName)
      {return smtk::attribute::DateTimeItemDefinitionPtr(new DateTimeItemDefinition(myName));}

      virtual ~DateTimeItemDefinition();
      virtual Item::Type type() const;

      // Returns or sets the display format to use in UI elements.
      // SMTK uses the Qt datetime format expressions documented at
      // http://doc.qt.io/qt-4.8/qdatetime.html#toString
      void setDisplayFormat(const std::string& format)
      {this->m_displayFormat = format;}
      std::string displayFormat() const
      {return this->m_displayFormat;}

      // Returns or sets flag indicating if a time zone component can be included
      // in the item representation. When set, the UI will include an option to
      // specify a TimeZone with the DateTime data.
      void setUseTimeZone(bool mode)
      {this->m_useTimeZone = mode;}
      bool useTimeZone() const
      {return this->m_useTimeZone;}

      // Returns or sets flag specifying if UI components include
      // a pop-up calendar widget.
      void setEnableCalendarPopup(bool mode)
      {this->m_useCalendarPopup = mode;}
      bool useCalendarPopup() const
      {return m_useCalendarPopup;}

      virtual smtk::attribute::ItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::attribute::ItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;

      virtual smtk::attribute::ItemDefinitionPtr
        createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const;
    protected:
      DateTimeItemDefinition(const std::string &myName);

      // Specifies format string to use in item UI.
      std::string m_displayFormat;

      // Specifies whether to include TimeZone option in item UI.
      bool m_useTimeZone;

      // Specifies whether to include calendar popup in item UI.
      bool m_useCalendarPopup;

    private:

    };
  }
}

#endif /* __smtk_attribute_DateTimeItemDefinition_h */
