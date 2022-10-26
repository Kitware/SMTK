//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_FileSystemItem_h
#define pybind_smtk_attribute_FileSystemItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/FileSystemItem.h"

#include "smtk/attribute/Item.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::FileSystemItem, smtk::attribute::Item > pybind11_init_smtk_attribute_FileSystemItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::FileSystemItem, smtk::attribute::Item > instance(m, "FileSystemItem");
  instance
    .def("deepcopy", (smtk::attribute::FileSystemItem & (smtk::attribute::FileSystemItem::*)(::smtk::attribute::FileSystemItem const &)) &smtk::attribute::FileSystemItem::operator=)
    .def("appendValue", &smtk::attribute::FileSystemItem::appendValue, py::arg("val"))
    .def("begin", &smtk::attribute::FileSystemItem::begin)
    .def("defaultValue", &smtk::attribute::FileSystemItem::defaultValue)
    .def("end", &smtk::attribute::FileSystemItem::end)
    .def("hasDefault", &smtk::attribute::FileSystemItem::hasDefault)
    .def("isExtensible", &smtk::attribute::FileSystemItem::isExtensible)
    .def("isSet", &smtk::attribute::FileSystemItem::isSet, py::arg("element") = 0)
    .def("isUsingDefault", (bool (smtk::attribute::FileSystemItem::*)(::size_t) const) &smtk::attribute::FileSystemItem::isUsingDefault, py::arg("elementIndex"))
    .def("isUsingDefault", (bool (smtk::attribute::FileSystemItem::*)() const) &smtk::attribute::FileSystemItem::isUsingDefault)
    .def("maxNumberOfValues", &smtk::attribute::FileSystemItem::maxNumberOfValues)
    .def("numberOfRequiredValues", &smtk::attribute::FileSystemItem::numberOfRequiredValues)
    .def("numberOfValues", &smtk::attribute::FileSystemItem::numberOfValues)
    .def("removeValue", &smtk::attribute::FileSystemItem::removeValue, py::arg("element"))
    .def("reset", &smtk::attribute::FileSystemItem::reset)
    .def("setNumberOfValues", &smtk::attribute::FileSystemItem::setNumberOfValues, py::arg("newSize"))
    .def("setToDefault", &smtk::attribute::FileSystemItem::setToDefault, py::arg("elementIndex") = 0)
    .def("setValue", (bool (smtk::attribute::FileSystemItem::*)(::std::string const &)) &smtk::attribute::FileSystemItem::setValue, py::arg("val"))
    .def("setValue", (bool (smtk::attribute::FileSystemItem::*)(::size_t, ::std::string const &)) &smtk::attribute::FileSystemItem::setValue, py::arg("element"), py::arg("val"))
    .def("shouldBeRelative", &smtk::attribute::FileSystemItem::shouldBeRelative)
    .def("shouldExist", &smtk::attribute::FileSystemItem::shouldExist)
    .def("type", &smtk::attribute::FileSystemItem::type)
    .def("unset", &smtk::attribute::FileSystemItem::unset, py::arg("element") = 0)
    .def("value", &smtk::attribute::FileSystemItem::value, py::arg("element") = 0)
    .def("valueAsString", (std::string (smtk::attribute::FileSystemItem::*)(::std::string const &) const) &smtk::attribute::FileSystemItem::valueAsString, py::arg("format") = "")
    .def("valueAsString", (std::string (smtk::attribute::FileSystemItem::*)(::size_t, ::std::string const &) const) &smtk::attribute::FileSystemItem::valueAsString, py::arg("element"), py::arg("format") = "")
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::FileSystemItem>(i);
      })
    ;
  return instance;
}

#endif
