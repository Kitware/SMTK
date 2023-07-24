//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME StringItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_StringItem_h
#define smtk_attribute_StringItem_h

#include "smtk/CoreExports.h"
#include "smtk/attribute/ValueItemTemplate.h"

namespace smtk
{
namespace attribute
{
class Attribute;
class StringItemDefinition;
class SMTKCORE_EXPORT StringItem : public ValueItemTemplate<std::string>
{
  friend class StringItemDefinition;

public:
  smtkTypeMacro(smtk::attribute::StringItem);
  ~StringItem() override;
  Item::Type type() const override;

  // Returns true if the item's value is not to be displayed in a GUI
  bool isSecure() const;

  bool setValueFromString(std::size_t element, const std::string& val) override;

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
  StringItem(Attribute* owningAttribute, int itemPosition);
  StringItem(Item* owningItem, int myPosition, int mySubGroupPosition);

private:
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_StringItem_h */
