//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_ResourceItem_h
#define smtk_attribute_ResourceItem_h

#include "smtk/attribute/ReferenceItem.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace common
{
class UUID;
}
namespace attribute
{

class Attribute;
class ResourceItemDefinition;

/**\brief Hold resources as an attribute value.
  *
  * An attribute whose value is a resource (such as a mesh collection,
  * model manager, or even an attribute collection).
  */
class SMTKCORE_EXPORT ResourceItem : public ReferenceItem
{
public:
  using Resource = smtk::resource::Resource;
  using ResourcePtr = smtk::resource::ResourcePtr;
  smtkTypeMacro(smtk::attribute::ResourceItem);
  smtkSuperclassMacro(ReferenceItem);

  /// Destructor
  ~ResourceItem() override;

  /// Return the type of storage used by the item.
  Item::Type type() const override;

  /// Return the \a ii-th value as a resource.
  ResourcePtr value(std::size_t ii = 0) const;

  /// Set the 0-th \a value, ensuring type-safety.
  bool setValue(ResourcePtr value) { return this->setValue(0, value); }
  /// Set the \a ii-th value, ensuring type-safety.
  bool setValue(std::size_t ii, ResourcePtr value);

  /**\brief Append a value to the item if possible.
    *
    * This method ensures compile-time type-safety while appendObjectValue() does not.
    */
  bool appendValue(ResourcePtr value) { return this->appendObjectValue(value); }

  /// Serialize the \a i-th value to a string.
  std::string valueAsString(std::size_t i) const override;

protected:
  friend class ResourceItemDefinition;
  friend class Definition;

  /// Construct an item given its owning attribute and location in the attribute.
  ResourceItem(Attribute* owningAttribute, int itemPosition);
  /// Construct an item given its owning item and position inside the item.
  ResourceItem(Item* owningItem, int myPosition, int mySubGroupPosition);
};

} // namespace attribute
} // namespace smtk

#endif
