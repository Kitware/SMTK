//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_Tag_h
#define pybind_smtk_attribute_Tag_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/Tag.h"

namespace py = pybind11;

inline py::class_< smtk::attribute::Tag > pybind11_init_smtk_attribute_Tag(py::module &m)
{
  py::class_< smtk::attribute::Tag > instance(m, "Tag");
  instance
    .def(py::init<::std::string const &>())
    .def(py::init<::std::string const &, ::std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > const &>())
    .def(py::init<::smtk::attribute::Tag const &>())
    .def("__lt__", (bool (smtk::attribute::Tag::*)(::smtk::attribute::Tag const &) const) &smtk::attribute::Tag::operator<)
    .def("deepcopy", (smtk::attribute::Tag & (smtk::attribute::Tag::*)(::smtk::attribute::Tag const &)) &smtk::attribute::Tag::operator=)
    .def("name", &smtk::attribute::Tag::name)
    .def("values", (std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > const & (smtk::attribute::Tag::*)() const) &smtk::attribute::Tag::values)
    .def("values", (std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > & (smtk::attribute::Tag::*)()) &smtk::attribute::Tag::values)
    .def("add", &smtk::attribute::Tag::add, py::arg("value"))
    .def("remove", &smtk::attribute::Tag::remove, py::arg("value"))
    .def("contains", &smtk::attribute::Tag::contains, py::arg("value"))
    ;
  return instance;
}

#endif
