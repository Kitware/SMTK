//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_FileSystemItemDefinition_h
#define pybind_smtk_attribute_FileSystemItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/FileSystemItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::FileSystemItemDefinition, smtk::attribute::ItemDefinition > pybind11_init_smtk_attribute_FileSystemItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::FileSystemItemDefinition, smtk::attribute::ItemDefinition > instance(m, "FileSystemItemDefinition");
  instance
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::FileSystemItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::FileSystemItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::FileSystemItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::FileSystemItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("classname", &smtk::attribute::FileSystemItemDefinition::classname)
    .def("createCopy", &smtk::attribute::FileSystemItemDefinition::createCopy, py::arg("info"))
    .def("defaultValue", &smtk::attribute::FileSystemItemDefinition::defaultValue)
    .def("hasDefault", &smtk::attribute::FileSystemItemDefinition::hasDefault)
    .def("hasValueLabels", &smtk::attribute::FileSystemItemDefinition::hasValueLabels)
    .def("isExtensible", &smtk::attribute::FileSystemItemDefinition::isExtensible)
    .def("isValueValid", &smtk::attribute::FileSystemItemDefinition::isValueValid, py::arg("val"))
    .def("maxNumberOfValues", &smtk::attribute::FileSystemItemDefinition::maxNumberOfValues)
    .def("numberOfRequiredValues", &smtk::attribute::FileSystemItemDefinition::numberOfRequiredValues)
    .def("setCommonValueLabel", &smtk::attribute::FileSystemItemDefinition::setCommonValueLabel, py::arg("elabel"))
    .def("setDefaultValue", &smtk::attribute::FileSystemItemDefinition::setDefaultValue, py::arg("val"))
    .def("setIsExtensible", &smtk::attribute::FileSystemItemDefinition::setIsExtensible, py::arg("mode"))
    .def("setMaxNumberOfValues", &smtk::attribute::FileSystemItemDefinition::setMaxNumberOfValues, py::arg("esize"))
    .def("setNumberOfRequiredValues", &smtk::attribute::FileSystemItemDefinition::setNumberOfRequiredValues, py::arg("esize"))
    .def("setShouldBeRelative", &smtk::attribute::FileSystemItemDefinition::setShouldBeRelative, py::arg("val"))
    .def("setShouldExist", &smtk::attribute::FileSystemItemDefinition::setShouldExist, py::arg("val"))
    .def("setValueLabel", &smtk::attribute::FileSystemItemDefinition::setValueLabel, py::arg("element"), py::arg("elabel"))
    .def("shouldBeRelative", &smtk::attribute::FileSystemItemDefinition::shouldBeRelative)
    .def("shouldExist", &smtk::attribute::FileSystemItemDefinition::shouldExist)
    .def("type", &smtk::attribute::FileSystemItemDefinition::type)
    .def("unsetDefaultValue", &smtk::attribute::FileSystemItemDefinition::unsetDefaultValue)
    .def("usingCommonLabel", &smtk::attribute::FileSystemItemDefinition::usingCommonLabel)
    .def("valueLabel", &smtk::attribute::FileSystemItemDefinition::valueLabel, py::arg("element"))
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::FileSystemItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::FileSystemItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
