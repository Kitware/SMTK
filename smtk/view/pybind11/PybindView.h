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

#include "smtk/view/Configuration.h"

#include <sstream>
#include <unordered_set>

namespace py = pybind11;

namespace
{
  std::unordered_set<const smtk::view::Configuration*> allViews;
}

inline PySharedPtrClass< smtk::view::Configuration > pybind11_init_smtk_view_View(py::module &m)
{
  PySharedPtrClass< smtk::view::Configuration > instance(m, "View");
  instance
    .def(py::init<::std::string const &, ::std::string const &>())
    .def(py::init<::smtk::view::Configuration const &>())
    .def("deepcopy", (smtk::view::Configuration & (smtk::view::Configuration::*)(::smtk::view::Configuration const &)) &smtk::view::Configuration::operator=)
    .def_static("New", &smtk::view::Configuration::New, py::arg("myType"), py::arg("myName"), py::return_value_policy::take_ownership)
    .def("name", &smtk::view::Configuration::name)
    .def("type", &smtk::view::Configuration::type)
    .def("iconName", &smtk::view::Configuration::iconName)
    .def("setIconName", &smtk::view::Configuration::setIconName, py::arg("name"))
    .def("details", (smtk::view::Configuration::Component& (smtk::view::Configuration::*)()) &smtk::view::Configuration::details, py::return_value_policy::reference)
    .def("pointer", [](const smtk::view::Configuration& self)
      {
        std::ostringstream addr;
        addr << std::hex << &self;
        allViews.insert(&self);
        return addr.str();
      })
    .def_static("fromPointer", [](const std::string& ptrStr)
      { // FIXME: DO NOT COMMIT THIS
        char* end = const_cast<char*>(ptrStr.c_str() + ptrStr.size());
        // NOLINTNEXTLINE(performance-no-int-to-ptr)
        auto* ptr = reinterpret_cast<smtk::view::Configuration*>(strtoull(ptrStr.c_str(), &end, 16));
        if (ptr && allViews.find(ptr) != allViews.end())
        {
          return ptr->shared_from_this();
        }
        return smtk::view::Configuration::Ptr();
      })
    ;
  py::class_< smtk::view::Configuration::Component >(instance, "Component")
    .def(py::init<::std::string const &>())
    .def(py::init<>())
    .def(py::init<::smtk::view::Configuration::Component const &>())
    .def("deepcopy", (smtk::view::Configuration::Component & (smtk::view::Configuration::Component::*)(::smtk::view::Configuration::Component const &)) &smtk::view::Configuration::Component::operator=)
    .def("name", &smtk::view::Configuration::Component::name)
    .def("contents", &smtk::view::Configuration::Component::contents)
    .def("contentsAsVector", &smtk::view::Configuration::Component::contentsAsVector, py::arg("vec"))
    .def("contentsAsInt", &smtk::view::Configuration::Component::contentsAsInt, py::arg("val"))
    .def("setContents", &smtk::view::Configuration::Component::setContents, py::arg("c"))
    .def("setAttribute", &smtk::view::Configuration::Component::setAttribute, py::arg("attname"), py::arg("value"))
    .def("unsetAttribute", &smtk::view::Configuration::Component::unsetAttribute, py::arg("attname"))
    .def("attribute", (bool (smtk::view::Configuration::Component::*)(::std::string const &, ::std::string &) const) &smtk::view::Configuration::Component::attribute, py::arg("attname"), py::arg("value"))
    .def("attribute", (bool (smtk::view::Configuration::Component::*)(::std::string const &) const) &smtk::view::Configuration::Component::attribute, py::arg("attname"))
    .def("attributeAsBool", (bool (smtk::view::Configuration::Component::*)(::std::string const &, bool &) const) &smtk::view::Configuration::Component::attributeAsBool, py::arg("attname"), py::arg("value"))
    .def("attributeAsBool", (bool (smtk::view::Configuration::Component::*)(::std::string const &) const) &smtk::view::Configuration::Component::attributeAsBool, py::arg("attname"))
    .def("attributeAsInt", (bool (smtk::view::Configuration::Component::*)(::std::string const &, int &) const) &smtk::view::Configuration::Component::attributeAsInt, py::arg("attname"), py::arg("value"))
    .def("attributeAsString", (std::string (smtk::view::Configuration::Component::*)(::std::string const &) const) &smtk::view::Configuration::Component::attributeAsString, py::arg("attname"))
    .def("attributes", &smtk::view::Configuration::Component::attributes)
    .def("addChild", &smtk::view::Configuration::Component::addChild, py::arg("childName"), py::return_value_policy::reference)
    .def("numberOfChildren", &smtk::view::Configuration::Component::numberOfChildren)
    .def("child", (smtk::view::Configuration::Component& (smtk::view::Configuration::Component::*)(std::size_t)) &smtk::view::Configuration::Component::child, py::arg("i"), py::return_value_policy::reference)
    .def("findChild", &smtk::view::Configuration::Component::findChild, py::arg("compName"))
    ;
  return instance;
}

#endif
