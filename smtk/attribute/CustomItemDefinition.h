//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_CustomItemDefinition_h
#define __smtk_attribute_CustomItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

#include "nlohmann/json.hpp"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

namespace smtk
{
namespace attribute
{
class SMTKCORE_EXPORT CustomItemBaseDefinition : public ItemDefinition
{
public:
  smtkTypeMacro(CustomItemBaseDefinition);

  CustomItemBaseDefinition(const std::string& myName)
    : ItemDefinition(myName)
  {
  }

  virtual const CustomItemBaseDefinition& operator>>(nlohmann::json& json) const = 0;
  virtual CustomItemBaseDefinition& operator<<(const nlohmann::json& json) = 0;

  virtual const CustomItemBaseDefinition& operator>>(pugi::xml_node& node) const = 0;
  virtual CustomItemBaseDefinition& operator<<(const pugi::xml_node& node) = 0;
};

template <typename ItemType>
class CustomItemDefinition : public CustomItemBaseDefinition
{
public:
  typedef std::shared_ptr<CustomItemDefinition> Ptr;

  CustomItemDefinition(const std::string& myName)
    : CustomItemBaseDefinition(myName)
  {
  }

  Item::Type type() const override { return Item::NUMBER_OF_TYPES; }

  ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override
  {
    return ItemPtr(new ItemType(owningAttribute, itemPosition));
  }

  ItemPtr buildItem(Item* owningItem, int position, int subGroupPosition) const override
  {
    return ItemPtr(new ItemType(owningItem, position, subGroupPosition));
  }
};
}
}

#endif
