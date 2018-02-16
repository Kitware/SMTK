//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_ComponentItem_h
#define smtk_attribute_ComponentItem_h

#include "smtk/attribute/ReferenceItem.txx"

#include "smtk/resource/Component.h"

namespace smtk
{
namespace common
{
class UUID;
}
namespace attribute
{

class Attribute;
class ComponentItemDefinition;

/**\brief Hold resource components as an attribute value.
  *
  * An attribute whose value is a resource component (such as a mesh set,
  * model entity, or even an attribute).
  *
  * While the actual value stored is a ComponentPtr, only the UUID of the
  * component and its owning resource are saved when the attribute is serialized.
  * Then, when deserialized, the attribute collection's resourceManager()
  * is used to recreate the pointer.
  * (Thus the UUIDs are assumed to be present in the resource manager referenced
  * by this item's attribute collection.)
  * If the entity is not present, the returned ComponentPtr will be null and
  * no type checking of attribute values can be performed.
  */
class SMTKCORE_EXPORT ComponentItem : public ReferenceItem<smtk::resource::Component>
{
public:
  smtkTypeMacro(ComponentItem);
  smtkSuperclassMacro(ReferenceItem<smtk::resource::Component>);

  /// Destructor
  ~ComponentItem() override;

  /// Return the type of storage used by the item.
  Item::Type type() const override;

  /// Serialize the \a i-th value to a string.
  std::string valueAsString(std::size_t i) const override;

protected:
  friend class ComponentItemDefinition;
  friend class Definition;

  /// Construct an item given its owning attribute and location in the attribute.
  ComponentItem(Attribute* owningAttribute, int itemPosition);
  /// Construct an item given its owning item and position inside the item.
  ComponentItem(Item* owningItem, int myPosition, int mySubGroupPosition);
};

} // namespace attribute
} // namespace smtk

#endif
