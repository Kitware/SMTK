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

#include "smtk/attribute/ReferenceItem.txx"

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
class SMTKCORE_EXPORT ResourceItem : public ReferenceItem<smtk::resource::Resource>
{
public:
  smtkTypeMacro(smtk::attribute::ResourceItem);
  smtkSuperclassMacro(ReferenceItem<smtk::resource::Resource>);

  /// Destructor
  ~ResourceItem() override;

  /// Return the type of storage used by the item.
  Item::Type type() const override;

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
