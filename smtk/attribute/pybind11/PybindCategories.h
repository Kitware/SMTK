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

inline py::class_< smtk::attribute::Categories > pybind11_init_smtk_attribute_Categories(py::module &m)
{
  py::class_< smtk::attribute::Categories > instance(m, "Categories");
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::attribute::Categories & (smtk::attribute::Categories::*)(::smtk::attribute::Categories const &)) &smtk::attribute::Categories::operator=)
    .def("passes", (bool (smtk::attribute::Categories::*)(const ::std::set<::std::string>&) const) &smtk::attribute::Categories::passes, py::arg("categories"))
    .def("passes", (bool (smtk::attribute::Categories::*)(const ::std::string&) const) &smtk::attribute::Categories::passes, py::arg("category"))
    .def("insert", (void (smtk::attribute::Categories::*)(const smtk::attribute::Categories&)) &smtk::attribute::Categories::insert, py::arg("categories"))
    .def("reset", &smtk::attribute::Categories::reset)
    .def("size", &smtk::attribute::Categories::size)
    // NOTE that the Python form of this method is returning a copy since Python
    // doesn't support const references
    .def("stacks", &smtk::attribute::Categories::stacks)
    .def("categoryNames", &smtk::attribute::Categories::categoryNames)
    ;
  py::class_< smtk::attribute::Categories::Set >(instance, "Set")
    .def(py::init<>())
    .def("deepcopy", (smtk::attribute::Categories::Set & (smtk::attribute::Categories::Set::*)(::smtk::attribute::Categories::Set const &)) &smtk::attribute::Categories::Set::operator=)

    .def("combinationMode", &smtk::attribute::Categories::Set::combinationMode)
    .def("setCombinationMode", &smtk::attribute::Categories::Set::setCombinationMode)

    .def("exclusionMode", &smtk::attribute::Categories::Set::exclusionMode)
    .def("setExclusionMode", &smtk::attribute::Categories::Set::setExclusionMode)
    .def("excludedCategoryNames", &smtk::attribute::Categories::Set::excludedCategoryNames)
    .def("setExclusions", &smtk::attribute::Categories::Set::setExclusions)
    .def("insertExclusion", &smtk::attribute::Categories::Set::insertExclusion)
    .def("eraseExclusion", &smtk::attribute::Categories::Set::eraseExclusion)
    .def("exclusionSize", &smtk::attribute::Categories::Set::exclusionSize)

    .def("inclusionMode", &smtk::attribute::Categories::Set::inclusionMode)
    .def("setInclusionMode", &smtk::attribute::Categories::Set::setInclusionMode)
    .def("includedCategoryNames", &smtk::attribute::Categories::Set::includedCategoryNames)
    .def("setInclusions", &smtk::attribute::Categories::Set::setInclusions)
    .def("insertInclusion", &smtk::attribute::Categories::Set::insertInclusion)
    .def("eraseInclusion", &smtk::attribute::Categories::Set::eraseInclusion)
    .def("inclusionSize", &smtk::attribute::Categories::Set::inclusionSize)

    .def("empty", &smtk::attribute::Categories::Set::empty)
    .def("reset", &smtk::attribute::Categories::Set::reset)
    .def("passes", (bool (smtk::attribute::Categories::Set::*)(const ::std::set<::std::string>&) const) &smtk::attribute::Categories::Set::passes, py::arg("categories"))
    .def("passes", (bool (smtk::attribute::Categories::Set::*)(const ::std::string&) const) &smtk::attribute::Categories::Set::passes, py::arg("category"))
    ;
  py::class_< smtk::attribute::Categories::Expression, smtk::attribute::Categories::Set >(instance, "Expression")
    .def(py::init<>())
    .def("deepcopy", (smtk::attribute::Categories::Expression & (smtk::attribute::Categories::Expression::*)(::smtk::attribute::Categories::Expression const &)) &smtk::attribute::Categories::Expression::operator=)

    .def("setExpression", &smtk::attribute::Categories::Expression::setExpression)
    .def("expression", &smtk::attribute::Categories::Expression::expression)
    .def("setAllPass", &smtk::attribute::Categories::Expression::setAllPass)
    .def("allPass", &smtk::attribute::Categories::Expression::allPass)
    .def("setAllReject", &smtk::attribute::Categories::Expression::setAllReject)
    .def("allReject", &smtk::attribute::Categories::Expression::allReject)
    ;
 py::class_< smtk::attribute::Categories::Stack >(instance, "Stack")
    .def(py::init<>())
    .def("deepcopy", (smtk::attribute::Categories::Stack & (smtk::attribute::Categories::Stack::*)(::smtk::attribute::Categories::Stack const &)) &smtk::attribute::Categories::Stack::operator=)

    .def("append", &smtk::attribute::Categories::Stack::append)
    .def("clear", &smtk::attribute::Categories::Stack::clear)
    .def("empty", &smtk::attribute::Categories::Stack::empty)
    .def("passes", (bool (smtk::attribute::Categories::Stack::*)(const ::std::set<::std::string>&) const) &smtk::attribute::Categories::Stack::passes, py::arg("categories"))
    .def("passes", (bool (smtk::attribute::Categories::Stack::*)(const ::std::string&) const) &smtk::attribute::Categories::Stack::passes, py::arg("category"))
    ;
  py::enum_<smtk::attribute::Categories::CombinationMode>(instance, "CombinationMode")
    .value("Any", smtk::attribute::Categories::CombinationMode::Or)
    .value("All", smtk::attribute::Categories::CombinationMode::And)
    .value("Or", smtk::attribute::Categories::CombinationMode::Or)
    .value("And", smtk::attribute::Categories::CombinationMode::And)
    .value("LocalOnly", smtk::attribute::Categories::CombinationMode::LocalOnly)
    .export_values();
  return instance;
}

#endif
