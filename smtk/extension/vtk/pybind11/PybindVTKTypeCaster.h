//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_extension_vtk_source_VTKTypeCaster_h
#define pybind_extension_vtk_source_VTKTypeCaster_h

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>

#include <type_traits>

#include "vtkObjectBase.h"
#include "vtkPythonUtil.h"

#define PYBIND11_VTK_TYPECASTER(VTK_OBJ)                                \
  namespace pybind11 { namespace detail {                               \
  template <> struct type_caster<VTK_OBJ> {                             \
protected:                                                              \
VTK_OBJ* value;                                                         \
public:                                                                 \
static constexpr auto name = _(#VTK_OBJ);                               \
template <typename T_,                                                  \
          enable_if_t<std::is_same<VTK_OBJ,                             \
                                   remove_cv_t<T_>>::value, int> = 0>   \
static handle cast(T_ *src, return_value_policy policy, handle parent)  \
 {                                                                      \
  if (!src) return none().release();                                    \
  if (policy == return_value_policy::take_ownership) {                  \
    auto h = cast(std::move(*src), policy, parent);                     \
    delete src; return h;                                               \
  } else {                                                              \
    return cast(*src, policy, parent);                                  \
  }                                                                     \
}                                                                       \
operator VTK_OBJ*() { return value; }                                   \
operator VTK_OBJ&() { return *value; }                                  \
operator VTK_OBJ&&() && { return std::move(*value); }                   \
template <typename T_>                                                  \
using cast_op_type = pybind11::detail::movable_cast_op_type<T_>;        \
bool load(handle src, bool) {                                           \
  value = dynamic_cast< VTK_OBJ *>(                                     \
    vtkPythonUtil::GetPointerFromObject(src.ptr(), #VTK_OBJ));          \
  if (!value) {                                                         \
    PyErr_Clear();                                                      \
    throw reference_cast_error();                                       \
  }                                                                     \
  return value != nullptr;                                              \
}                                                                       \
static handle cast(const VTK_OBJ& src, return_value_policy, handle) {   \
  return vtkPythonUtil::GetObjectFromPointer(                           \
    const_cast< VTK_OBJ *>(&src));                                      \
}                                                                       \
};                                                                      \
}}

#endif
