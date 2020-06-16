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

#include "smtk/common/UUID.h"
#include "smtk/resource/Lock.h"

#include <iterator>
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
class ReferenceItemDefinition;
class ValueItemDefinition;

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
  * + the virtual valueAsString() method this class declares; and
  * + a method to construct an instance and return a shared pointer to it.
  */
class SMTKCORE_EXPORT ReferenceItem : public Item
{
  using PersistentObjectPtr = smtk::resource::PersistentObjectPtr;

public:
  /**\brief An iterator for references held by a ReferenceItem.
  *
  * Iterators into ReferenceItem will always dereference to a
  * PersistentObjectPtr, regardless of the underlying storage mechanism used
  * to hold the reference.
  */
  class SMTKCORE_EXPORT const_iterator
  {
    friend class ReferenceItem;

  public:
    typedef const_iterator self_type;
    typedef std::random_access_iterator_tag iterator_category;
    typedef const smtk::resource::PersistentObjectPtr value_type;
    typedef value_type reference;
    typedef value_type pointer;
    typedef std::ptrdiff_t difference_type;

    const_iterator();
    const_iterator(const const_iterator& it);

    ~const_iterator();

    const_iterator& operator=(const const_iterator& it);

    const_iterator& operator++();
    const_iterator& operator--();

    const_iterator operator++(int);
    const_iterator operator--(int);

    const_iterator operator+(const difference_type& d) const;
    const_iterator operator-(const difference_type& d) const;

    reference operator*() const;
    pointer operator->() const;
    reference operator[](const difference_type& d);

    bool isSet() const;

    friend difference_type SMTKCORE_EXPORT operator-(const const_iterator&, const const_iterator&);

    friend bool SMTKCORE_EXPORT operator<(const const_iterator& it1, const const_iterator& it2);
    friend bool SMTKCORE_EXPORT operator>(const const_iterator& it1, const const_iterator& it2);
    friend bool SMTKCORE_EXPORT operator<=(const const_iterator& it1, const const_iterator& it2);
    friend bool SMTKCORE_EXPORT operator>=(const const_iterator& it1, const const_iterator& it2);
    friend bool SMTKCORE_EXPORT operator==(const const_iterator& it1, const const_iterator& it2);
    friend bool SMTKCORE_EXPORT operator!=(const const_iterator& it1, const const_iterator& it2);

  private:
    struct CacheIterator;
    std::unique_ptr<CacheIterator> m_cacheIterator;
  };

  /// A Key is a pair of UUIDs. the First UUID is the id of the resource link,
  /// and the second one is the id of the component link.
  using Key = std::pair<smtk::common::UUID, smtk::common::UUID>;

  smtkTypeMacro(ReferenceItem);
  smtkSuperclassMacro(Item);

  ReferenceItem(const ReferenceItem&);
  ~ReferenceItem() override;

  ReferenceItem& operator=(const ReferenceItem&);

  /// Indicate we are a reference to a persistent object.
  Item::Type type() const override { return Item::ReferenceType; }

  /// Return the size of the item (number of entities associated with the item).
  std::size_t numberOfValues() const;
  /// Set the number of entities to be associated with this item (returns true if permitted).
  bool setNumberOfValues(std::size_t newSize);
  ///\brief Remove all invalid references.
  ///
  /// Go through all values that are set and verify that they are still valid.
  /// If the value's resource is loaded in memory, the associated value is checked to
  /// see if it exists within the resource.  If it is not then it is removed.
  /// The method returns true if there were values removed.
  /// Note that if the resource is not loaded the values are left alone.
  bool removeInvalidValues();

  /// Return this item's definition.
  virtual std::shared_ptr<const ReferenceItemDefinition> definition() const;
  /// Return the association constraints for this item
  const std::multimap<std::string, std::string>& acceptableEntries() const;

  /// Return the number of values required by this item's definition (if it has one).
  std::size_t numberOfRequiredValues() const;
  /// Return the maximum number of values allowed by this item's definition (or 0).
  std::size_t maxNumberOfValues() const;

  /// Return true if the ReferenceItem contains a reference to the given object.
  bool contains(const smtk::resource::PersistentObjectPtr& obj) const;

  /// Return true if the component is contained in this item; false otherwise.
  bool contains(const smtk::common::UUID& compId) const;

  /**\brief Invoke a method on each value of this item.
    *
    * If the lambda returns false, iteration will terminate immediately.
    * Otherwise, iteration continues.
    */
  void visit(std::function<bool(const PersistentObjectPtr&)> visitor) const;

