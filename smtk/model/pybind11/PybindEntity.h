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
#include "smtk/model/Resource.h"
#include "smtk/resource/Component.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::model::Entity, smtk::resource::Component > pybind11_init_smtk_model_Entity(py::module &m)
{
  PySharedPtrClass< smtk::model::Entity, smtk::resource::Component > instance(m, "Entity");
  instance
    .def(py::init<::smtk::model::Entity const &>())
    .def("deepcopy", (smtk::model::Entity & (smtk::model::Entity::*)(::smtk::model::Entity const &)) &smtk::model::Entity::operator=)
    .def("shared_from_this", (std::shared_ptr<smtk::model::Entity> (smtk::model::Entity::*)()) &smtk::model::Entity::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::Entity> (smtk::model::Entity::*)() const) &smtk::model::Entity::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::model::Entity> (*)()) &smtk::model::Entity::create)
    .def_static("create", (std::shared_ptr<smtk::model::Entity> (*)(::std::shared_ptr<smtk::model::Entity> &)) &smtk::model::Entity::create, py::arg("ref"))
    .def_static("create", (smtk::model::EntityPtr (*)(::smtk::model::BitFlags, int, ::smtk::model::ResourcePtr)) &smtk::model::Entity::create, py::arg("entityFlags"), py::arg("dimension"), py::arg("resource") = nullptr)
    .def("setup", &smtk::model::Entity::setup, py::arg("entityFlags"), py::arg("dimension"), py::arg("resource") = nullptr, py::arg("resetRelations") = true)
    .def("resource", &smtk::model::Entity::resource)
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
    .def_static("countForType", &smtk::model::Entity::countForType, py::arg("flags"), py::arg("counters"), py::arg("incr") = false)
    .def_static("defaultNameFromCounters", &smtk::model::Entity::defaultNameFromCounters, py::arg("entityFlags"), py::arg("counters"), py::arg("incr") = true)
    .def_static("flagToSpecifierString", &smtk::model::Entity::flagToSpecifierString, py::arg("flagsOrMask"), py::arg("textual") = true)
    .def("modelResource", &smtk::model::Entity::modelResource)
    .def("resource", &smtk::model::Entity::resource)
    .def_static("specifierStringToFlag", &smtk::model::Entity::specifierStringToFlag, py::arg("spec"))
    .def_static("dimensionToDimensionBits", &smtk::model::Entity::dimensionToDimensionBits, py::arg("dim"))
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Component> i) {
        return std::dynamic_pointer_cast<smtk::model::Entity>(i);
      })
    ;
  return instance;
}

#endif
