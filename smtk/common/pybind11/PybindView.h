//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_View_h
#define pybind_smtk_common_View_h

#include <pybind11/pybind11.h>

#include "smtk/common/View.h"

namespace py = pybind11;

py::class_< smtk::common::View > pybind11_init_smtk_common_View(py::module &m)
{
  py::class_< smtk::common::View > instance(m, "View");
  instance
    .def(py::init<::std::string const &, ::std::string const &>())
    .def(py::init<::smtk::common::View const &>())
    .def("deepcopy", (smtk::common::View & (smtk::common::View::*)(::smtk::common::View const &)) &smtk::common::View::operator=)
    .def_static("New", &smtk::common::View::New, py::arg("myType"), py::arg("myTitle"))
    .def("title", &smtk::common::View::title)
    .def("type", &smtk::common::View::type)
    .def("iconName", &smtk::common::View::iconName)
    .def("setIconName", &smtk::common::View::setIconName, py::arg("name"))
    .def("details", &smtk::common::View::details)
    ;
  py::class_< smtk::common::View::Component >(instance, "Component")
    .def(py::init<::std::string const &>())
    .def(py::init<>())
    .def(py::init<::smtk::common::View::Component const &>())
    .def("deepcopy", (smtk::common::View::Component & (smtk::common::View::Component::*)(::smtk::common::View::Component const &)) &smtk::common::View::Component::operator=)
    .def("name", &smtk::common::View::Component::name)
    .def("contents", &smtk::common::View::Component::contents)
    .def("contentsAsVector", &smtk::common::View::Component::contentsAsVector, py::arg("vec"))
    .def("contentsAsInt", &smtk::common::View::Component::contentsAsInt, py::arg("val"))
    .def("setContents", &smtk::common::View::Component::setContents, py::arg("c"))
    .def("setAttribute", &smtk::common::View::Component::setAttribute, py::arg("attname"), py::arg("value"))
    .def("attribute", &smtk::common::View::Component::attribute, py::arg("attname"), py::arg("value"))
    .def("attributeAsBool", (bool (smtk::common::View::Component::*)(::std::string const &, bool &) const) &smtk::common::View::Component::attributeAsBool, py::arg("attname"), py::arg("value"))
    .def("attributeAsBool", (bool (smtk::common::View::Component::*)(::std::string const &) const) &smtk::common::View::Component::attributeAsBool, py::arg("attname"))
    .def("attributes", &smtk::common::View::Component::attributes)
    .def("addChild", &smtk::common::View::Component::addChild, py::arg("childName"))
    .def("numberOfChildren", &smtk::common::View::Component::numberOfChildren)
    .def("child", &smtk::common::View::Component::child, py::arg("i"))
    .def("findChild", &smtk::common::View::Component::findChild, py::arg("compName"))
    ;
  return instance;
}

#endif
