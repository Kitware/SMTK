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
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ValueItem.h"

#include <iostream>
using namespace smtk::attribute;
using namespace smtk;

Item::Item(Attribute* owningAttribute, int itemPosition)
  : m_attribute(owningAttribute)
  , m_owningItem(nullptr)
  , m_position(itemPosition)
  , m_isEnabled(true)
  , m_isIgnored(false)
  , m_forceRequired(false)
{
  m_hasLocalAdvanceLevelInfo[0] = false;
  m_hasLocalAdvanceLevelInfo[1] = false;
  m_localAdvanceLevel[0] = m_localAdvanceLevel[1] = 0;
}

Item::Item(Item* inOwningItem, int itemPosition, int inSubGroupPosition)
  : m_attribute(nullptr)
  , m_owningItem(inOwningItem)
  , m_position(itemPosition)
  , m_subGroupPosition(inSubGroupPosition)
  , m_isEnabled(true)
  , m_isIgnored(false)
  , m_forceRequired(false)
{
  m_hasLocalAdvanceLevelInfo[0] = false;
  m_hasLocalAdvanceLevelInfo[1] = false;
  m_localAdvanceLevel[0] = m_localAdvanceLevel[1] = 0;
}

Item::~Item() = default;

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

bool Item::isValid(bool useActiveCategories) const
{
  // If the item is not enabled, or marked to be ignored, then return true since the item
  // is not going to have an effect
  if ((!this->isEnabled()) || this->isIgnored())
  {
    return true;
  }

  // If the resource has active categories enabled, use them
  if (useActiveCategories)
  {
    auto myAttribute = this->attribute();
    if (myAttribute)
    {
      auto aResource = myAttribute->attributeResource();
      if (aResource && aResource->activeCategoriesEnabled())
      {
        return this->isValidInternal(true, aResource->activeCategories());
      }
    }
  }

  std::set<std::string> cats;
  return this->isValidInternal(false, cats);
}

bool Item::isRelevant(bool includeCategories, bool includeReadAccess, unsigned int readAccessLevel)
  const
{
  if (m_isIgnored)
  {
    return false; // Item has been marked to be ignored
  }

  if (includeCategories)
  {
    auto myAttribute = this->attribute();
    if (myAttribute)
    {
      auto aResource = myAttribute->attributeResource();
      if (aResource && aResource->activeCategoriesEnabled())
      {
        if (!this->categories().passes(aResource->activeCategories()))
        {
          return false;
        }
      }
    }
  }

  return (includeReadAccess ? (this->advanceLevel() <= readAccessLevel) : true);
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

smtk::attribute::ItemPtr Item::findInternal(const std::string& /*unused*/, SearchStyle /*unused*/)
{
  // By default there are no children to search
  return nullptr;
}

smtk::attribute::ConstItemPtr Item::findInternal(
  const std::string& /*unused*/,
  SearchStyle /*unused*/) const
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
  return ((!m_forceRequired) && m_definition->isOptional());
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

const smtk::attribute::Categories& Item::categories() const
{
  return this->definition()->categories();
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

void Item::setLocalAdvanceLevel(int mode, unsigned int level)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  m_hasLocalAdvanceLevelInfo[mode] = true;
  m_localAdvanceLevel[mode] = level;
}

void Item::unsetLocalAdvanceLevel(int mode)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  m_hasLocalAdvanceLevelInfo[mode] = false;
}

unsigned int Item::advanceLevel(int mode) const
{
  // Any invalid mode returns mode = 0
  if ((mode < 0) || (mode > 1))
  {
    mode = 0;
  }
  unsigned int level = 0;
  if (m_hasLocalAdvanceLevelInfo[mode])
  {
    level = m_localAdvanceLevel[mode];
  }
  else if (m_definition)
  {
    level = m_definition->advanceLevel(mode);
  }

  if (m_owningItem)
  {
    return std::max(m_owningItem->advanceLevel(mode), level);
  }

  if (m_attribute)
  {
    return std::max(m_attribute->advanceLevel(mode), level);
  }
  return level;
}

Item::Status Item::assign(
  const smtk::attribute::ConstItemPtr& sourceItem,
  const CopyAssignmentOptions& options)
{
  Status status = this->assign(sourceItem, options, smtk::io::Logger::instance());
  return status;
}

Item::Status Item::assign(
  const smtk::attribute::ConstItemPtr& sourceItem,
  const CopyAssignmentOptions&,
  smtk::io::Logger&)
{
  Status status;
  // Assigns my contents to be same as sourceItem
  m_isEnabled = sourceItem->m_isEnabled;
  m_isIgnored = sourceItem->m_isIgnored;
  m_forceRequired = sourceItem->m_forceRequired;

  for (unsigned i = 0; i < 2; ++i)
  {
    m_hasLocalAdvanceLevelInfo[i] = sourceItem->m_hasLocalAdvanceLevelInfo[i];
    m_localAdvanceLevel[i] = sourceItem->m_localAdvanceLevel[i];
  } // for
  return status;
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
      return "Custom";
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
