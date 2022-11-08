//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME IntItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_IntItem_h
#define smtk_attribute_IntItem_h

#include "smtk/CoreExports.h"
#include "smtk/attribute/ValueItemTemplate.h"

namespace smtk
{
namespace attribute
{
class Attribute;
class IntItemDefinition;
class SMTKCORE_EXPORT IntItem : public ValueItemTemplate<int>
{
  friend class IntItemDefinition;

public:
  smtkTypeMacro(smtk::attribute::IntItem);
  ~IntItem() override;
  Item::Type type() const override;

  using Item::assign;
  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occurred.  By default, an attribute being used by this
  // to represent an expression will be copied if needed.  Use IGNORE_EXPRESSIONS option to prevent this
  // When an expression attribute is copied, its  associations are by default not.
  // Use COPY_NEW_ASSOCIATIONS if you want them copied as well.These options are defined in CopyAssigmentOptions.h .
  bool assign(
    const smtk::attribute::ConstItemPtr& sourceItem,
    const CopyAssignmentOptions& options,
    smtk::io::Logger& logger) override;

protected:
  IntItem(Attribute* owningAttribute, int itemPosition);
  IntItem(Item* owningItem, int myPosition, int mySubGroupPosition);

private:
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_IntItem_h */
