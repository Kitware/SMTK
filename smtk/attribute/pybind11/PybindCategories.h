//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_Categories_h
#define pybind_smtk_attribute_Categories_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/Categories.h"

namespace py = pybind11;

py::class_< smtk::attribute::Categories > pybind11_init_smtk_attribute_Categories(py::module &m)
{
  py::class_< smtk::attribute::Categories > instance(m, "Categories");
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::attribute::Categories & (smtk::attribute::Categories::*)(::smtk::attribute::Categories const &)) &smtk::attribute::Categories::operator=)
    .def("passes", (bool (smtk::attribute::Categories::*)(const ::std::set<::std::string>&) const) &smtk::attribute::Categories::passes, py::arg("categories"))
    .def("passes", (bool (smtk::attribute::Categories::*)(const ::std::string&) const) &smtk::attribute::Categories::passes, py::arg("category"))
    .def("insert", (void (smtk::attribute::Categories::*)(const smtk::attribute::Categories&)) &smtk::attribute::Categories::insert, py::arg("categories"))
    .def("insert", (void (smtk::attribute::Categories::*)(const smtk::attribute::Categories::Set&)) &smtk::attribute::Categories::insert, py::arg("categorySet"))
    .def("reset", &smtk::attribute::Categories::reset)
    .def("size", &smtk::attribute::Categories::size)
    // NOTE that the Python form of this method is returning a copy since Python
    // doesn't support const references
    .def("sets", &smtk::attribute::Categories::sets)
    .def("categoryNames", &smtk::attribute::Categories::categoryNames)
    ;
  py::class_< smtk::attribute::Categories::Set >(instance, "Set")
    .def(py::init<>())
    .def("deepcopy", (smtk::attribute::Categories::Set & (smtk::attribute::Categories::Set::*)(::smtk::attribute::Categories::Set const &)) &smtk::attribute::Categories::Set::operator=)
    .def("mode", &smtk::attribute::Categories::Set::mode)
    .def("setMode", &smtk::attribute::Categories::Set::setMode)
    .def("categoryNames", &smtk::attribute::Categories::Set::categoryNames)
    .def("set", &smtk::attribute::Categories::Set::set)
    .def("insert", &smtk::attribute::Categories::Set::insert)
    .def("erase", &smtk::attribute::Categories::Set::erase)
    .def("empty", &smtk::attribute::Categories::Set::empty)
    .def("reset", &smtk::attribute::Categories::Set::reset)
    .def("size", &smtk::attribute::Categories::Set::size)
    .def("passes", (bool (smtk::attribute::Categories::Set::*)(const ::std::set<::std::string>&) const) &smtk::attribute::Categories::Set::passes, py::arg("categories"))
    .def("passes", (bool (smtk::attribute::Categories::Set::*)(const ::std::string&) const) &smtk::attribute::Categories::Set::passes, py::arg("category"))
    ;
  py::enum_<smtk::attribute::Categories::Set::CombinationMode>(instance, "CombinationMode")
    .value("Any", smtk::attribute::Categories::Set::CombinationMode::Any)
    .value("All", smtk::attribute::Categories::Set::CombinationMode::All)
    .export_values();
  return instance;
}

#endif