  /**\brief Populate a container of the given type with members of this item.
    *
    * Note that you can use the \a converter parameter to downcast
    * items as desired (and thus skip entries not of the downcast type).
    * For example:
    *
    *     ReferenceItemPtr modelEntityItem;
    *     std::set<smtk::model::EntityPtr> modelEntities;
    *     modelEntityItem->as(modelEntities,
    *       [](const PersistentObjectPtr& obj)
    *       { return std::dynamic_pointer_cast<smtk::model::Entity>(obj); });
    *
    * will populate the \a modelEntities set with only those persistent
    * objects that inherit smtk::model::Entity; other objects return a
    * nullptr when cast and this method skips values which evaluate to false.
    */
  template <typename Container>
  void as(Container& result,
    std::function<typename Container::value_type(const PersistentObjectPtr&)> converter = [](
      const PersistentObjectPtr& obj) { return obj; }) const
  {
    for (auto it = this->begin(); it != this->end(); ++it)
    {
      if (it.isSet() == false)
      {
        continue;
      }

      typename Container::value_type val = converter(*it);
      if (val)
      {
        result.insert(result.end(), val);
      }
    }
  }
  template <typename Container>
  Container as(std::function<typename Container::value_type(const PersistentObjectPtr&)> converter =
                 [](const PersistentObjectPtr& obj) { return obj; }) const
  {
    Container result;
    this->as(result, converter);
    return result;
  }

  /// Set/get object key (used for serialization).
  Key objectKey(std::size_t i = 0) const;
  bool setObjectKey(std::size_t i, const Key& key);

  /// Return the \a i-th object stored in this item.
  PersistentObjectPtr value(std::size_t i = 0) const;
  template <typename T>
  typename T::Ptr valueAs(std::size_t i = 0) const
  {
    return std::dynamic_pointer_cast<T>(this->value(i));
  }
  /** Return the \a i-th object stored in this item.
    * \deprecated This method will go away in future versions of SMTK
    * See instead value(std::size_t)
    */
  [[deprecated(
    "ReferenceItem::objectValue has been replaced with ReferenceItem::value")]] PersistentObjectPtr
  objectValue(std::size_t i = 0) const;

  virtual bool isValueValid(std::size_t ii, const PersistentObjectPtr& entity) const;
  bool isValueValid(const PersistentObjectPtr& entity) const
  {
    return this->isValueValid(0, entity);
  }

  /**\brief Set the component stored with this item.
    *
    * This always sets the 0-th item and is a convenience method
    * for cases where only 1 value is needed.
    */
  bool setValue(const PersistentObjectPtr& val);
  /** Set the \a i-th value to the given item. This method does no checking to see if \a i is valid.
    * bool setObjectValue(std::size_t i, const PersistentObjectPtr& val);
    * Return the \a i-th object stored in this item.
    */
  bool setValue(std::size_t i, const PersistentObjectPtr& val);
  /**\brief Set the component stored with this item.
    *
    * This always sets the 0-th item and is a convenience method
    * for cases where only 1 value is needed.
    * \deprecated This method will go away in future versions of SMTK
    * See instead setValue()
    */
  [[deprecated(
    "ReferenceItem::setObjectValue has been replaced with ReferenceItem::setValue")]] bool
  setObjectValue(const PersistentObjectPtr& val)
  {
    return this - setValue(val);
  }
  /** Set the \a i-th value to the given item. This method does no checking to see if \a i is valid.
    * bool setObjectValue(std::size_t i, const PersistentObjectPtr& val);
    * Return the \a i-th object stored in this item.
    * \deprecated This method will go away in future versions of SMTK
    * See instead setValue(std::size_t, const PersistentObjectPtr&)
    */
  [[deprecated(
    "ReferenceItem::setObjectValue has been replaced with ReferenceItem::setValue")]] bool
  setObjectValue(std::size_t i, const PersistentObjectPtr& val)
  {
    return this->setValue(i, val);
  }
  [[deprecated(
    "ReferenceItem::appendObjectValue has been replaced with ReferenceItem::appendValue")]] bool
  appendObjectValue(const PersistentObjectPtr& val)
  {
    return this->appendValue(val);
  }

  template <typename I>
  bool setValues(I vbegin, I vend, typename std::iterator_traits<I>::difference_type offset = 0);
  template <typename I>
  bool appendValues(I vbegin, I vend);

  template <typename I>
  [[deprecated(
    "ReferenceItem::setObjectValues has been replaced with ReferenceItem::setValues")]] bool
  setObjectValues(I vbegin, I vend, typename std::iterator_traits<I>::difference_type offset = 0)
  {
    return this->setValues<I>(vbegin, vend, offset);
  }

  template <typename I>
  [[deprecated(
    "ReferenceItem::appendObjectValues has been replaced with ReferenceItem::appendValues")]] bool
  appendObjectValues(I vbegin, I vend)
  {
    return this->appendValues<I>(vbegin, vend);
  }

  template <typename I, typename T>
  bool setValuesVia(I vbegin, I vend, const T& converter,
    typename std::iterator_traits<I>::difference_type offset = 0);
  template <typename I, typename T>
  bool appendValuesVia(I vbegin, I vend, const T& converter);

