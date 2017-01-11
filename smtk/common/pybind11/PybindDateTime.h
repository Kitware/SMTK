//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind___smtk_common_DateTime_h
#define pybind___smtk_common_DateTime_h

#include <pybind11/pybind11.h>

#include "smtk/common/DateTime.h"
#include "smtk/common/TimeZone.h"

namespace py = pybind11;

py::class_< smtk::common::DateTime > pybind11_init_smtk_common_DateTime(py::module &m)
{
  py::class_< smtk::common::DateTime > instance(m, "DateTime");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::DateTime const &>())
    .def("__eq__", (bool (smtk::common::DateTime::*)(::smtk::common::DateTime const &) const) &smtk::common::DateTime::operator==)
    .def("__lt__", (bool (smtk::common::DateTime::*)(::smtk::common::DateTime const &) const) &smtk::common::DateTime::operator<)
    .def("__gt__", (bool (smtk::common::DateTime::*)(::smtk::common::DateTime const &) const) &smtk::common::DateTime::operator>)
    .def("deepcopy", (smtk::common::DateTime & (smtk::common::DateTime::*)(::smtk::common::DateTime const &)) &smtk::common::DateTime::operator=)
    .def("setComponents", (bool (smtk::common::DateTime::*)(::smtk::common::TimeZone, int, int, int, int, int, int, int)) &smtk::common::DateTime::setComponents, py::arg("timeZone"), py::arg("year"), py::arg("month") = 1, py::arg("day") = 1, py::arg("hour") = 0, py::arg("minute") = 0, py::arg("second") = 0, py::arg("millisecond") = 0)
    .def("setComponents", (bool (smtk::common::DateTime::*)(int, int, int, int, int, int, int)) &smtk::common::DateTime::setComponents, py::arg("year"), py::arg("month") = 1, py::arg("day") = 1, py::arg("hour") = 0, py::arg("minute") = 0, py::arg("second") = 0, py::arg("millisecond") = 0)
    //.def("components", (bool (smtk::common::DateTime::*)(::smtk::common::TimeZone, int &, int &, int &, int &, int &, int &, int &) const) &smtk::common::DateTime::components, py::arg("timeZone"), py::arg("year"), py::arg("month"), py::arg("day"), py::arg("hour"), py::arg("minute"), py::arg("second"), py::arg("millisecond"))
    //.def("components", (bool (smtk::common::DateTime::*)(int &, int &, int &, int &, int &, int &, int &) const) &smtk::common::DateTime::components, py::arg("year"), py::arg("month"), py::arg("day"), py::arg("hour"), py::arg("minute"), py::arg("second"), py::arg("millisecond"))
    .def("isSet", &smtk::common::DateTime::isSet)
    .def("deserialize", &smtk::common::DateTime::deserialize, py::arg("ts"))
    .def("serialize", &smtk::common::DateTime::serialize)
    .def("parseBoostFormat", &smtk::common::DateTime::parseBoostFormat, py::arg("ts"))

    // Modified methods
    .def("components",
         [](const smtk::common::DateTime &dt) {
         int yr=-1, month=-1, day=-1, hour=-1, minute=-1, sec=-1, msec=-1;
         dt.components(yr, month, day, hour, minute, sec, msec);
         return std::make_tuple(yr, month, day, hour, minute, sec, msec);
         })
    .def("components",
         [](const smtk::common::DateTime &dt, smtk::common::TimeZone tz) {
         int yr=-1, month=-1, day=-1, hour=-1, minute=-1, sec=-1, msec=-1;
         dt.components(tz, yr, month, day, hour, minute, sec, msec);
         return std::make_tuple(yr, month, day, hour, minute, sec, msec);
         })
    ;
  return instance;
}

#endif
