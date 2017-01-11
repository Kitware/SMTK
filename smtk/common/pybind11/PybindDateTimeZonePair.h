//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind___smtk_common_DateTimeZonePair_h
#define pybind___smtk_common_DateTimeZonePair_h

#include <pybind11/pybind11.h>

#include "smtk/common/DateTimeZonePair.h"

#include "smtk/common/DateTime.h"
#include "smtk/common/TimeZone.h"

namespace py = pybind11;

py::class_< smtk::common::DateTimeZonePair > pybind11_init_smtk_common_DateTimeZonePair(py::module &m)
{
  py::class_< smtk::common::DateTimeZonePair > instance(m, "DateTimeZonePair");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::DateTimeZonePair const &>())
    .def("__eq__", (bool (smtk::common::DateTimeZonePair::*)(::smtk::common::DateTimeZonePair const &) const) &smtk::common::DateTimeZonePair::operator==)
    .def("__lt__", (bool (smtk::common::DateTimeZonePair::*)(::smtk::common::DateTimeZonePair const &) const) &smtk::common::DateTimeZonePair::operator<)
    .def("__gt__", (bool (smtk::common::DateTimeZonePair::*)(::smtk::common::DateTimeZonePair const &) const) &smtk::common::DateTimeZonePair::operator>)
    .def("deepcopy", (smtk::common::DateTimeZonePair & (smtk::common::DateTimeZonePair::*)(::smtk::common::DateTimeZonePair const &)) &smtk::common::DateTimeZonePair::operator=)
    .def("dateTime", &smtk::common::DateTimeZonePair::dateTime)
    .def("timeZone", &smtk::common::DateTimeZonePair::timeZone)
    .def("setDateTime", &smtk::common::DateTimeZonePair::setDateTime, py::arg("dt"))
    .def("setTimeZone", &smtk::common::DateTimeZonePair::setTimeZone, py::arg("tz"))
    .def("serialize", &smtk::common::DateTimeZonePair::serialize)
    .def("deserialize", &smtk::common::DateTimeZonePair::deserialize, py::arg("content"))
    ;
  return instance;
}

#endif
