//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_attribute_ValueItemTemplate_h
#define pybind_attribute_ValueItemTemplate_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemTemplate.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::ValueItemTemplate<int>, smtk::attribute::ValueItem > pybind11_init_smtk_attribute_ValueItemTemplate_int_(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ValueItemTemplate<int>, smtk::attribute::ValueItem > instance(m, "ValueItemTemplate_int_");
  instance
    .def("appendExpression", &smtk::attribute::ValueItemTemplate<int>::appendExpression, py::arg("exp"))
    .def("appendValue", &smtk::attribute::ValueItemTemplate<int>::appendValue, py::arg("val"))
    .def("assign", &smtk::attribute::ValueItemTemplate<int>::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def("begin", &smtk::attribute::ValueItemTemplate<int>::begin)
    .def("concreteDefinition", &smtk::attribute::ValueItemTemplate<int>::concreteDefinition)
    .def("defaultValue", &smtk::attribute::ValueItemTemplate<int>::defaultValue)
    .def("defaultValues", &smtk::attribute::ValueItemTemplate<int>::defaultValues)
    .def("end", &smtk::attribute::ValueItemTemplate<int>::end)
    .def("isUsingDefault", (bool (smtk::attribute::ValueItemTemplate<int>::*)(::size_t) const) &smtk::attribute::ValueItemTemplate<int>::isUsingDefault)
    .def("isUsingDefault", (bool (smtk::attribute::ValueItemTemplate<int>::*)() const) &smtk::attribute::ValueItemTemplate<int>::isUsingDefault)
    .def("removeValue", &smtk::attribute::ValueItemTemplate<int>::removeValue, py::arg("element"))
    .def("reset", &smtk::attribute::ValueItemTemplate<int>::reset)
    .def("setNumberOfValues", &smtk::attribute::ValueItemTemplate<int>::setNumberOfValues, py::arg("newSize"))
    .def("setToDefault", &smtk::attribute::ValueItemTemplate<int>::setToDefault, py::arg("element") = 0)
    .def("setValue", (bool (smtk::attribute::ValueItemTemplate<int>::*)(int const &)) &smtk::attribute::ValueItemTemplate<int>::setValue)
    .def("setValue", (bool (smtk::attribute::ValueItemTemplate<int>::*)(::size_t, int const &)) &smtk::attribute::ValueItemTemplate<int>::setValue)
    .def("value", &smtk::attribute::ValueItemTemplate<int>::value, py::arg("element") = 0)
    .def("valueAsString", (std::string (smtk::attribute::ValueItemTemplate<int>::*)() const) &smtk::attribute::ValueItemTemplate<int>::valueAsString)
    .def("valueAsString", (std::string (smtk::attribute::ValueItemTemplate<int>::*)(::size_t) const) &smtk::attribute::ValueItemTemplate<int>::valueAsString)
    ;
  return instance;
}

