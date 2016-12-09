//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_MeshSelectionItemDefinition_h
#define pybind_smtk_attribute_MeshSelectionItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/MeshSelectionItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::MeshSelectionItemDefinition, smtk::attribute::ItemDefinition > pybind11_init_smtk_attribute_MeshSelectionItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::MeshSelectionItemDefinition, smtk::attribute::ItemDefinition > instance(m, "MeshSelectionItemDefinition");
  instance
    .def(py::init<::smtk::attribute::MeshSelectionItemDefinition const &>())
    .def_static("New", &smtk::attribute::MeshSelectionItemDefinition::New, py::arg("myName"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::MeshSelectionItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::MeshSelectionItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::MeshSelectionItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::MeshSelectionItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::MeshSelectionItemDefinition::classname)
    .def("createCopy", &smtk::attribute::MeshSelectionItemDefinition::createCopy, py::arg("info"))
    .def("isValueValid", &smtk::attribute::MeshSelectionItemDefinition::isValueValid, py::arg("val"))
    .def("membershipMask", &smtk::attribute::MeshSelectionItemDefinition::membershipMask)
    .def("modifyMode", &smtk::attribute::MeshSelectionItemDefinition::modifyMode)
    .def("refModelEntityName", &smtk::attribute::MeshSelectionItemDefinition::refModelEntityName)
    .def("setMembershipMask", &smtk::attribute::MeshSelectionItemDefinition::setMembershipMask, py::arg("entMask"))
    .def("setModifyMode", &smtk::attribute::MeshSelectionItemDefinition::setModifyMode, py::arg("mode"))
    .def("setRefModelEntityName", &smtk::attribute::MeshSelectionItemDefinition::setRefModelEntityName, py::arg("defName"))
    .def("type", &smtk::attribute::MeshSelectionItemDefinition::type)
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::MeshSelectionItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::MeshSelectionItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
