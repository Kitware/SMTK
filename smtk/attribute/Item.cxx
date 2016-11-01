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
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/ValueItem.h"
#include <iostream>
using namespace smtk::attribute;
using namespace smtk;

//----------------------------------------------------------------------------
Item::Item(Attribute *owningAttribute, int itemPosition):
  m_attribute(owningAttribute), m_owningItem(NULL),
  m_position(itemPosition), m_isEnabled(true), m_definition()
{
  this->m_usingDefAdvanceLevelInfo[0] = true;
  this->m_usingDefAdvanceLevelInfo[1] = true;
  this->m_advanceLevel[0] = this->m_advanceLevel[1] = 0;
}

//----------------------------------------------------------------------------
Item::Item(Item *inOwningItem, int itemPosition, int inSubGroupPosition):
  m_attribute(NULL), m_owningItem(inOwningItem),
  m_position(itemPosition), m_subGroupPosition(inSubGroupPosition),
  m_isEnabled(true), m_definition()
{
  this->m_usingDefAdvanceLevelInfo[0] = true;
  this->m_usingDefAdvanceLevelInfo[1] = true;
  this->m_advanceLevel[0] = this->m_advanceLevel[1] = 0;
}

//----------------------------------------------------------------------------
Item::~Item()
{
}
//----------------------------------------------------------------------------
AttributePtr Item::attribute() const
{
  if (this->m_attribute)
    {
    return this->m_attribute->shared_from_this();
    }
  if (this->m_owningItem)
    {
    return this->m_owningItem->attribute();
    }
  return AttributePtr();
}
//----------------------------------------------------------------------------
bool Item::isValid() const
{
  return true;
}
//----------------------------------------------------------------------------
std::string Item::name() const
{
  if (!this->m_definition)
    {
    return "";
    }
  return this->m_definition->name();
}
//----------------------------------------------------------------------------
std::string Item::label() const
{
  if (!this->m_definition)
    {
    return "";
    }
  return this->m_definition->label();
}
//----------------------------------------------------------------------------
bool Item::setDefinition(smtk::attribute::ConstItemDefinitionPtr def)
{
  if (this->m_definition)
    {
    return false;
    }
  this->m_definition = def;
  if (def && def->isOptional())
    {
    this->m_isEnabled = def->isEnabledByDefault();
    }
  return true;
}
//----------------------------------------------------------------------------
bool Item::isOptional() const
{
  if (!this->m_definition)
    {
    return false;
    }
  return this->m_definition->isOptional();
}
//----------------------------------------------------------------------------=
bool Item::isEnabled() const
{
  // determine if the item is locally enabled - meaning that
  //either it is not optional or its enabled flag is true
  bool enabled = this->isOptional() ? this->m_isEnabled : true;
  
  // If it is not enabled we are done
  if (!enabled)
    {
      return false;
    }
  
  // If there is no owning item then its enabled
  if (!this->m_owningItem)
    {
      return true;
    }

  // Else delegate this to the owning item 
  return this->m_owningItem->isEnabled();
}
//----------------------------------------------------------------------------
bool Item::isMemberOf(const std::string &category) const
{
  return this->definition()->isMemberOf(category);
}
//----------------------------------------------------------------------------
bool Item::isMemberOf(const std::vector<std::string> &categories) const
{
  return this->definition()->isMemberOf(categories);
}
//----------------------------------------------------------------------------
void Item::reset()
{
  if (this->m_definition && this->m_definition->isOptional())
    {
    this->m_isEnabled = this->m_definition->isEnabledByDefault();
    }
}
//----------------------------------------------------------------------------
void Item::setAdvanceLevel(int mode, int level)
{
  if ((mode < 0) || (mode > 1))
    {
    return;
    }
  this->m_usingDefAdvanceLevelInfo[mode] = false;
  this->m_advanceLevel[mode] = level;
}
//----------------------------------------------------------------------------
void Item::unsetAdvanceLevel(int mode)
{
  if ((mode < 0) || (mode > 1))
    {
    return;
    }
  this->m_usingDefAdvanceLevelInfo[mode] = true;
}
//----------------------------------------------------------------------------
int Item::advanceLevel(int mode) const
{
  // Any invalid mode returns mode = 0
  if ((mode < 0) || (mode > 1))
    {
    mode = 0;
    }
  if (this->m_definition && this->m_usingDefAdvanceLevelInfo[mode])
    {
    return this->m_definition->advanceLevel(mode);
    }
  return this->m_advanceLevel[mode];
}
//----------------------------------------------------------------------------
bool Item::assign(ConstItemPtr &sourceItem, unsigned int)
{
  // Assigns my contents to be same as sourceItem
  m_isEnabled = sourceItem->isEnabled();
  for (unsigned i=0; i<2; ++i)
    {
    if (!sourceItem->usingDefinitionAdvanceLevel(i))
      {
      this->setAdvanceLevel(i, sourceItem->advanceLevel(i));
      }
    }  // for
  return true;
}
//----------------------------------------------------------------------------
std::string Item::type2String(Item::Type t)
{
  switch (t)
    {
    case ATTRIBUTE_REF:
      return "AttributeRef";
    case COLOR:
      return "Color";
    case DIRECTORY:
      return "Directory";
    case DOUBLE:
      return "Double";
    case FILE:
      return "File";
    case GROUP:
      return "Group";
    case INT:
      return "Int";
    case STRING:
      return "String";
    case VOID:
      return "Void";
    case MODEL_ENTITY:
      return "ModelEntity";
    case MESH_SELECTION:
      return "MeshSelection";
    case MESH_ENTITY:
      return "MeshEntity";
    case DATE_TIME:
      return "DateTime";
    default:
      return "";
    }
  return "Error!";
}
//----------------------------------------------------------------------------
Item::Type Item::string2Type(const std::string &s)
{
  if (s == "AttributeRef")
    {
    return ATTRIBUTE_REF;
    }
  if (s == "Color")
    {
    return COLOR;
    }
  if (s == "Directory")
    {
    return DIRECTORY;
    }
  if (s == "Double")
    {
    return DOUBLE;
    }
  if (s == "File")
    {
    return FILE;
    }
  if (s == "Group")
    {
    return GROUP;
    }
  if (s == "Int")
    {
    return INT;
    }
  if (s == "String")
    {
    return STRING;
    }
  if (s == "Void")
    {
    return VOID;
    }
  if (s == "ModelEntity")
    {
    return MODEL_ENTITY;
    }
  if (s == "MeshSelection")
    {
    return MESH_SELECTION;
    }
  if (s == "MeshEntity")
    {
    return MESH_ENTITY;
    }
  if (s == "DateTime")
    {
    return DATE_TIME;
    }
  return NUMBER_OF_TYPES;
}
//----------------------------------------------------------------------------
