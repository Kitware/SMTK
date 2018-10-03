//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"
#include "smtk/model/Resource.h"

using namespace smtk::attribute;

/// Construct an item given its owning attribute and location in the attribute.
ModelEntityItem::ModelEntityItem(Attribute* owningAttribute, int itemPosition)
  : ComponentItem(owningAttribute, itemPosition)
{
}

/// Construct an item given its owning item and position inside the item.
ModelEntityItem::ModelEntityItem(Item* inOwningItem, int itemPosition, int mySubGroupPosition)
  : ComponentItem(inOwningItem, itemPosition, mySubGroupPosition)
{
}

/// Destructor
ModelEntityItem::~ModelEntityItem()
{
}

/// Return the type of storage used by the item.
Item::Type ModelEntityItem::type() const
{
  return ModelEntityType;
}

/// Return the \a i-th entity stored in this item.
smtk::model::EntityRef ModelEntityItem::value(std::size_t i) const
{
  auto entity = std::dynamic_pointer_cast<smtk::model::Entity>(this->ComponentItem::value(i));
  return (
    entity != nullptr ? entity->referenceAs<smtk::model::EntityRef>() : smtk::model::EntityRef());
}

/**\brief Set the entity stored with this item.
  *
  * This always sets the 0-th item and is a convenience method
  * for cases where only 1 value is needed.
  */
bool ModelEntityItem::setValue(const smtk::model::EntityRef& val)
{
  return this->ComponentItem::setValue(0, val.entityRecord());
}

/// Set the \a i-th value to the given item. This method does no checking to see if \a i is valid.
bool ModelEntityItem::setValue(std::size_t i, const smtk::model::EntityRef& val)
{
  return this->ComponentItem::setValue(i, val.entityRecord());
}

bool ModelEntityItem::appendValue(const smtk::model::EntityRef& val)
{
  return this->ComponentItem::appendValue(val.entityRecord());
}

/**\brief
  *
  */
bool ModelEntityItem::has(const smtk::model::EntityRef& entity) const
{
  return this->find(entity.entityRecord()) >= 0;
}

/**\brief
  *
  */
std::ptrdiff_t ModelEntityItem::find(const smtk::model::EntityRef& entity) const
{
  return this->ComponentItem::find(entity.entityRecord());
}
