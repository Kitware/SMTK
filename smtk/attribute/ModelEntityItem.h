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
// Then, when deserialized, the attribute system's refModelManager()
// is used to recreate the entityref.
// (Thus the UUID is assumed to be present in the model manager referenced by
// the attribute system. See smtk::attrib::System::refModelManager().)
// If the entity is not present, the returned smtk::model::EntityRef
// instances will be invalid and no type checking of attribute values
// can be performed.
//
// .SECTION See Also

#ifndef __smtk_attribute_ModelEntityItem_h
#define __smtk_attribute_ModelEntityItem_h

#include "smtk/CoreExports.h"
#include "smtk/attribute/Item.h"
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
class SMTKCORE_EXPORT ModelEntityItem : public Item
{
  friend class ModelEntityItemDefinition;

public:
  typedef smtk::model::EntityRefArray::const_iterator const_iterator;

  smtkTypeMacro(ModelEntityItem);
  virtual ~ModelEntityItem();
  virtual Item::Type type() const;
  virtual bool isValid() const;

  std::size_t numberOfValues() const;
  bool setNumberOfValues(std::size_t newSize);

  std::size_t numberOfRequiredValues() const;
  smtk::model::EntityRef value(std::size_t element = 0) const;
  bool setValue(const smtk::model::EntityRef& val);
  bool setValue(std::size_t element, const smtk::model::EntityRef& val);

  template <typename I>
  bool setValues(I vbegin, I vend, std::size_t offset = 0);
  template <typename I>
  bool appendValues(I vbegin, I vend);

  bool appendValue(const smtk::model::EntityRef& val);
  bool removeValue(std::size_t element);
  virtual void reset();
  virtual std::string valueAsString() const;
  virtual std::string valueAsString(std::size_t element) const;
  virtual bool isSet(std::size_t element = 0) const;
  virtual void unset(std::size_t element = 0);
  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occured.  By default, the model enity is assigned.
  // Use IGNORE_MODEL_ENTITIES option to prevent this (defined in Item.h).
  virtual bool assign(smtk::attribute::ConstItemPtr& sourceItem, unsigned int options = 0);

  bool isExtensible() const;

  bool has(const smtk::common::UUID& entity) const;
  bool has(const smtk::model::EntityRef& entity) const;

  const_iterator begin() const;
  const_iterator end() const;

  std::ptrdiff_t find(const smtk::common::UUID& entity) const;
  std::ptrdiff_t find(const smtk::model::EntityRef& entity) const;

protected:
  friend class Definition;

  ModelEntityItem(Attribute* owningAttribute, int itemPosition);
  ModelEntityItem(Item* owningItem, int myPosition, int mySubGroupPosition);

  virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def);

  smtk::model::EntityRefArray m_values;
};

template <typename I>
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
    this->setIsEnabled(num > 0 ? true : false);
  }
  return ok;
}

template <typename I>
bool ModelEntityItem::appendValues(I vbegin, I vend)
{
  return this->setValues(vbegin, vend, this->numberOfValues());
}

} // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ModelEntityItem_h */
