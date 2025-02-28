//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_WeakRefernece_h
#define smtk_WeakRefernece_h

#include <memory>

namespace smtk
{

/// Reference wrapper for handling weak references in a similar way as
/// std::reference_wrapper.
///
/// Requirements for \a Type are it must be derived from
/// std::enable_shared_from_this<Type>.
template<class Type>
class WeakReferenceWrapper
{
  std::weak_ptr<Type> _weak_ptr;
  mutable Type* _cache = nullptr;

  using U = typename std::remove_reference<typename std::remove_cv<Type>::type>::type;

public:
  WeakReferenceWrapper() = default;

  /// Copy Construct.
  WeakReferenceWrapper(const WeakReferenceWrapper& other)
    : _weak_ptr(other._weak_ptr)
    , _cache(other._cache)
  {
  }

  /// Move Construct.
  WeakReferenceWrapper(WeakReferenceWrapper&& other) noexcept
    : _weak_ptr(std::move(other._weak_ptr))
    , _cache(other._cache)
  {
    other._cache = nullptr;
  }

  /// Construct from a refernece.
  WeakReferenceWrapper(Type&& ref) = delete;

  WeakReferenceWrapper(Type& ref)
  {
    auto ptr = std::dynamic_pointer_cast<Type>(ref.shared_from_this());
    _weak_ptr = ptr;
    _cache = ptr.get();
  }

  /// Construct from a weak pointer.
  WeakReferenceWrapper(std::weak_ptr<Type> ptr)
    : _weak_ptr(ptr)
    , _cache(nullptr)
  {
  }

  /// Copy assignment.
  WeakReferenceWrapper& operator=(const WeakReferenceWrapper& other)
  {
    this->_weak_ptr = other._weak_ptr;
    return *this;
  }

  /// Move assignment. This also handles assignment from weak_ptr and reference implicitly.
  WeakReferenceWrapper& operator=(WeakReferenceWrapper&& other) noexcept
  {
    this->_weak_ptr = std::move(other._weak_ptr);
    this->_cache = other._cache;
    other._cache = nullptr;
    return *this;
  }

  /// The the referenced type. Throws an exception if the reference expired.
  Type& get() const
  {
    if (!_weak_ptr.expired())
    {
      if (!_cache)
      {
        _cache = _weak_ptr.lock().get();
      }
      return *_cache;
    }
    else
    {
      throw std::bad_weak_ptr();
    }
  }

  /// Implicit conversion to referenced type. Throws an exception if the reference expired.
  operator Type&() const { return this->get(); }

  /// Implicit conversion to bool
  operator bool() const noexcept { return !this->expired(); }

  /// Check if the reference is still valid
  bool expired() const { return _weak_ptr.expired(); }

  /// Reset the refernece/make it expired
  void reset()
  {
    // Do not reset the _cache variable since it is used for the
    // hash which must remain consistent. Comparison handles
    // case where the hash is the same but one/both referneces are
    // expired.
    _weak_ptr.reset();
  }

  /// Compute a consistent hash of the reference
  size_t hash() const
  {
    // Set the cache
    if (!this->expired() && !_cache)
    {
      this->get();
    }
    // Use this union to avoid strict-aliasing warnings about type-punned pointers:
    union
    {
      Type* const* cache_ptr;
      const size_t* hash_ptr;
    } data;
    data.cache_ptr = &_cache;
    return *data.hash_ptr;
  }

  /// Compare if references refernece the same data in memory
  template<class U>
  bool operator==(const WeakReferenceWrapper<U>& ref) const
  {
    // Expired references are all the same
    if (this->expired() && ref.expired())
    {
      return true;
    }
    // Don't fail a comparison if only one reference is expired
    // They are just not equal
    else if (this->expired() || ref.expired())
    {
      return false;
    }
    // Compared the things being referenced
    else
    {
      return _cache == ref._cache;
    }
  }

  template<class U>
  friend class WeakReferenceWrapper;
};

/// Explicitly disable creating WeakReferenceWrappers from rvalues
template<class Type>
void weakRef(Type&&) = delete;

/// Helper function for creating WeakReferenceWrappers from \a Type reference.
template<class Type, class U = typename std::remove_reference<Type>::type>
WeakReferenceWrapper<U> weakRef(Type& ref)
{
  return WeakReferenceWrapper<U>(ref);
}

template<class Type, class U = typename std::remove_reference<Type>::type>
WeakReferenceWrapper<U> weakCRef(Type& ref)
{
  return WeakReferenceWrapper<U>(ref);
}

} // namespace smtk

namespace std
{

template<class Type>
struct hash<smtk::WeakReferenceWrapper<Type>>
{
  using argument_type = smtk::WeakReferenceWrapper<Type>;
  using result_type = std::size_t;

  result_type operator()(const argument_type& ref) const { return ref.hash(); }
};
} // namespace std

#endif // smtk_WeakRefernece_h
