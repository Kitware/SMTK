//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/operators/EditProperties.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/operation/MarkGeometry.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/properties/CoordinateFrame.h"

#include "smtk/operation/EditProperties_xml.h"

using smtk::attribute::DoubleItem;
using smtk::attribute::GroupItem;
using smtk::attribute::IntItem;
using smtk::attribute::ReferenceItem;
using smtk::attribute::StringItem;
using Float = double;
using FloatList = std::vector<Float>;
using Integer = long;
using IntegerList = std::vector<Integer>;
using String = std::string;
using StringList = std::vector<String>;
using smtk::resource::Component;
using smtk::resource::properties::CoordinateFrame;

namespace smtk
{
namespace operation
{

template<typename V, typename VL, typename VI>
void EditPropertiesValue(
  const std::string& name,
  const typename VI::Ptr& item,
  smtk::attribute::ReferenceItemPtr& entities,
  bool erase = false)
{
  if (!item || erase)
  {
    // Erase the property of this type from these entities,
    // if they had the property in the first place.
    for (smtk::resource::PersistentObjectPtr entity : *entities)
    {
      entity->properties().erase<VL>(name);
    }
  }
  else
  {
    // Get the array of values from the item.
    VL values;
    values.reserve(item->numberOfValues());
    for (std::size_t i = 0; i < item->numberOfValues(); ++i)
    {
      values.push_back(item->value(i));
    }

    // Add or overwrite the property with the values.
    for (smtk::resource::PersistentObjectPtr entity : *entities)
    {
      entity->properties().get<VL>()[name] = values;
    }
  }
}

template<typename V, typename VI>
void EditPropertiesValue(
  const std::string& name,
  const typename VI::Ptr& item,
  smtk::attribute::ReferenceItemPtr& entities,
  bool erase = false)
{
  if (!item || erase)
  {
    // Erase the property of this type from these entities,
    // if they had the property in the first place.
    for (smtk::resource::PersistentObjectPtr entity : *entities)
    {
      entity->properties().erase<V>(name);
    }
  }
  else
  {
    // Get the array of values from the item.
    V value = item->value();

    // Add or overwrite the property with the values.
    for (smtk::resource::PersistentObjectPtr entity : *entities)
    {
      entity->properties().get<V>()[name] = value;
    }
  }
}

void EditPropertiesValue(
  const std::string& name,
  const GroupItem::Ptr& item,
  smtk::attribute::ReferenceItemPtr& entities,
  bool erase = false)
{
  if (!item || erase)
  {
    // Erase the property of this type from these entities,
    // if they had the property in the first place.
    for (smtk::resource::PersistentObjectPtr entity : *entities)
    {
      entity->properties().erase<CoordinateFrame>(name);
      // For special names, ensure the geometry is updated for rendering:
      if (name.substr(0, 14) == "smtk.geometry." || name == "transform")
      {
        smtk::operation::MarkGeometry().markModified(entity);
      }
    }
  }
  else
  {
    // Construct a CoordinateFrame object from the GroupItem.
    smtk::attribute::ConstGroupItemPtr constItem(item);
    CoordinateFrame value;
    for (int ii = 0; ii < 3; ++ii)
    {
      value.origin[ii] = constItem->findAs<DoubleItem>(0, "Origin")->value(ii);
      value.xAxis[ii] = constItem->findAs<DoubleItem>(0, "XAxis")->value(ii);
      value.yAxis[ii] = constItem->findAs<DoubleItem>(0, "YAxis")->value(ii);
      value.zAxis[ii] = constItem->findAs<DoubleItem>(0, "ZAxis")->value(ii);
    }
    auto parentItem = constItem->findAs<ReferenceItem>(0, "Parent");
    value.parent = smtk::common::UUID::null();
    if (parentItem->isEnabled() && parentItem->isSet(0))
    {
      value.parent = parentItem->value(0)->id();
    }

    // Add or overwrite the property with the values.
    for (smtk::resource::PersistentObjectPtr entity : *entities)
    {
      entity->properties().get<CoordinateFrame>()[name] = value;
      if (name == "transform" || name == "smtk.geometry.transform")
      {
        smtk::operation::MarkGeometry().markModified(entity);
      }
    }
  }
}

EditProperties::Result EditProperties::operateInternal()
{
  smtk::attribute::StringItemPtr nameItem = this->parameters()->findString("name");
  smtk::attribute::IntItemPtr typeItem = this->parameters()->findInt("type");
  smtk::attribute::ItemPtr removeItem = this->parameters()->find("remove");
  bool removeProperty = removeItem->isEnabled();

  auto associations = this->parameters()->associations();

  if (nameItem->value(0).empty())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  switch (typeItem->value())
  {
    case 0: // dealing with strings
    {
      auto stringItem =
        std::dynamic_pointer_cast<smtk::attribute::StringItem>(typeItem->find(("string value")));
      if (stringItem->numberOfValues() == 1)
      {
        EditPropertiesValue<String, StringItem>(
          nameItem->value(0), stringItem, associations, removeProperty);
      }
      else
      {
        EditPropertiesValue<String, StringList, StringItem>(
          nameItem->value(0), stringItem, associations, removeProperty);
      }
      break;
    }
    case 1: // dealing with doubles
    {
      auto floatItem =
        std::dynamic_pointer_cast<smtk::attribute::DoubleItem>(typeItem->find(("float value")));
      if (floatItem->numberOfValues() == 1)
      {
        EditPropertiesValue<Float, DoubleItem>(
          nameItem->value(0), floatItem, associations, removeProperty);
      }
      else
      {
        EditPropertiesValue<Float, FloatList, DoubleItem>(
          nameItem->value(0), floatItem, associations, removeProperty);
      }
      break;
    }
    case 2: // dealing with ints
    {
      auto integerItem =
        std::dynamic_pointer_cast<smtk::attribute::IntItem>(typeItem->find(("integer value")));
      if (integerItem->numberOfValues() == 1)
      {
        EditPropertiesValue<Integer, IntItem>(
          nameItem->value(0), integerItem, associations, removeProperty);
      }
      else
      {
        EditPropertiesValue<Integer, IntegerList, IntItem>(
          nameItem->value(0), integerItem, associations, removeProperty);
      }
      break;
    }
    case 3: // Dealing with Coordinate Systems
    {
      auto groupItem =
        std::dynamic_pointer_cast<smtk::attribute::GroupItem>(typeItem->find(("Coordinate Frame")));
      EditPropertiesValue(nameItem->value(0), groupItem, associations, removeProperty);
    }
    default:
      break;
  }

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItemPtr modifiedItem = result->findComponent("modified");
  for (smtk::resource::PersistentObjectPtr association : *associations)
  {
    if (auto component = std::dynamic_pointer_cast<Component>(association))
    {
      modifiedItem->appendValue(component);
    }
  }

  return result;
}

const char* EditProperties::xmlDescription() const
{
  return EditProperties_xml;
}

} // namespace operation
} // namespace smtk
