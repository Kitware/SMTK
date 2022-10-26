//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_ModelEntityItem_h
#define pybind_smtk_attribute_ModelEntityItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/EntityRef.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::ModelEntityItem, smtk::attribute::ComponentItem > pybind11_init_smtk_attribute_ModelEntityItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ModelEntityItem, smtk::attribute::ComponentItem > instance(m, "ModelEntityItem");
  instance
    .def(py::init<::smtk::attribute::ModelEntityItem const &>())
    .def("deepcopy", (smtk::attribute::ModelEntityItem & (smtk::attribute::ModelEntityItem::*)(::smtk::attribute::ModelEntityItem const &)) &smtk::attribute::ModelEntityItem::operator=)
    .def("appendValue", (bool (smtk::attribute::ModelEntityItem::*)(const smtk::model::EntityRef&)) &smtk::attribute::ModelEntityItem::appendValue, py::arg("val"))
    .def("begin", &smtk::attribute::ModelEntityItem::begin)
    .def("end", &smtk::attribute::ModelEntityItem::end)
    .def("find", (ptrdiff_t (smtk::attribute::ModelEntityItem::*)(::smtk::model::EntityRef const &) const) &smtk::attribute::ModelEntityItem::find, py::arg("entity"))
    .def("contains", (bool (smtk::attribute::ModelEntityItem::*)(::smtk::model::EntityRef const &) const) &smtk::attribute::ModelEntityItem::contains, py::arg("entity"))
    .def("isExtensible", &smtk::attribute::ModelEntityItem::isExtensible)
    .def("isSet", &smtk::attribute::ModelEntityItem::isSet, py::arg("element") = 0)
    .def("numberOfRequiredValues", &smtk::attribute::ModelEntityItem::numberOfRequiredValues)
    .def("numberOfValues", &smtk::attribute::ModelEntityItem::numberOfValues)
    .def("removeValue", &smtk::attribute::ModelEntityItem::removeValue, py::arg("element"))
    .def("reset", &smtk::attribute::ModelEntityItem::reset)
    .def("setNumberOfValues", &smtk::attribute::ModelEntityItem::setNumberOfValues, py::arg("newSize"))
    .def("setValue", (bool (smtk::attribute::ModelEntityItem::*)(::smtk::model::EntityRef const &)) &smtk::attribute::ModelEntityItem::setValue, py::arg("val"))
    .def("setValue", (bool (smtk::attribute::ModelEntityItem::*)(::size_t, ::smtk::model::EntityRef const &)) &smtk::attribute::ModelEntityItem::setValue, py::arg("element"), py::arg("val"))
    .def("type", &smtk::attribute::ModelEntityItem::type)
    .def("unset", &smtk::attribute::ModelEntityItem::unset, py::arg("element") = 0)
    .def("value", &smtk::attribute::ModelEntityItem::value, py::arg("element") = 0)
    .def("valueAsString", (std::string (smtk::attribute::ModelEntityItem::*)(::size_t) const) &smtk::attribute::ModelEntityItem::valueAsString, py::arg("element") = 0)
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::ModelEntityItem>(i);
      })
    ;
  return instance;
}

#endif
