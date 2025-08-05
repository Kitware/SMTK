//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME VoidItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_VoidItem_h
#define smtk_attribute_VoidItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <vector>

namespace smtk
{
namespace attribute
{
class VoidItemDefinition;
class SMTKCORE_EXPORT VoidItem : public Item
{
  friend class VoidItemDefinition;

public:
  smtkTypeMacro(smtk::attribute::VoidItem);
  smtkSuperclassMacro(smtk::attribute::Item);
  ~VoidItem() override;
  Item::Type type() const override;

protected:
  VoidItem(Attribute* owningAttribute, int itemPosition);
  VoidItem(Item* owningItem, int myPosition, int mySubGroupPosition);
  bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def) override;
  bool isValidInternal(bool useCategories, const std::set<std::string>& categories) const override;

private:
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_VoidItem_h */
