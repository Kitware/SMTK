//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ModelEntityItem.h - Hold a model entity as an attribute value.
// .SECTION Description
// An attribute whose value is a model entity (such as an edge or face).
//
// While the actual value stored is a EntityRef, only the UUID of the model entity
// is saved when the attribute is serialized.
// Then, when deserialized, the attribute resource's refModelManager()
// is used to recreate the entityref.
// (Thus the UUID is assumed to be present in the model manager referenced by
// the attribute resource. See smtk::attrib::Resource::refModelManager().)
// If the entity is not present, the returned smtk::model::EntityRef
// instances will be invalid and no type checking of attribute values
// can be performed.
//
// .SECTION See Also

#ifndef smtk_attribute_ModelEntityItem_h
#define smtk_attribute_ModelEntityItem_h

#include "smtk/CoreExports.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/model/EntityRef.h"

namespace smtk
{
namespace common
{
class UUID;
}
namespace attribute
{

class Attribute;
class ModelEntityItemDefinition;
class SMTKCORE_EXPORT ModelEntityItem : public ComponentItem
{
  friend class ModelEntityItemDefinition;

public:
  smtkTypeMacro(smtk::attribute::ModelEntityItem);
  smtkSuperclassMacro(smtk::attribute::ComponentItem);
  ~ModelEntityItem() override;

  using ComponentItem::appendValue;
  using ComponentItem::setValue;

  Item::Type type() const override;

  smtk::model::EntityRef value(std::size_t element = 0) const;
  bool setValue(const smtk::model::EntityRef& val);
  bool setValue(std::size_t element, const smtk::model::EntityRef& val);

  template<typename I>
  bool setValues(I vbegin, I vend, std::size_t offset = 0);
  template<typename I>
  bool appendValues(I vbegin, I vend);

  bool appendValue(const smtk::model::EntityRef& val);

  bool contains(const smtk::model::EntityRef& entity) const;

  std::ptrdiff_t find(const smtk::model::EntityRef& entity) const;

  using Superclass::valueAsString;

protected:
  friend class Definition;

  ModelEntityItem(Attribute* owningAttribute, int itemPosition);
  ModelEntityItem(Item* owningItem, int myPosition, int mySubGroupPosition);
};

template<typename I>
bool ModelEntityItem::setValues(I vbegin, I vend, std::size_t offset)
{
  bool ok = false;
  std::size_t num = vend - vbegin + offset;
  if (this->setNumberOfValues(num))
  {
    ok = true;
    std::size_t i = 0;
    for (I it = vbegin; it != vend; ++it, ++i)
    {
      if (!this->setValue(offset + i, *it))
      {
        ok = false;
        break;
      }
    }
  }
  // Enable or disable the item if it is optional.
  if (ok)
  {
    this->setIsEnabled(num > 0);
  }
  return ok;
}

template<typename I>
bool ModelEntityItem::appendValues(I vbegin, I vend)
{
  return this->setValues(vbegin, vend, this->numberOfValues());
}

} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_ModelEntityItem_h */
