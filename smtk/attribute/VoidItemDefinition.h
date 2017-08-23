//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME VoidItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_VoidItemDefinition_h
#define __smtk_attribute_VoidItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ItemDefinition.h"

namespace smtk
{
namespace attribute
{
class SMTKCORE_EXPORT VoidItemDefinition : public ItemDefinition
{
public:
  smtkTypeMacro(VoidItemDefinition);
  static smtk::attribute::VoidItemDefinitionPtr New(const std::string& myName)
  {
    return smtk::attribute::VoidItemDefinitionPtr(new VoidItemDefinition(myName));
  }

  ~VoidItemDefinition() override;
  Item::Type type() const override;
  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const override;
  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  VoidItemDefinition(const std::string& myName);

private:
};
}
}

#endif /* __smtk_attribute_VoidItemDefinition_h */
