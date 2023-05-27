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

#ifndef smtk_attribute_DateTimeItemDefinition_h
#define smtk_attribute_DateTimeItemDefinition_h

#include "smtk/attribute/ItemDefinition.h"
#include "smtk/common/DateTimeZonePair.h"
#include <string>

namespace smtk
{
namespace attribute
{
class SMTKCORE_EXPORT DateTimeItemDefinition : public ItemDefinition
{
public:
  smtkTypeMacro(smtk::attribute::DateTimeItemDefinition);
  static smtk::attribute::DateTimeItemDefinitionPtr New(const std::string& myName)
  {
    return smtk::attribute::DateTimeItemDefinitionPtr(new DateTimeItemDefinition(myName));
  }

  ~DateTimeItemDefinition() override;
  Item::Type type() const override;

  const ::smtk::common::DateTimeZonePair& defaultValue() const { return m_defaultValue; }
  bool setDefaultValue(const ::smtk::common::DateTimeZonePair& value);
  bool hasDefault() const { return m_hasDefault; }

  std::size_t numberOfRequiredValues() const { return m_numberOfRequiredValues; }
  bool setNumberOfRequiredValues(std::size_t esize);

  bool isValueValid(const ::smtk::common::DateTimeZonePair& value) const;

  // Returns or sets the display format to use in UI elements.
  // SMTK uses the Qt datetime format expressions documented at
  // http://doc.qt.io/qt-4.8/qdatetime.html#toString
  void setDisplayFormat(const std::string& format) { m_displayFormat = format; }
  std::string displayFormat() const { return m_displayFormat; }

  // Returns or sets flag indicating if a time zone component can be included
  // in the item representation. When set, the UI will include an option to
  // specify a TimeZone with the DateTime data.
  void setUseTimeZone(bool mode) { m_useTimeZone = mode; }
  bool useTimeZone() const { return m_useTimeZone; }

  // Returns or sets flag specifying if UI components include
  // a pop-up calendar widget.
  void setEnableCalendarPopup(bool mode) { m_useCalendarPopup = mode; }
  bool useCalendarPopup() const { return m_useCalendarPopup; }

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(Item* owningItem, int position, int subGroupPosition)
    const override;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  DateTimeItemDefinition(const std::string& myName);

  ::smtk::common::DateTimeZonePair m_defaultValue;
  bool m_hasDefault = false;
  std::size_t m_numberOfRequiredValues = 1;

  // Specifies format string to use in item UI.
  std::string m_displayFormat;

  // Specifies whether to include TimeZone option in item UI.
  bool m_useTimeZone = false;

  // Specifies whether to include calendar popup in item UI.
  bool m_useCalendarPopup = false;

private:
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_DateTimeItemDefinition_h */
