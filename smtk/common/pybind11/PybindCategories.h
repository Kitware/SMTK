//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_Categories_h
#define pybind_smtk_common_Categories_h

#include <pybind11/pybind11.h>

#include "smtk/common/Categories.h"

namespace py = pybind11;

inline py::class_< smtk::common::Categories > pybind11_init_smtk_common_Categories(py::module &m)
{
  py::class_< smtk::common::Categories > instance(m, "Categories");
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::common::Categories & (smtk::common::Categories::*)(::smtk::common::Categories const &)) &smtk::common::Categories::operator=)
    .def("passes", (bool (smtk::common::Categories::*)(const ::std::set<::std::string>&) const) &smtk::common::Categories::passes, py::arg("categories"))
    .def("passes", (bool (smtk::common::Categories::*)(const ::std::string&) const) &smtk::common::Categories::passes, py::arg("category"))
    .def("insert", (void (smtk::common::Categories::*)(const smtk::common::Categories&)) &smtk::common::Categories::insert, py::arg("categories"))
    .def("reset", &smtk::common::Categories::reset)
    .def("size", &smtk::common::Categories::size)
    // NOTE that the Python form of this method is returning a copy since Python
    // doesn't support const references
    .def("stacks", &smtk::common::Categories::stacks)
    .def("categoryNames", &smtk::common::Categories::categoryNames)
    ;
  py::class_< smtk::common::Categories::Set >(instance, "Set")
    .def(py::init<>())
    .def("deepcopy", (smtk::common::Categories::Set & (smtk::common::Categories::Set::*)(::smtk::common::Categories::Set const &)) &smtk::common::Categories::Set::operator=)

    .def("combinationMode", &smtk::common::Categories::Set::combinationMode)
    .def("setCombinationMode", &smtk::common::Categories::Set::setCombinationMode)

    .def("exclusionMode", &smtk::common::Categories::Set::exclusionMode)
    .def("setExclusionMode", &smtk::common::Categories::Set::setExclusionMode)
    .def("excludedCategoryNames", &smtk::common::Categories::Set::excludedCategoryNames)
    .def("setExclusions", &smtk::common::Categories::Set::setExclusions)
    .def("insertExclusion", &smtk::common::Categories::Set::insertExclusion)
    .def("eraseExclusion", &smtk::common::Categories::Set::eraseExclusion)
    .def("exclusionSize", &smtk::common::Categories::Set::exclusionSize)

    .def("inclusionMode", &smtk::common::Categories::Set::inclusionMode)
    .def("setInclusionMode", &smtk::common::Categories::Set::setInclusionMode)
    .def("includedCategoryNames", &smtk::common::Categories::Set::includedCategoryNames)
    .def("setInclusions", &smtk::common::Categories::Set::setInclusions)
    .def("insertInclusion", &smtk::common::Categories::Set::insertInclusion)
    .def("eraseInclusion", &smtk::common::Categories::Set::eraseInclusion)
    .def("inclusionSize", &smtk::common::Categories::Set::inclusionSize)

    .def("empty", &smtk::common::Categories::Set::empty)
    .def("reset", &smtk::common::Categories::Set::reset)
    .def("passes", (bool (smtk::common::Categories::Set::*)(const ::std::set<::std::string>&) const) &smtk::common::Categories::Set::passes, py::arg("categories"))
    .def("passes", (bool (smtk::common::Categories::Set::*)(const ::std::string&) const) &smtk::common::Categories::Set::passes, py::arg("category"))
    ;
  py::class_< smtk::common::Categories::Expression, smtk::common::Categories::Set >(instance, "Expression")
    .def(py::init<>())
    .def("deepcopy", (smtk::common::Categories::Expression & (smtk::common::Categories::Expression::*)(::smtk::common::Categories::Expression const &)) &smtk::common::Categories::Expression::operator=)

    .def("setExpression", &smtk::common::Categories::Expression::setExpression)
    .def("expression", &smtk::common::Categories::Expression::expression)
    .def("setAllPass", &smtk::common::Categories::Expression::setAllPass)
    .def("allPass", &smtk::common::Categories::Expression::allPass)
    .def("setAllReject", &smtk::common::Categories::Expression::setAllReject)
    .def("allReject", &smtk::common::Categories::Expression::allReject)
    ;
 py::class_< smtk::common::Categories::Stack >(instance, "Stack")
    .def(py::init<>())
    .def("deepcopy", (smtk::common::Categories::Stack & (smtk::common::Categories::Stack::*)(::smtk::common::Categories::Stack const &)) &smtk::common::Categories::Stack::operator=)

    .def("append", &smtk::common::Categories::Stack::append)
    .def("clear", &smtk::common::Categories::Stack::clear)
    .def("empty", &smtk::common::Categories::Stack::empty)
    .def("passes", (bool (smtk::common::Categories::Stack::*)(const ::std::set<::std::string>&) const) &smtk::common::Categories::Stack::passes, py::arg("categories"))
    .def("passes", (bool (smtk::common::Categories::Stack::*)(const ::std::string&) const) &smtk::common::Categories::Stack::passes, py::arg("category"))
    ;
  py::enum_<smtk::common::Categories::CombinationMode>(instance, "CombinationMode")
    .value("Any", smtk::common::Categories::CombinationMode::Or)
    .value("All", smtk::common::Categories::CombinationMode::And)
    .value("Or", smtk::common::Categories::CombinationMode::Or)
    .value("And", smtk::common::Categories::CombinationMode::And)
    .value("LocalOnly", smtk::common::Categories::CombinationMode::LocalOnly)
    .export_values();
  return instance;
}

#endif
