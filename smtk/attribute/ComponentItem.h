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

#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemConstIteratorTemplate.h"

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
  * Then, when deserialized, the attribute resource's resourceManager()
  * is used to recreate the pointer.
  * (Thus the UUIDs are assumed to be present in the resource manager referenced
  * by this item's attribute resource.)
  * If the entity is not present, the returned ComponentPtr will be null and
  * no type checking of attribute values can be performed.
  */
class SMTKCORE_EXPORT ComponentItem : public ReferenceItem
{
public:
  using Component = smtk::resource::Component;
  using ComponentPtr = smtk::resource::ComponentPtr;

  using const_iterator = ReferenceItemConstIteratorTemplate<Component>;
  using value_type = ComponentPtr;

  smtkTypeMacro(smtk::attribute::ComponentItem);
  smtkSuperclassMacro(ReferenceItem);

  /// Destructor
  ~ComponentItem() override;

  /// Return the type of storage used by the item.
  Item::Type type() const override;

  /// Return the \a i-th value as a component.
  ComponentPtr value(std::size_t ii = 0) const
  {
    return std::dynamic_pointer_cast<Component>(ReferenceItem::value(ii));
  }

  /// Set the \a i-th value as a component.
  bool setValue(ComponentPtr value) { return this->setValue(0, value); }
  /// Set the \a i-th value as a component.
  bool setValue(std::size_t ii, ComponentPtr value);

  /**\brief Append a value to the item if possible.
    *
    * This method ensures compile-time type-safety while appendValue() does not.
    */
  bool appendValue(ComponentPtr value, bool allowDuplicates = true)
  {
    return ReferenceItem::appendValue(value, allowDuplicates);
  }

  /// Serialize the \a i-th value to a string.
  std::string valueAsString() const override { return this->valueAsString(0); }
  std::string valueAsString(std::size_t ii) const override;

  /**\brief Return an iterator to the first value in this item.
  *
  */
  const_iterator begin() const
  {
    const_iterator result = const_iterator();
    result.m_refItemIterator = this->ReferenceItem::begin();
    return result;
  }

  /**\brief Return an iterator just past the last value in this item.
    *
    */
  const_iterator end() const
  {
    const_iterator result = const_iterator();
    result.m_refItemIterator = this->ReferenceItem::end();
    return result;
  }

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
