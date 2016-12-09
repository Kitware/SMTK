//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_MeshItemDefinition_h
#define pybind_smtk_attribute_MeshItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/MeshItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/mesh/MeshSet.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::MeshItemDefinition, smtk::attribute::ItemDefinition > pybind11_init_smtk_attribute_MeshItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::MeshItemDefinition, smtk::attribute::ItemDefinition > instance(m, "MeshItemDefinition");
  instance
    .def(py::init<::smtk::attribute::MeshItemDefinition const &>())
    .def_static("New", &smtk::attribute::MeshItemDefinition::New, py::arg("myName"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::MeshItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::MeshItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::MeshItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::MeshItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::MeshItemDefinition::classname)
    .def("createCopy", &smtk::attribute::MeshItemDefinition::createCopy, py::arg("info"))
    .def("isExtensible", &smtk::attribute::MeshItemDefinition::isExtensible)
    .def("isValueValid", &smtk::attribute::MeshItemDefinition::isValueValid, py::arg("val"))
    .def("maxNumberOfValues", &smtk::attribute::MeshItemDefinition::maxNumberOfValues)
    .def("numberOfRequiredValues", &smtk::attribute::MeshItemDefinition::numberOfRequiredValues)
    .def("setIsExtensible", &smtk::attribute::MeshItemDefinition::setIsExtensible, py::arg("extensible"))
    .def("setMaxNumberOfValues", &smtk::attribute::MeshItemDefinition::setMaxNumberOfValues, py::arg("maxNum"))
    .def("setNumberOfRequiredValues", &smtk::attribute::MeshItemDefinition::setNumberOfRequiredValues, py::arg("esize"))
    .def("type", &smtk::attribute::MeshItemDefinition::type)
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::MeshItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::MeshItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
