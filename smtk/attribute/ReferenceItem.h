//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_ReferenceItem_h
#define smtk_attribute_ReferenceItem_h

#include "smtk/attribute/Item.h"

#include "smtk/resource/Lock.h"

#include <vector>

namespace smtk
{
namespace common
{
class UUID;
}
namespace attribute
{

class Attribute;
template <typename T>
class ReferenceItemDefinition;

/**\brief Hold associations that link resources or components as an attribute value.
  *
  * An attribute whose value is a resource or component (such as a mesh set,
  * model entity, or even an attribute).
  *
  * While the actual value stored is a pointer to the linked object, only the
  * UUID(s) of the object are saved when the attribute is serialized.
  * Then, when deserialized, the application's resource manager
  * is used to find a pointer to the deserialized object given its UUID(s).
  * If the entity is not present, the returned pointer will be null and
  * no type checking of attribute values can be performed.
  *
  * Anything inheriting this class must implement
  * + the type() method;
  * + the pure virtual valueAsString() method this class declares; and
  * + a method to construct an instance and return a shared pointer to it.
  */
template <typename T>
class ReferenceItem : public Item
{
public:
  typedef typename std::vector<typename T::Ptr>::const_iterator const_iterator;
  smtkTypeMacro(ReferenceItem<T>);
  smtkSuperclassMacro(Item);
  /// Destructor
  ~ReferenceItem() override;
  /**\brief Return whether the item is in a valid state.
    *
    * If the item is not enabled or if all of its values are set then it is valid.
    * If it is enabled and contains unset values then it is invalid.
    */
  bool isValid() const override;

  /// Return the size of the item (number of entities associated with the item).
  std::size_t numberOfValues() const;
  /// Set the number of entities to be associated with this item (returns true if permitted).
  bool setNumberOfValues(std::size_t newSize);
  /// Return this item's definition.
  virtual std::shared_ptr<const ReferenceItemDefinition<T> > definition() const;

  /// Return the number of values required by this item's definition (if it has one).
  std::size_t numberOfRequiredValues() const;
  /// Return the maximum number of values allowed by this item's definition (or 0).
  std::size_t maxNumberOfValues() const;
  /// Return the \a i-th component stored in this item.
  typename T::Ptr value(std::size_t i = 0) const;
  /**\brief Set the component stored with this item.
    *
    * This always sets the 0-th item and is a convenience method
    * for cases where only 1 value is needed.
    */
  bool setValue(typename T::Ptr val);
  /// Set the \a i-th value to the given item. This method does no checking to see if \a i is valid.
  bool setValue(std::size_t i, typename T::Ptr val);

  template <typename I>
  bool setValues(I vbegin, I vend, std::size_t offset = 0);
  template <typename I>
  bool appendValues(I vbegin, I vend);

  /**\brief Add \a val if it is allowed and \a val is not already present in the item.
    *
    * This will **not** enable the item if it is disabled.
    *
    * This will **not** always add \a val to the end of the item's array;
    * if there is an unset value anywhere in the allocated array, that will
    * be preferred to reallocation.
    */
  bool appendValue(typename T::Ptr val);
  /**\brief Remove the value at the \a i-th location.
    *
    * If the number of values may not be changed, then the \a i-th
    * array entry is set to nullptr. Otherwise, the value is erased
    * from the array (reducing the number of values stored by 1).
    */
  bool removeValue(std::size_t i);
  /// Clear the list of values and fill it with null entries up to the number of required values.
  void reset() override;
  /// A convenience method to obtain the first value in the item as a string.
  virtual std::string valueAsString() const;
  /**\brief Return the value of the \a i-th component as a string.
    *
    * This returns a string of the form "[" <UUID> "," <UUID> "]" where
    * the first UUID is the component's resource and the second UUID is
    * the component's.
    */
  virtual std::string valueAsString(std::size_t i) const = 0;
  /**\brief Return whether the \a i-th value is set.
    *
    * This returns true when the item and its UUID are non-NULL and false otherwise.
    *
    * Note that this is **not always what you would expect**!
    * You can set a value to be an invalid, non-NULL UUID so that
    * entities which have been expunged can be reported (and other
    * use cases).
    */
  virtual bool isSet(std::size_t i = 0) const;
  /// Force the \a i-th value of the item to be invalid.
  virtual void unset(std::size_t i = 0);
  /**\brief Assigns contents to be same as source item
    *
    * Assigns this item to be equivalent to another.
    * Returns true if success and false if a problem occured.
    */
  bool assign(smtk::attribute::ConstItemPtr& sourceItem, unsigned int options = 0) override;

  /// A convenience method returning whether the item's definition is extensible.
  bool isExtensible() const;

  /// Return true if the component is contained in this item; false otherwise.
  bool has(const smtk::common::UUID& compId) const;
  /// Return true if \a comp is contained in this item; false otherwise.
  bool has(typename T::Ptr comp) const;

  /**\brief Return an iterator to the first model-entity value in this item.
    *
    */
  const_iterator begin() const;
  /**\brief Return an iterator just past the last model-entity value in this item.
    *
    */
  const_iterator end() const;

  /**\brief Return the index of the first component with the given \a compId.
    *
    */
  std::ptrdiff_t find(const smtk::common::UUID& compId) const;
  /**\brief Return the index of the given \a component.
    *
    */
  std::ptrdiff_t find(typename T::Ptr component) const;

  // Returns Read/Rrite/DoNotLock for read locking, write locking, or bypassing
  // locks.
  smtk::resource::LockType lockType() const;

protected:
  friend class ReferenceItemDefinition<T>;
  friend class Definition;

  /// Construct an item given its owning attribute and location in the attribute.
  ReferenceItem(Attribute* owningAttribute, int itemPosition);
  /// Construct an item given its owning item and position inside the item.
  ReferenceItem(Item* owningItem, int myPosition, int mySubGroupPosition);

  /// Set the definition of this attribute.
  bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def) override;

  std::vector<typename T::Ptr> m_values;
};

template <typename T>
template <typename I>
bool ReferenceItem<T>::setValues(I vbegin, I vend, std::size_t offset)
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

template <typename T>
template <typename I>
bool ReferenceItem<T>::appendValues(I vbegin, I vend)
{
  return this->setValues(vbegin, vend, this->numberOfValues());
}

} // namespace attribute
} // namespace smtk

#endif
