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
#include "smtk/attribute/Resource.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Observer.h"

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

/// A base class for custom (i.e. user-defined) item definitions. This class
/// defines the requisite API for custom item definitions.
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

/// Custom item definitions inherit from from a specialization of this template
/// class, using the corresponding custom item type as the template parameter.
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
