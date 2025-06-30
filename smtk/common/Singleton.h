//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_Singleton_h
#define smtk_common_Singleton_h

namespace smtk
{
namespace common
{

/// This is a base class for objects that should have a single instance per process.
///
/// The template provides an `instance()` method for creating or fetching the instance.
/// This is distinct from the the Singletons (plural) class also in `smtk/common`, which
/// is a container that can hold a single instance of any given type for access as needed.
template<typename T>
class Singleton
{
public:
  typedef T Type;
  static typename T::Ptr& instance();

private:
  virtual ~Singleton() = default;
  explicit Singleton() = default;
};

template<typename T>
typename T::Ptr& Singleton<T>::instance()
{
  static typename T::Ptr instance = nullptr;
  if (instance == nullptr)
  {
    instance = T::create();
  }
  return instance;
}
} // namespace common
} // namespace smtk

#endif
