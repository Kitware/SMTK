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

template<class Type>
class WeakReferenceWrapper
{
  std::weak_ptr<Type> _weak_ptr;
  mutable Type* _cache = nullptr;

public:
  WeakReferenceWrapper() = default;

  WeakReferenceWrapper(const WeakReferenceWrapper& other)
    : _weak_ptr(other._weak_ptr)
    , _cache(nullptr)
  {
  }

  WeakReferenceWrapper(WeakReferenceWrapper&& other) noexcept
    : _weak_ptr(std::move(other._weak_ptr))
    , _cache(other._cache)
  {
    other._cache = nullptr;
  }

  WeakReferenceWrapper(Type& ref)
  {
    auto ptr = std::dynamic_pointer_cast<Type>(ref.shared_from_this());
    _weak_ptr = ptr;
    _cache = ptr.get();
  }

  WeakReferenceWrapper(std::weak_ptr<Type> ptr)
    : _weak_ptr(ptr)
    , _cache(nullptr)
  {
  }

  WeakReferenceWrapper& operator=(const WeakReferenceWrapper& other)
  {
    this->_weak_ptr = other._weak_ptr;
    return *this;
  }

  WeakReferenceWrapper& operator=(WeakReferenceWrapper&& other) noexcept
  {
    this->_weak_ptr = std::move(other._weak_ptr);
    this->_cache = other._cache;
    other._cache = nullptr;
    return *this;
  }

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

  operator Type&() const { return this->get(); }

  bool expired() const { return _weak_ptr.expired(); }
  size_t hash() const
  {
    // Set the cache
    if (!this->expired() && !_cache)
    {
      this->get();
    }
    return *((size_t*)&_cache);
  }

  bool operator==(const WeakReferenceWrapper& ref) const
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
};

template<class Type>
void weakRef(Type&&) = delete;

template<class Type>
WeakReferenceWrapper<Type> weakRef(Type& ref)
{
  return WeakReferenceWrapper<Type>(ref);
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
