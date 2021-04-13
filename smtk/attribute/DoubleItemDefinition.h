//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DoubleItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_DoubleItemDefinition_h
#define __smtk_attribute_DoubleItemDefinition_h

#include "smtk/attribute/ValueItemDefinitionTemplate.h"

namespace smtk
{
namespace attribute
{
class SMTKCORE_EXPORT DoubleItemDefinition : public ValueItemDefinitionTemplate<double>
{
public:
  smtkTypeMacro(smtk::attribute::DoubleItemDefinition);
  static smtk::attribute::DoubleItemDefinitionPtr New(const std::string& myName)
  {
    return smtk::attribute::DoubleItemDefinitionPtr(new DoubleItemDefinition(myName));
  }

  ~DoubleItemDefinition() override;
  Item::Type type() const override;
  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(Item* owningItem, int position, int subGroupPosition)
    const override;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  DoubleItemDefinition(const std::string& myName);

private:
};
} // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_DoubleItemDefinition_h */
