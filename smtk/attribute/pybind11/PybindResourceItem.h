//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_ResourceItem_h
#define pybind_smtk_attribute_ResourceItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ResourceItem.h"

#include "smtk/attribute/Item.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/resource/Resource.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::ResourceItem, smtk::attribute::Item > pybind11_init_smtk_attribute_ResourceItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ResourceItem, smtk::attribute::Item > instance(m, "ResourceItem");
  instance
    .def(py::init<::smtk::attribute::ResourceItem const &>())
    .def("deepcopy", (smtk::attribute::ResourceItem & (smtk::attribute::ResourceItem::*)(::smtk::attribute::ResourceItem const &)) &smtk::attribute::ResourceItem::operator=)
    .def("appendValue", &smtk::attribute::ResourceItem::appendValue, py::arg("val"))
    .def("assign", &smtk::attribute::ResourceItem::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def("begin", &smtk::attribute::ResourceItem::begin)
    .def("definition", &smtk::attribute::ResourceItem::definition)
    .def("end", &smtk::attribute::ResourceItem::end)
    .def("find", (ptrdiff_t (smtk::attribute::ResourceItem::*)(::smtk::common::UUID const &) const) &smtk::attribute::ResourceItem::find, py::arg("rsrcId"))
    .def("find", (ptrdiff_t (smtk::attribute::ResourceItem::*)(::smtk::resource::ResourcePtr) const) &smtk::attribute::ResourceItem::find, py::arg("resource"))
    .def("has", (bool (smtk::attribute::ResourceItem::*)(::smtk::common::UUID const &) const) &smtk::attribute::ResourceItem::has, py::arg("rsrcId"))
    .def("has", (bool (smtk::attribute::ResourceItem::*)(::smtk::resource::ResourcePtr) const) &smtk::attribute::ResourceItem::has, py::arg("rsrc"))
    .def("isExtensible", &smtk::attribute::ResourceItem::isExtensible)
    .def("isSet", &smtk::attribute::ResourceItem::isSet, py::arg("i") = 0)
    .def("isValid", &smtk::attribute::ResourceItem::isValid)
    .def("numberOfRequiredValues", &smtk::attribute::ResourceItem::numberOfRequiredValues)
    .def("numberOfValues", &smtk::attribute::ResourceItem::numberOfValues)
    .def("removeValue", &smtk::attribute::ResourceItem::removeValue, py::arg("i"))
    .def("reset", &smtk::attribute::ResourceItem::reset)
    .def("setNumberOfValues", &smtk::attribute::ResourceItem::setNumberOfValues, py::arg("newSize"))
    .def("setValue", (bool (smtk::attribute::ResourceItem::*)(::smtk::resource::ResourcePtr)) &smtk::attribute::ResourceItem::setValue, py::arg("val"))
    .def("setValue", (bool (smtk::attribute::ResourceItem::*)(::size_t, ::smtk::resource::ResourcePtr)) &smtk::attribute::ResourceItem::setValue, py::arg("i"), py::arg("val"))
    .def("type", &smtk::attribute::ResourceItem::type)
    .def("unset", &smtk::attribute::ResourceItem::unset, py::arg("i") = 0)
    .def("value", &smtk::attribute::ResourceItem::value, py::arg("i") = 0)
    .def("valueAsString", (std::string (smtk::attribute::ResourceItem::*)() const) &smtk::attribute::ResourceItem::valueAsString)
    .def("valueAsString", (std::string (smtk::attribute::ResourceItem::*)(::size_t) const) &smtk::attribute::ResourceItem::valueAsString, py::arg("i"))
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(i);
      })
    ;
  return instance;
}

#endif
