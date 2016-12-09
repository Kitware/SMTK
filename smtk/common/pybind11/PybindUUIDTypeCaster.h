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
    char* ustr = uuidBytes ? PyString_AsString(uuidBytes) : NULL;
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
    unsigned char* data = const_cast<unsigned char*>(src.begin());
    PyObject* byteStr =
      Py_BuildValue("s#", data, src.size());
    PyObject* none = Py_None;
    char v_uuid[]="UUID";
    char v_00[]="OO";
    PyObject* out = PyObject_CallMethod(module, v_uuid, v_00, none, byteStr);
    // FIXME: This hardly seems wise, but
    // http://docs.python.org/2/c-api/none.html
    // says Py_None is reference-counted:
    Py_DECREF(none);
    Py_DECREF(byteStr);
    return out;
  }
};
}
} // namespace pybind11::detail

#endif
