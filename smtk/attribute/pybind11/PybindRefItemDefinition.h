//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_RefItemDefinition_h
#define pybind_smtk_attribute_RefItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/RefItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::RefItemDefinition, smtk::attribute::ItemDefinition > pybind11_init_smtk_attribute_RefItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::RefItemDefinition, smtk::attribute::ItemDefinition > instance(m, "RefItemDefinition");
  instance
    .def(py::init<::smtk::attribute::RefItemDefinition const &>())
    .def_static("New", &smtk::attribute::RefItemDefinition::New, py::arg("myName"))
    .def("attributeDefinition", &smtk::attribute::RefItemDefinition::attributeDefinition)
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::RefItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::RefItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::RefItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::RefItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::RefItemDefinition::classname)
    .def("createCopy", &smtk::attribute::RefItemDefinition::createCopy, py::arg("info"))
    .def("hasValueLabels", &smtk::attribute::RefItemDefinition::hasValueLabels)
    .def("isValueValid", &smtk::attribute::RefItemDefinition::isValueValid, py::arg("att"))
    .def("numberOfRequiredValues", &smtk::attribute::RefItemDefinition::numberOfRequiredValues)
    .def("setAttributeDefinition", &smtk::attribute::RefItemDefinition::setAttributeDefinition, py::arg("def"))
    .def("setCommonValueLabel", &smtk::attribute::RefItemDefinition::setCommonValueLabel, py::arg("elabel"))
    .def("setNumberOfRequiredValues", &smtk::attribute::RefItemDefinition::setNumberOfRequiredValues, py::arg("esize"))
    .def("setValueLabel", &smtk::attribute::RefItemDefinition::setValueLabel, py::arg("element"), py::arg("elabel"))
    .def("type", &smtk::attribute::RefItemDefinition::type)
    .def("usingCommonLabel", &smtk::attribute::RefItemDefinition::usingCommonLabel)
    .def("valueLabel", &smtk::attribute::RefItemDefinition::valueLabel, py::arg("element"))
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::RefItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::RefItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
