//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_pybind_ScopedRawPointer_h
#define smtk_common_pybind_ScopedRawPointer_h

#include <pybind11/pybind11.h>

namespace smtk
{
namespace common
{

/// Hold references to a raw pointer scoped by a shared-pointer to its owner.
///
/// Example: The task::Manager owns taskInstances and adaptorInstances objects.
/// In its C++ API, these objects are returned by reference which cannot be
/// wrapped. (These objects should never be passed by value and the scope of
/// the returned references is unclear to pybind11.)
/// To handle this, we generate python bindings that return custom "holder"
/// objects of type `ScopedRawPointer<task::Manager::TaskInstances,task::Manager>`.
/// A shared pointer to the task::Manager is held so that the raw pointer to the
/// TaskInstances object can always be safely returned.
///
/// To use this class from your pybind module, make sure to declare the
/// holder type using pybind's holder-type macro like so:
/// ```c++
/// PYBIND11_DECLARE_HOLDER_TYPE(Object, PYBIND11_TYPE(ScopedRawPointer<Object, ScopeOwner>));
/// ```
/// being careful to replace `ScopeOwner` with your module's owner type.
/// Note the use of `PYBIND11_TYPE` to prevent the multiple template
/// parameters that ScopedRawPointer takes from confusing the
/// `PYBIND11_DECLARE_HOLDER_TYPE` macro (which cannot discriminate between
/// commas separating its arguments from those separating template parameters).
template<typename Object, typename ScopeOwner>
class ScopedRawPointer
{
public:
  ScopedRawPointer(Object& object, const std::shared_ptr<ScopeOwner>& owner)
    : m_owner(owner)
    , m_object(&object)
  {
  }

  ScopedRawPointer(Object* object, const std::shared_ptr<ScopeOwner>& owner)
    : m_owner(owner)
    , m_object(object)
  {
  }

  ScopedRawPointer(const ScopedRawPointer& other) = default;

  Object* get() const { return m_object; }

protected:
  std::shared_ptr<ScopeOwner> m_owner;
  Object* m_object = nullptr;
};

} // namespace common
} // namespace smtk

#endif // smtk_common_pybind_ScopedRawPointer_h