PySharedPtrClass<smtk::attribute::ValueItemTemplate<double>, smtk::attribute::ValueItem > pybind11_init_smtk_attribute_ValueItemTemplate_double_(py::module &m)
{
  PySharedPtrClass<smtk::attribute::ValueItemTemplate<double>, smtk::attribute::ValueItem > instance(m, "ValueItemTemplate_double_");
  instance
    .def("appendExpression", &smtk::attribute::ValueItemTemplate<double>::appendExpression, py::arg("exp"))
    .def("appendValue", &smtk::attribute::ValueItemTemplate<double>::appendValue, py::arg("val"))
    .def("assign", &smtk::attribute::ValueItemTemplate<double>::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def("begin", &smtk::attribute::ValueItemTemplate<double>::begin)
    .def("concreteDefinition", &smtk::attribute::ValueItemTemplate<double>::concreteDefinition)
    .def("defaultValue", &smtk::attribute::ValueItemTemplate<double>::defaultValue)
    .def("defaultValues", &smtk::attribute::ValueItemTemplate<double>::defaultValues)
    .def("end", &smtk::attribute::ValueItemTemplate<double>::end)
    .def("isUsingDefault", (bool (smtk::attribute::ValueItemTemplate<double>::*)(::size_t) const) &smtk::attribute::ValueItemTemplate<double>::isUsingDefault)
    .def("isUsingDefault", (bool (smtk::attribute::ValueItemTemplate<double>::*)() const) &smtk::attribute::ValueItemTemplate<double>::isUsingDefault)
    .def("removeValue", &smtk::attribute::ValueItemTemplate<double>::removeValue, py::arg("element"))
    .def("reset", &smtk::attribute::ValueItemTemplate<double>::reset)
    .def("setNumberOfValues", &smtk::attribute::ValueItemTemplate<double>::setNumberOfValues, py::arg("newSize"))
    .def("setToDefault", &smtk::attribute::ValueItemTemplate<double>::setToDefault, py::arg("element") = 0)
    .def("setValue", (bool (smtk::attribute::ValueItemTemplate<double>::*)(double const &)) &smtk::attribute::ValueItemTemplate<double>::setValue)
    .def("setValue", (bool (smtk::attribute::ValueItemTemplate<double>::*)(::size_t, double const &)) &smtk::attribute::ValueItemTemplate<double>::setValue)
    .def("value", &smtk::attribute::ValueItemTemplate<double>::value, py::arg("element") = 0)
    .def("valueAsString", (std::string (smtk::attribute::ValueItemTemplate<double>::*)() const) &smtk::attribute::ValueItemTemplate<double>::valueAsString)
    .def("valueAsString", (std::string (smtk::attribute::ValueItemTemplate<double>::*)(::size_t) const) &smtk::attribute::ValueItemTemplate<double>::valueAsString)
    ;
  return instance;
}

PySharedPtrClass<smtk::attribute::ValueItemTemplate<std::string>, smtk::attribute::ValueItem > pybind11_init_smtk_attribute_ValueItemTemplate_string_(py::module &m)
{
  PySharedPtrClass<smtk::attribute::ValueItemTemplate<std::string>, smtk::attribute::ValueItem > instance(m, "ValueItemTemplate_string_");
  instance
    .def("appendExpression", &smtk::attribute::ValueItemTemplate<std::string>::appendExpression, py::arg("exp"))
    .def("appendValue", &smtk::attribute::ValueItemTemplate<std::string>::appendValue, py::arg("val"))
    .def("assign", &smtk::attribute::ValueItemTemplate<std::string>::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def("begin", &smtk::attribute::ValueItemTemplate<std::string>::begin)
    .def("concreteDefinition", &smtk::attribute::ValueItemTemplate<std::string>::concreteDefinition)
    .def("defaultValue", &smtk::attribute::ValueItemTemplate<std::string>::defaultValue)
    .def("defaultValues", &smtk::attribute::ValueItemTemplate<std::string>::defaultValues)
    .def("end", &smtk::attribute::ValueItemTemplate<std::string>::end)
    .def("isUsingDefault", (bool (smtk::attribute::ValueItemTemplate<std::string>::*)(::size_t) const) &smtk::attribute::ValueItemTemplate<std::string>::isUsingDefault)
    .def("isUsingDefault", (bool (smtk::attribute::ValueItemTemplate<std::string>::*)() const) &smtk::attribute::ValueItemTemplate<std::string>::isUsingDefault)
    .def("removeValue", &smtk::attribute::ValueItemTemplate<std::string>::removeValue, py::arg("element"))
    .def("reset", &smtk::attribute::ValueItemTemplate<std::string>::reset)
    .def("setNumberOfValues", &smtk::attribute::ValueItemTemplate<std::string>::setNumberOfValues, py::arg("newSize"))
    .def("setToDefault", &smtk::attribute::ValueItemTemplate<std::string>::setToDefault, py::arg("element") = 0)
    .def("setValue", (bool (smtk::attribute::ValueItemTemplate<std::string>::*)(::std::basic_string<char, std::char_traits<char>, std::allocator<char> > const &)) &smtk::attribute::ValueItemTemplate<std::string>::setValue)
    .def("setValue", (bool (smtk::attribute::ValueItemTemplate<std::string>::*)(::size_t, ::std::basic_string<char, std::char_traits<char>, std::allocator<char> > const &)) &smtk::attribute::ValueItemTemplate<std::string>::setValue)
    .def("value", &smtk::attribute::ValueItemTemplate<std::string>::value, py::arg("element") = 0)
    .def("valueAsString", (std::string (smtk::attribute::ValueItemTemplate<std::string>::*)() const) &smtk::attribute::ValueItemTemplate<std::string>::valueAsString)
    .def("valueAsString", (std::string (smtk::attribute::ValueItemTemplate<std::string>::*)(::size_t) const) &smtk::attribute::ValueItemTemplate<std::string>::valueAsString)
    ;
  return instance;

}

#endif
