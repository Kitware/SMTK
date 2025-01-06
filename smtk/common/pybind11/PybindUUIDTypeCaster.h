//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_common_UUIDTypeCaster_h
#define pybind_common_UUIDTypeCaster_h

#include <pybind11/pybind11.h>

#include "smtk/common/UUID.h"

namespace pybind11
{
namespace detail
{
template <>
struct type_caster<smtk::common::UUID>
{
public:
  typedef smtk::common::UUID UUID;
  PYBIND11_TYPE_CASTER(UUID, _("UUID"));

  bool load(handle src, bool)
  {
    PyObject* uuidBytes = PyObject_GetAttrString(src.ptr(), "bytes");
    if (uuidBytes == nullptr)
    {
      // If uuidBytes is null, then the method PyObject_GetAttrString raises a
      // python exception about the field "bytes" not existing. We "catch" this
      // python exception (by simply resetting it) and throw a c++ pybind11
      // "reference_cast_error". When disambiguating multiple functions with
      // the same name and different signatures, this c++ exception is caught
      // and signals the need to try the next function.
      PyErr_Clear();
      throw reference_cast_error();
    }

#if PY_VERSION_HEX >= 0x03030000
    char* ustr = uuidBytes ? PyBytes_AS_STRING(uuidBytes) : nullptr;
#else
    char* ustr = uuidBytes ? PyString_AsString(uuidBytes) : nullptr;
#endif
    if (ustr)
    {
      for (int i = 0; i < 16; ++i)
        *((this->value).begin() + i) = ustr[i];
    }
    return !PyErr_Occurred();
  }

  static handle cast(const smtk::common::UUID& src,
                     return_value_policy /* policy */,
                     handle /* parent */)
  {
    // Get the uuid module object
    PyObject* module = PyImport_ImportModule("uuid");

    // Call the class inside the module to create an instance
    return PyObject_CallMethod(module, const_cast<char*>("UUID"), const_cast<char*>("(s)"),
                               const_cast<char*>(src.toString().c_str()));
  }
};
}
} // namespace pybind11::detail

#endif
