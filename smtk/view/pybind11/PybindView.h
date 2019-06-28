//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_view_View_h
#define pybind_smtk_view_View_h

#include <pybind11/pybind11.h>

#include "smtk/view/View.h"

namespace py = pybind11;

PySharedPtrClass< smtk::view::View > pybind11_init_smtk_view_View(py::module &m)
{
  PySharedPtrClass< smtk::view::View > instance(m, "View");
  instance
    .def(py::init<::std::string const &, ::std::string const &>())
    .def(py::init<::smtk::view::View const &>())
    .def("deepcopy", (smtk::view::View & (smtk::view::View::*)(::smtk::view::View const &)) &smtk::view::View::operator=)
    .def_static("New", &smtk::view::View::New, py::arg("myType"), py::arg("myName"), py::return_value_policy::take_ownership)
    .def("name", &smtk::view::View::name)
    .def("type", &smtk::view::View::type)
    .def("iconName", &smtk::view::View::iconName)
    .def("setIconName", &smtk::view::View::setIconName, py::arg("name"))
    .def("details", &smtk::view::View::details, py::return_value_policy::reference)
    ;
  py::class_< smtk::view::View::Component >(instance, "Component")
    .def(py::init<::std::string const &>())
    .def(py::init<>())
    .def(py::init<::smtk::view::View::Component const &>())
    .def("deepcopy", (smtk::view::View::Component & (smtk::view::View::Component::*)(::smtk::view::View::Component const &)) &smtk::view::View::Component::operator=)
    .def("name", &smtk::view::View::Component::name)
    .def("contents", &smtk::view::View::Component::contents)
    .def("contentsAsVector", &smtk::view::View::Component::contentsAsVector, py::arg("vec"))
    .def("contentsAsInt", &smtk::view::View::Component::contentsAsInt, py::arg("val"))
    .def("setContents", &smtk::view::View::Component::setContents, py::arg("c"))
    .def("setAttribute", &smtk::view::View::Component::setAttribute, py::arg("attname"), py::arg("value"))
    .def("unsetAttribute", &smtk::view::View::Component::unsetAttribute, py::arg("attname"))
    .def("attribute", (bool (smtk::view::View::Component::*)(::std::string const &, ::std::string &) const) &smtk::view::View::Component::attribute, py::arg("attname"), py::arg("value"))
    .def("attribute", (bool (smtk::view::View::Component::*)(::std::string const &) const) &smtk::view::View::Component::attribute, py::arg("attname"))
    .def("attributeAsBool", (bool (smtk::view::View::Component::*)(::std::string const &, bool &) const) &smtk::view::View::Component::attributeAsBool, py::arg("attname"), py::arg("value"))
    .def("attributeAsBool", (bool (smtk::view::View::Component::*)(::std::string const &) const) &smtk::view::View::Component::attributeAsBool, py::arg("attname"))
    .def("attributes", &smtk::view::View::Component::attributes)
    .def("addChild", &smtk::view::View::Component::addChild, py::arg("childName"), py::return_value_policy::reference)
    .def("numberOfChildren", &smtk::view::View::Component::numberOfChildren)
    .def("child", &smtk::view::View::Component::child, py::arg("i"), py::return_value_policy::reference)
    .def("findChild", &smtk::view::View::Component::findChild, py::arg("compName"))
    ;
  return instance;
}

#endif
