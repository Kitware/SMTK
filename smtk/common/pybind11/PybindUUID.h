//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_UUID_h
#define pybind_smtk_common_UUID_h

#include <pybind11/pybind11.h>

#include "smtk/common/UUID.h"

#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

inline py::class_< smtk::common::UUID > pybind11_init_smtk_common_UUID(py::module &m)
{
  py::class_< smtk::common::UUID > instance(m, "UUID");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::UUID const &>())
    .def(py::init<::smtk::common::UUID::const_iterator, ::smtk::common::UUID::const_iterator>())
    .def(py::init<::std::string const &>())
    .def(py::init<::boost::uuids::uuid const &>())
    .def("__ne__", (bool (smtk::common::UUID::*)(::smtk::common::UUID const &) const) &smtk::common::UUID::operator!=)
    .def("__eq__", (bool (smtk::common::UUID::*)(::smtk::common::UUID const &) const) &smtk::common::UUID::operator==)
    .def("__lt__", (bool (smtk::common::UUID::*)(::smtk::common::UUID const &) const) &smtk::common::UUID::operator<)
    .def("__hash__", [](const smtk::common::UUID& uid) { return uid.hash(); })
    .def("__repr__", [](const smtk::common::UUID& uid) { return "UUID('" + uid.toString() + "')"; })
    .def("deepcopy", (smtk::common::UUID & (smtk::common::UUID::*)(::smtk::common::UUID const &)) &smtk::common::UUID::operator=)
    .def_static("random", &smtk::common::UUID::random)
    .def_static("null", &smtk::common::UUID::null)
    .def_static("size", &smtk::common::UUID::size)
    .def("isNull", &smtk::common::UUID::isNull)
    .def("begin", (smtk::common::UUID::iterator (smtk::common::UUID::*)()) &smtk::common::UUID::begin)
    .def("begin", (smtk::common::UUID::const_iterator (smtk::common::UUID::*)() const) &smtk::common::UUID::begin)
    .def("end", (smtk::common::UUID::iterator (smtk::common::UUID::*)()) &smtk::common::UUID::end)
    .def("end", (smtk::common::UUID::const_iterator (smtk::common::UUID::*)() const) &smtk::common::UUID::end)
    .def("toString", &smtk::common::UUID::toString)
    .def("hash", &smtk::common::UUID::hash)
    ;
  return instance;
}

#endif
