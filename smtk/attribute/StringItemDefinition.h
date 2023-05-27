//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME StringItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_StringItemDefinition_h
#define smtk_attribute_StringItemDefinition_h

#include "smtk/attribute/ValueItemDefinitionTemplate.h"

namespace smtk
{
namespace attribute
{
class SMTKCORE_EXPORT StringItemDefinition : public ValueItemDefinitionTemplate<std::string>
{
public:
  smtkTypeMacro(smtk::attribute::StringItemDefinition);
  static smtk::attribute::StringItemDefinitionPtr New(const std::string& myName)
  {
    return smtk::attribute::StringItemDefinitionPtr(new StringItemDefinition(myName));
  }

  ~StringItemDefinition() override;
  Item::Type type() const override;
  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(Item* owningItem, int position, int subGroupPosition)
    const override;
  bool isMultiline() const { return m_multiline; }
  void setIsMultiline(bool val) { m_multiline = val; }

  //Set/Get the secure property of the item definition
  //Note this is purely a GUI hint for now - the default is false.  Also this
  // will not be supported when the Item is multiline.
  void setIsSecure(bool val) { m_secure = val; }
  bool isSecure() const { return m_secure; }

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  StringItemDefinition(const std::string& myName);
  bool m_multiline{ false };

private:
  bool m_secure{ false };
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_StringItemDefinition_h */
