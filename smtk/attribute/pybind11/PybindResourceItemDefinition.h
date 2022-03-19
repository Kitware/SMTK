//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_ResourceItemDefinition_h
#define pybind_smtk_attribute_ResourceItemDefinition_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ResourceItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/resource/Resource.h"

namespace py = pybind11;

inline py::class_< smtk::attribute::ResourceItemDefinition, smtk::attribute::ReferenceItemDefinition > pybind11_init_smtk_attribute_ResourceItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ResourceItemDefinition, smtk::attribute::ReferenceItemDefinition > instance(m, "ResourceItemDefinition");
  instance
    .def(py::init<::smtk::attribute::ResourceItemDefinition const &>())
    .def_static("New", &smtk::attribute::ResourceItemDefinition::New, py::arg("sname"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ResourceItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::ResourceItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::ResourceItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::ResourceItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("createCopy", &smtk::attribute::ResourceItemDefinition::createCopy, py::arg("info"))
    .def("hasValueLabels", &smtk::attribute::ResourceItemDefinition::hasValueLabels)
    .def("isExtensible", &smtk::attribute::ResourceItemDefinition::isExtensible)
    .def("isValueValid", &smtk::attribute::ResourceItemDefinition::isValueValid, py::arg("entity"))
    .def("maxNumberOfValues", &smtk::attribute::ResourceItemDefinition::maxNumberOfValues)
    .def("numberOfRequiredValues", &smtk::attribute::ResourceItemDefinition::numberOfRequiredValues)
    // .def("setAcceptsEntries", (bool (smtk::attribute::ResourceItemDefinition::*)(::smtk::resource::Resource::Index, bool)) &smtk::attribute::ResourceItemDefinition::setAcceptsEntries, py::arg("resourceIndex"), py::arg("accept"))
    .def("setAcceptsEntries", (bool (smtk::attribute::ResourceItemDefinition::*)(const std::string&, const std::string&, bool)) &smtk::attribute::ResourceItemDefinition::setAcceptsEntries, py::arg("typeName"), py::arg("queryString"), py::arg("accept"))
    .def("setAcceptsEntries", (bool (smtk::attribute::ResourceItemDefinition::*)(const std::string&, bool)) &smtk::attribute::ResourceItemDefinition::setAcceptsEntries, py::arg("typeName"), py::arg("accept"))
    .def("setCommonValueLabel", &smtk::attribute::ResourceItemDefinition::setCommonValueLabel, py::arg("elabel"))
    .def("setIsExtensible", &smtk::attribute::ResourceItemDefinition::setIsExtensible, py::arg("extensible"))
    .def("setMaxNumberOfValues", &smtk::attribute::ResourceItemDefinition::setMaxNumberOfValues, py::arg("maxNum"))
    .def("setNumberOfRequiredValues", &smtk::attribute::ResourceItemDefinition::setNumberOfRequiredValues, py::arg("esize"))
    .def("setValueLabel", &smtk::attribute::ResourceItemDefinition::setValueLabel, py::arg("element"), py::arg("elabel"))
    .def("type", &smtk::attribute::ResourceItemDefinition::type)
    .def("usingCommonLabel", &smtk::attribute::ResourceItemDefinition::usingCommonLabel)
    .def("valueLabel", &smtk::attribute::ResourceItemDefinition::valueLabel, py::arg("element"))

    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::ResourceItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::ResourceItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
