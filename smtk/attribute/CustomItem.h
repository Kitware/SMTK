//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_CustomItem_h
#define __smtk_attribute_CustomItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/CustomItemDefinition.h"
#include "smtk/attribute/Item.h"

#include "nlohmann/json.hpp"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

namespace smtk
{
namespace attribute
{
class SMTKCORE_EXPORT CustomItemBase : public Item
{
public:
  smtkTypeMacro(CustomItemBase);

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

template <typename ItemType>
class CustomItem : public CustomItemBase
{
public:
  typedef std::shared_ptr<ItemType> Ptr;

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
}
}

#endif
