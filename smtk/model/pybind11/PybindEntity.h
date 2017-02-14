//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Entity_h
#define pybind_smtk_model_Entity_h

#include <pybind11/pybind11.h>

#include "smtk/model/Entity.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

py::class_< smtk::model::Entity > pybind11_init_smtk_model_Entity(py::module &m)
{
  py::class_< smtk::model::Entity > instance(m, "Entity");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::BitFlags, int>())
    .def(py::init<::smtk::model::Entity const &>())
    .def("deepcopy", (smtk::model::Entity & (smtk::model::Entity::*)(::smtk::model::Entity const &)) &smtk::model::Entity::operator=)
    .def("dimension", &smtk::model::Entity::dimension)
    .def("dimensionBits", &smtk::model::Entity::dimensionBits)
    .def("entityFlags", &smtk::model::Entity::entityFlags)
    .def("setEntityFlags", &smtk::model::Entity::setEntityFlags, py::arg("flags"))
    .def("relations", (smtk::common::UUIDArray & (smtk::model::Entity::*)()) &smtk::model::Entity::relations)
    .def("relations", (smtk::common::UUIDArray const & (smtk::model::Entity::*)() const) &smtk::model::Entity::relations)
    .def("appendRelation", &smtk::model::Entity::appendRelation, py::arg("b"), py::arg("useHoles") = true)
    .def("pushRelation", &smtk::model::Entity::pushRelation, py::arg("b"))
    .def("removeRelation", &smtk::model::Entity::removeRelation, py::arg("b"))
    .def("resetRelations", &smtk::model::Entity::resetRelations)
    .def("findOrAppendRelation", &smtk::model::Entity::findOrAppendRelation, py::arg("r"))
    .def("invalidateRelation", &smtk::model::Entity::invalidateRelation, py::arg("r"))
    .def("invalidateRelationByIndex", &smtk::model::Entity::invalidateRelationByIndex, py::arg("relIdx"))
    .def("flagSummary", (std::string (smtk::model::Entity::*)(int) const) &smtk::model::Entity::flagSummary, py::arg("form") = 0)
    .def("flagDescription", (std::string (smtk::model::Entity::*)(int) const) &smtk::model::Entity::flagDescription, py::arg("form") = 0)
    .def_static("flagDimensionList", &smtk::model::Entity::flagDimensionList, py::arg("entityFlags"), py::arg("plural"))
    .def_static("flagSummaryHelper", &smtk::model::Entity::flagSummaryHelper, py::arg("entityFlags"), py::arg("form") = 0)
    .def_static("flagSummary", (std::string (*)(::smtk::model::BitFlags, int)) &smtk::model::Entity::flagSummary, py::arg("entityFlags"), py::arg("form") = 0)
    .def_static("flagDescription", (std::string (*)(::smtk::model::BitFlags, int)) &smtk::model::Entity::flagDescription, py::arg("entityFlags"), py::arg("form") = 0)
    .def_static("countForType", &smtk::model::Entity::countForType, py::arg("flags"), py::arg("counters"), py::arg("incr") = false)
    .def_static("defaultNameFromCounters", &smtk::model::Entity::defaultNameFromCounters, py::arg("entityFlags"), py::arg("counters"), py::arg("incr") = true)
    .def_static("flagToSpecifierString", &smtk::model::Entity::flagToSpecifierString, py::arg("flagsOrMask"), py::arg("textual") = true)
    .def_static("specifierStringToFlag", &smtk::model::Entity::specifierStringToFlag, py::arg("spec"))
    .def_static("dimensionToDimensionBits", &smtk::model::Entity::dimensionToDimensionBits, py::arg("dim"))
    ;
  return instance;
}

#endif
