//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Item.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/ValueItem.h"
#include <iostream>
using namespace smtk::attribute;
using namespace smtk;

Item::Item(Attribute* owningAttribute, int itemPosition)
  : m_attribute(owningAttribute)
  , m_owningItem(NULL)
  , m_position(itemPosition)
  , m_isEnabled(true)
  , m_definition()
{
  m_usingDefAdvanceLevelInfo[0] = true;
  m_usingDefAdvanceLevelInfo[1] = true;
  m_advanceLevel[0] = m_advanceLevel[1] = 0;
}

Item::Item(Item* inOwningItem, int itemPosition, int inSubGroupPosition)
  : m_attribute(NULL)
  , m_owningItem(inOwningItem)
  , m_position(itemPosition)
  , m_subGroupPosition(inSubGroupPosition)
  , m_isEnabled(true)
  , m_definition()
{
  m_usingDefAdvanceLevelInfo[0] = true;
  m_usingDefAdvanceLevelInfo[1] = true;
  m_advanceLevel[0] = m_advanceLevel[1] = 0;
}

Item::~Item()
{
}

AttributePtr Item::attribute() const
{
  if (m_attribute)
  {
    return m_attribute->shared_from_this();
  }
  if (m_owningItem)
  {
    return m_owningItem->attribute();
  }
  return AttributePtr();
}

bool Item::isValid() const
{
  return true;
}

std::string Item::name() const
{
  if (!m_definition)
  {
    return "";
  }
  return m_definition->name();
}

std::string Item::label() const
{
  if (!m_definition)
  {
    return "";
  }
  return m_definition->label();
}

smtk::attribute::ItemPtr Item::find(const std::string& itemName, SearchStyle style)
{
  return this->findInternal(itemName, style);
}

smtk::attribute::ConstItemPtr Item::find(const std::string& itemName, SearchStyle style) const
{
  return this->findInternal(itemName, style);
}

smtk::attribute::ItemPtr Item::findInternal(const std::string&, SearchStyle)
{
  // By default there are no children to search
  return nullptr;
}

smtk::attribute::ConstItemPtr Item::findInternal(const std::string&, SearchStyle) const
{
  // By default there are no children to search
  return nullptr;
}

bool Item::setDefinition(smtk::attribute::ConstItemDefinitionPtr def)
{
  if (m_definition)
  {
    return false;
  }
  m_definition = def;
  if (def && def->isOptional())
  {
    m_isEnabled = def->isEnabledByDefault();
  }
  return true;
}

bool Item::isOptional() const
{
  if (!m_definition)
  {
    return false;
  }
  return m_definition->isOptional();
}

bool Item::isEnabled() const
{
  // determine if the item is locally enabled - meaning that
  //either it is not optional or its enabled flag is true
  bool enabled = this->isOptional() ? m_isEnabled : true;

  // If it is not enabled we are done
  if (!enabled)
  {
    return false;
  }

  // If there is no owning item then its enabled
  if (!m_owningItem)
  {
    return true;
  }

  // Else delegate this to the owning item
  return m_owningItem->isEnabled();
}

bool Item::isMemberOf(const std::string& category) const
{
  return this->definition()->isMemberOf(category);
}

bool Item::isMemberOf(const std::vector<std::string>& categories) const
{
  return this->definition()->isMemberOf(categories);
}

void Item::reset()
{
  if (m_definition && m_definition->isOptional())
  {
    m_isEnabled = m_definition->isEnabledByDefault();
  }
}

bool Item::rotate(std::size_t fromPosition, std::size_t toPosition)
{
  // No default behavior. Method must be overriden
  (void)fromPosition;
  (void)toPosition;
  return false;
}

void Item::setAdvanceLevel(int mode, int level)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  m_usingDefAdvanceLevelInfo[mode] = false;
  m_advanceLevel[mode] = level;
}

void Item::unsetAdvanceLevel(int mode)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  m_usingDefAdvanceLevelInfo[mode] = true;
}

int Item::advanceLevel(int mode) const
{
  // Any invalid mode returns mode = 0
  if ((mode < 0) || (mode > 1))
  {
    mode = 0;
  }
  if (m_definition && m_usingDefAdvanceLevelInfo[mode])
  {
    return m_definition->advanceLevel(mode);
  }
  return m_advanceLevel[mode];
}

bool Item::assign(ConstItemPtr& sourceItem, unsigned int)
{
  // Assigns my contents to be same as sourceItem
  m_isEnabled = sourceItem->isEnabled();
  for (unsigned i = 0; i < 2; ++i)
  {
    if (!sourceItem->usingDefinitionAdvanceLevel(i))
    {
      this->setAdvanceLevel(i, sourceItem->advanceLevel(i));
    }
  } // for
  return true;
}

std::string Item::type2String(Item::Type t)
{
  switch (t)
  {
    case AttributeRefType:
      return "AttributeRef";
    case ColorType:
      return "Color";
    case DirectoryType:
      return "Directory";
    case DoubleType:
      return "Double";
    case FileType:
      return "File";
    case GroupType:
      return "Group";
    case IntType:
      return "Int";
    case StringType:
      return "String";
    case VoidType:
      return "Void";
    case ModelEntityType:
      return "ModelEntity";
    case MeshSelectionType:
      return "MeshSelection";
    case MeshEntityType:
      return "MeshEntity";
    case DateTimeType:
      return "DateTime";
    case ReferenceType:
      return "Reference";
    case ComponentType:
      return "Component";
    case ResourceType:
      return "Resource";
    default:
      return "";
  }
  return "Error!";
}

Item::Type Item::string2Type(const std::string& s)
{
  if (s == "AttributeRef")
  {
    return AttributeRefType;
  }
  if (s == "Color")
  {
    return ColorType;
  }
  if (s == "Directory")
  {
    return DirectoryType;
  }
  if (s == "Double")
  {
    return DoubleType;
  }
  if (s == "File")
  {
    return FileType;
  }
  if (s == "Group")
  {
    return GroupType;
  }
  if (s == "Int")
  {
    return IntType;
  }
  if (s == "String")
  {
    return StringType;
  }
  if (s == "Void")
  {
    return VoidType;
  }
  if (s == "ModelEntity")
  {
    return ModelEntityType;
  }
  if (s == "MeshSelection")
  {
    return MeshSelectionType;
  }
  if (s == "MeshEntity")
  {
    return MeshEntityType;
  }
  if (s == "DateTime")
  {
    return DateTimeType;
  }
  if (s == "Reference")
  {
    return ReferenceType;
  }
  if (s == "Resource")
  {
    return ResourceType;
  }
  if (s == "Component")
  {
    return ComponentType;
  }
  return NUMBER_OF_TYPES;
}