  /**\brief Add \a val if it is allowed and \a val is not already present in the item.
    *
    * This will **not** enable the item if it is disabled.
    *
    * This will **not** always add \a val to the end of the item's array;
    * if there is an unset value anywhere in the allocated array, that will
    * be preferred to reallocation.
    */
  bool appendValue(const PersistentObjectPtr& val);
  /**\brief Remove the value at the \a i-th location.
    *
    * If the number of values may not be changed, then the \a i-th
    * array entry is set to nullptr. Otherwise, the value is erased
    * from the array (reducing the number of values stored by 1).
    */
  bool removeValue(std::size_t i);
  /// Release the item's dependency on its parent attribute's Resource.
  void detachOwningResource() override;
  /// Clear the list of values and fill it with null entries up to the number of required values.
  void reset() override;
  /// A convenience method to obtain the first value in the item as a string.
  virtual std::string valueAsString() const;
  /**\brief Return the value of the \a i-th component as a string.
    *
    * This returns a string of the form "[" {UUID} "," {UUID} "]" where
    * the first UUID is the component's resource and the second UUID is
    * the component's.
    */
  virtual std::string valueAsString(std::size_t i) const;
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
  std::ptrdiff_t find(const PersistentObjectPtr& component) const;

  // Returns Read/Write/DoNotLock for read locking, write locking, or bypassing
  // locks.
  smtk::resource::LockType lockType() const;

protected:
  friend class ReferenceItemDefinition;
  friend class ValueItemDefinition;
  friend class Definition;

  /// Construct an item given its owning attribute and location in the attribute.
  ReferenceItem(Attribute* owningAttribute, int itemPosition);
  /// Construct an item given its owning item and position inside the item.
  ReferenceItem(Item* owningItem, int myPosition, int mySubGroupPosition);

  /// Set the definition of this attribute.
  bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def) override;

  /// Return the object stored in this item associated with \a key.
  PersistentObjectPtr value(const ReferenceItem::Key& key) const;

  /// Resolve the object pointers by accessing them using their associated keys.
  /// Return true if all object pointers were successfully resolved.
  bool resolve() const;

  /// Construct a link between the attribute that owns this item and \a val.
  Key linkTo(const PersistentObjectPtr& val);

  bool isValidInternal(bool useCategories, const std::set<std::string>& categories) const override;

  std::vector<Key> m_keys;
  /// In order to clean up its links when being deleted the item needs to track its
  /// referencing attribute.  During deletion, the attribute() call may return nullptr
  /// not because the owning attribute is being deleted but because the owning item is
  /// being deleted.
  smtk::attribute::WeakAttributePtr m_referencedAttribute;

private:
  /// To make it easier to catch errors associated with dereferencing unset
  /// reference item entries, ReferenceItem's const_iterator throws when it is
  /// dereferenced and the value under iteration is unset. Since we expose
  /// access methods that are templated over the iterator type, we separate the
  /// logic for testing if an iterator is valid into its own template with a
  /// specialization for ReferenceItem::const_iterator.
  template <typename I>
  bool iteratorIsSet(const I& iterator) const;

  void assignToCache(std::size_t i, const PersistentObjectPtr& obj) const;
  void appendToCache(const PersistentObjectPtr& obj) const;

  struct Cache;
  mutable std::unique_ptr<Cache> m_cache;
};

template <>
SMTKCORE_EXPORT bool ReferenceItem::iteratorIsSet<ReferenceItem::const_iterator>(
  const ReferenceItem::const_iterator& iterator) const;

template <typename I>
bool ReferenceItem::setValues(
  I vbegin, I vend, typename std::iterator_traits<I>::difference_type offset)
{
  bool ok = false;
  std::size_t num = std::distance(vbegin, vend) + offset;
  if (this->setNumberOfValues(num))
  {
    ok = true;
    std::size_t i = 0;
    for (I it = vbegin; it != vend; ++it, ++i)
    {
      if (iteratorIsSet(it) == false)
      {
        continue;
      }

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
bool ReferenceItem::appendValues(I vbegin, I vend)
{
  return this->setValues(vbegin, vend, this->numberOfValues());
}

template <typename I, typename T>
bool ReferenceItem::setValuesVia(
  I vbegin, I vend, const T& converter, typename std::iterator_traits<I>::difference_type offset)
{
  bool ok = false;
  std::size_t num = std::distance(vbegin, vend) + offset;
  if (this->setNumberOfValues(num))
  {
    ok = true;
    std::size_t i = 0;
    for (I it = vbegin; it != vend; ++it, ++i)
    {
      if (iteratorIsSet(it) == false)
      {
        continue;
      }

      if (!this->setValue(offset + i, converter(*it)))
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

template <typename I, typename T>
bool ReferenceItem::appendValuesVia(I vbegin, I vend, const T& converter)
{
  return this->setValuesVia(vbegin, vend, converter, this->numberOfValues());
}

template <typename I>
bool ReferenceItem::iteratorIsSet(const I& iterator) const
{
  return !!(*iterator);
}

} // namespace attribute
} // namespace smtk

#endif
