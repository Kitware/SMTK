//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_CustomItem_h
#define smtk_attribute_CustomItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/CustomItemDefinition.h"
#include "smtk/attribute/Item.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace pugi
{
class xml_node;
}

namespace smtk
{
namespace attribute
{

/// A base class for custom (i.e. user-defined) items. This class defines the
/// requisite API for custom items.
class SMTKCORE_EXPORT CustomItemBase : public Item
{
public:
  smtkTypeMacro(smtk::attribute::CustomItemBase);
  smtkSuperclassMacro(smtk::attribute::Item);

  CustomItemBase(smtk::attribute::Attribute* owningAttribute, int itemPosition)
    : Item(owningAttribute, itemPosition)
  {
  }

  CustomItemBase(smtk::attribute::Item* owningItem, int myPosition, int mySubGroupPosition)
    : Item(owningItem, myPosition, mySubGroupPosition)
  {
  }

  virtual const CustomItemBase& operator>>(nlohmann::json& json) const = 0;
  virtual CustomItemBase& operator<<(const nlohmann::json& json) = 0;

  virtual const CustomItemBase& operator>>(pugi::xml_node& node) const = 0;
  virtual CustomItemBase& operator<<(const pugi::xml_node& node) = 0;
};

/// Custom items inherit from a specialization of this template class, using the
/// corresponding custom item type as the template parameter.
template<typename ItemType>
class CustomItem : public CustomItemBase
{
public:
  smtkTypedefs(smtk::attribute::CustomItem<ItemType>);
  std::string typeName() const override
  {
    std::ostringstream tname;
    tname << "smtk::attribute::CustomItem<" << smtk::common::typeName<ItemType>() << ">";
    return tname.str();
  }
  smtk::string::Token typeToken() const override { return smtk::string::Token(this->typeName()); }
  smtkInheritanceHierarchy(smtk::attribute::CustomItem<ItemType>);
  smtkSuperclassMacro(smtk::attribute::CustomItemBase);

private:
  // Prevent smtk::common::typeName<Self>() from grabbing our subclass's type_name
  // by changing its access specifier:
  using CustomItemBase::type_name;

public:
  static Ptr New(const std::string& myName) { return Ptr(new ItemType(myName)); }

  CustomItem(smtk::attribute::Attribute* owningAttribute, int itemPosition)
    : CustomItemBase(owningAttribute, itemPosition)
  {
  }

  CustomItem(smtk::attribute::Item* owningItem, int myPosition, int mySubGroupPosition)
    : CustomItemBase(owningItem, myPosition, mySubGroupPosition)
  {
  }

  bool setDefinition(smtk::attribute::ConstItemDefinitionPtr adef) override
  {
    const CustomItemDefinition<ItemType>* def =
      dynamic_cast<const CustomItemDefinition<ItemType>*>(adef.get());
    return !((def == nullptr) || (!Item::setDefinition(adef)));
  }

  Item::Type type() const override { return this->definition()->type(); }
};
} // namespace attribute
} // namespace smtk

#endif
