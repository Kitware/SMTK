//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_MeshItem_h
#define pybind_smtk_attribute_MeshItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/MeshItem.h"

#include "smtk/attribute/Item.h"
#include "smtk/mesh/MeshSet.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::MeshItem, smtk::attribute::Item > pybind11_init_smtk_attribute_MeshItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::MeshItem, smtk::attribute::Item > instance(m, "MeshItem");
  instance
    .def(py::init<::smtk::attribute::MeshItem const &>())
    .def("deepcopy", (smtk::attribute::MeshItem & (smtk::attribute::MeshItem::*)(::smtk::attribute::MeshItem const &)) &smtk::attribute::MeshItem::operator=)
    .def("appendValue", &smtk::attribute::MeshItem::appendValue, py::arg("arg0"))
    .def("appendValues", (bool (smtk::attribute::MeshItem::*)(::smtk::mesh::MeshList const &)) &smtk::attribute::MeshItem::appendValues, py::arg("arg0"))
    .def("appendValues", (bool (smtk::attribute::MeshItem::*)(::smtk::mesh::MeshSets const &)) &smtk::attribute::MeshItem::appendValues, py::arg("arg0"))
    .def("assign", &smtk::attribute::MeshItem::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def("begin", &smtk::attribute::MeshItem::begin)
    .def("classname", &smtk::attribute::MeshItem::classname)
    .def("end", &smtk::attribute::MeshItem::end)
    .def("find", &smtk::attribute::MeshItem::find, py::arg("mesh"))
    .def("hasValue", &smtk::attribute::MeshItem::hasValue, py::arg("arg0"))
    .def("isExtensible", &smtk::attribute::MeshItem::isExtensible)
    .def("isSet", &smtk::attribute::MeshItem::isSet, py::arg("element") = 0)
    .def("isValid", &smtk::attribute::MeshItem::isValid)
    .def("numberOfRequiredValues", &smtk::attribute::MeshItem::numberOfRequiredValues)
    .def("numberOfValues", &smtk::attribute::MeshItem::numberOfValues)
    .def("removeValue", &smtk::attribute::MeshItem::removeValue, py::arg("element"))
    .def("reset", &smtk::attribute::MeshItem::reset)
    .def("setNumberOfValues", &smtk::attribute::MeshItem::setNumberOfValues, py::arg("newSize"))
    .def("setValue", (bool (smtk::attribute::MeshItem::*)(::smtk::mesh::MeshSet const &)) &smtk::attribute::MeshItem::setValue, py::arg("val"))
    .def("setValue", (bool (smtk::attribute::MeshItem::*)(::size_t, ::smtk::mesh::MeshSet const &)) &smtk::attribute::MeshItem::setValue, py::arg("element"), py::arg("val"))
    .def("type", &smtk::attribute::MeshItem::type)
    .def("unset", &smtk::attribute::MeshItem::unset, py::arg("element") = 0)
    .def("value", &smtk::attribute::MeshItem::value, py::arg("element") = 0)
    .def("values", &smtk::attribute::MeshItem::values)
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::MeshItem>(i);
      })
    ;
  return instance;
}

#endif
