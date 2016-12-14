//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_ModelEntityItemDefinition_h
#define pybind_smtk_attribute_ModelEntityItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ModelEntityItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/model/EntityRef.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::ModelEntityItemDefinition, smtk::attribute::ItemDefinition > pybind11_init_smtk_attribute_ModelEntityItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ModelEntityItemDefinition, smtk::attribute::ItemDefinition > instance(m, "ModelEntityItemDefinition");
  instance
    .def(py::init<::smtk::attribute::ModelEntityItemDefinition const &>())
    .def_static("New", &smtk::attribute::ModelEntityItemDefinition::New, py::arg("sname"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ModelEntityItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::ModelEntityItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ModelEntityItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::ModelEntityItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::ModelEntityItemDefinition::classname)
    .def("createCopy", &smtk::attribute::ModelEntityItemDefinition::createCopy, py::arg("info"))
    .def("hasValueLabels", &smtk::attribute::ModelEntityItemDefinition::hasValueLabels)
    .def("isExtensible", &smtk::attribute::ModelEntityItemDefinition::isExtensible)
    .def("isValueValid", &smtk::attribute::ModelEntityItemDefinition::isValueValid, py::arg("entity"))
    .def("maxNumberOfValues", &smtk::attribute::ModelEntityItemDefinition::maxNumberOfValues)
    .def("membershipMask", &smtk::attribute::ModelEntityItemDefinition::membershipMask)
    .def("numberOfRequiredValues", &smtk::attribute::ModelEntityItemDefinition::numberOfRequiredValues)
    .def("setCommonValueLabel", &smtk::attribute::ModelEntityItemDefinition::setCommonValueLabel, py::arg("elabel"))
    .def("setIsExtensible", &smtk::attribute::ModelEntityItemDefinition::setIsExtensible, py::arg("extensible"))
    .def("setMaxNumberOfValues", &smtk::attribute::ModelEntityItemDefinition::setMaxNumberOfValues, py::arg("maxNum"))
    .def("setMembershipMask", &smtk::attribute::ModelEntityItemDefinition::setMembershipMask, py::arg("entMask"))
    .def("setNumberOfRequiredValues", &smtk::attribute::ModelEntityItemDefinition::setNumberOfRequiredValues, py::arg("esize"))
    .def("setValueLabel", &smtk::attribute::ModelEntityItemDefinition::setValueLabel, py::arg("element"), py::arg("elabel"))
    .def("type", &smtk::attribute::ModelEntityItemDefinition::type)
    .def("usingCommonLabel", &smtk::attribute::ModelEntityItemDefinition::usingCommonLabel)
    .def("valueLabel", &smtk::attribute::ModelEntityItemDefinition::valueLabel, py::arg("element"))
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::ModelEntityItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::ModelEntityItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
