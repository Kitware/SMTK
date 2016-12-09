//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_MeshSelectionItem_h
#define pybind_smtk_attribute_MeshSelectionItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/MeshSelectionItem.h"

#include "smtk/attribute/Item.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

void pybind11_init_smtk_attribute_MeshModifyMode(py::module &m)
{
  py::enum_<smtk::attribute::MeshModifyMode>(m, "MeshModifyMode")
    .value("NONE", smtk::attribute::MeshModifyMode::NONE)
    .value("RESET", smtk::attribute::MeshModifyMode::RESET)
    .value("MERGE", smtk::attribute::MeshModifyMode::MERGE)
    .value("SUBTRACT", smtk::attribute::MeshModifyMode::SUBTRACT)
    .value("ACCEPT", smtk::attribute::MeshModifyMode::ACCEPT)
    .value("NUM_OF_MODIFYMODES", smtk::attribute::MeshModifyMode::NUM_OF_MODIFYMODES)
    .export_values();
}

PySharedPtrClass< smtk::attribute::MeshSelectionItem, smtk::attribute::Item > pybind11_init_smtk_attribute_MeshSelectionItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::MeshSelectionItem, smtk::attribute::Item > instance(m, "MeshSelectionItem");
  instance
    .def(py::init<::smtk::attribute::MeshSelectionItem const &>())
    .def("deepcopy", (smtk::attribute::MeshSelectionItem & (smtk::attribute::MeshSelectionItem::*)(::smtk::attribute::MeshSelectionItem const &)) &smtk::attribute::MeshSelectionItem::operator=)
    .def("assign", &smtk::attribute::MeshSelectionItem::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def("begin", &smtk::attribute::MeshSelectionItem::begin)
    .def("classname", &smtk::attribute::MeshSelectionItem::classname)
    .def("end", &smtk::attribute::MeshSelectionItem::end)
    .def("isCtrlKeyDown", &smtk::attribute::MeshSelectionItem::isCtrlKeyDown)
    .def("isValid", &smtk::attribute::MeshSelectionItem::isValid)
    .def("modifyMode", &smtk::attribute::MeshSelectionItem::modifyMode)
    .def_static("modifyMode2String", &smtk::attribute::MeshSelectionItem::modifyMode2String, py::arg("m"))
    .def("numberOfValues", &smtk::attribute::MeshSelectionItem::numberOfValues)
    .def("removeValues", &smtk::attribute::MeshSelectionItem::removeValues, py::arg("arg0"), py::arg("arg1"))
    .def("reset", &smtk::attribute::MeshSelectionItem::reset)
    .def("setCtrlKeyDown", &smtk::attribute::MeshSelectionItem::setCtrlKeyDown, py::arg("val"))
    .def("setModifyMode", &smtk::attribute::MeshSelectionItem::setModifyMode, py::arg("mode"))
    .def("setValues", &smtk::attribute::MeshSelectionItem::setValues, py::arg("arg0"), py::arg("arg1"))
    .def_static("string2ModifyMode", &smtk::attribute::MeshSelectionItem::string2ModifyMode, py::arg("s"))
    .def("type", &smtk::attribute::MeshSelectionItem::type)
    .def("unionValues", &smtk::attribute::MeshSelectionItem::unionValues, py::arg("arg0"), py::arg("arg1"))
    .def("values", &smtk::attribute::MeshSelectionItem::values, py::arg("arg0"))
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::MeshSelectionItem>(i);
      })    ;
  return instance;
}

#endif
