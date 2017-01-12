//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind___smtk_common_TimeZone_h
#define pybind___smtk_common_TimeZone_h

#include <pybind11/pybind11.h>

#include "smtk/common/TimeZone.h"

namespace py = pybind11;

py::class_< smtk::common::TimeZone > pybind11_init_smtk_common_TimeZone(py::module &m)
{
  py::class_< smtk::common::TimeZone > instance(m, "TimeZone");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::TimeZone const &>())
    .def("deepcopy", (smtk::common::TimeZone & (smtk::common::TimeZone::*)(::smtk::common::TimeZone const &)) &smtk::common::TimeZone::operator=)
    .def("isSet", &smtk::common::TimeZone::isSet)
    .def("setUTC", &smtk::common::TimeZone::setUTC)
    .def("isUTC", &smtk::common::TimeZone::isUTC)
    .def("setRegion", &smtk::common::TimeZone::setRegion, py::arg("region"))
    .def("region", &smtk::common::TimeZone::region)
    .def("setPosixString", &smtk::common::TimeZone::setPosixString, py::arg("posixTimeZoneString"))
    .def("posixString", &smtk::common::TimeZone::posixString)
    .def("stdZoneName", &smtk::common::TimeZone::stdZoneName)
    .def("stdZoneAbbreviation", &smtk::common::TimeZone::stdZoneAbbreviation)
    .def("dstZoneName", &smtk::common::TimeZone::dstZoneName)
    .def("dstZoneAbbreviation", &smtk::common::TimeZone::dstZoneAbbreviation)
    .def("hasDST", &smtk::common::TimeZone::hasDST)
    .def("utcOffset", &smtk::common::TimeZone::utcOffset, py::arg("hours"), py::arg("minutes"))
    .def("dstShift", &smtk::common::TimeZone::dstShift, py::arg("hours"), py::arg("minutes"))
    .def("boostPointer", &smtk::common::TimeZone::boostPointer)
    ;
  return instance;
}

#endif
