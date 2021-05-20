//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Events_h
#define pybind_smtk_model_Events_h

#include <pybind11/pybind11.h>

#include "smtk/model/Events.h"

namespace py = pybind11;

inline void pybind11_init_smtk_model_ResourceEventChangeType(py::module &m)
{
  py::enum_<smtk::model::ResourceEventChangeType>(m, "ResourceEventChangeType")
    .value("ADD_EVENT", smtk::model::ResourceEventChangeType::ADD_EVENT)
    .value("MOD_EVENT", smtk::model::ResourceEventChangeType::MOD_EVENT)
    .value("DEL_EVENT", smtk::model::ResourceEventChangeType::DEL_EVENT)
    .value("ANY_EVENT", smtk::model::ResourceEventChangeType::ANY_EVENT)
    .export_values();
}

inline void pybind11_init_smtk_model_ResourceEventRelationType(py::module &m)
{
  py::enum_<smtk::model::ResourceEventRelationType>(m, "ResourceEventRelationType")
    .value("ENTITY_ENTRY", smtk::model::ResourceEventRelationType::ENTITY_ENTRY)
    .value("TESSELLATION_ENTRY", smtk::model::ResourceEventRelationType::TESSELLATION_ENTRY)
    .value("ENTITY_HAS_PROPERTY", smtk::model::ResourceEventRelationType::ENTITY_HAS_PROPERTY)
    .value("ENTITY_HAS_ATTRIBUTE", smtk::model::ResourceEventRelationType::ENTITY_HAS_ATTRIBUTE)
    .value("SESSION_INCLUDES_MODEL", smtk::model::ResourceEventRelationType::SESSION_INCLUDES_MODEL)
    .value("MODEL_INCLUDES_FREE_CELL", smtk::model::ResourceEventRelationType::MODEL_INCLUDES_FREE_CELL)
    .value("MODEL_INCLUDES_FREE_USE", smtk::model::ResourceEventRelationType::MODEL_INCLUDES_FREE_USE)
    .value("MODEL_INCLUDES_FREE_SHELL", smtk::model::ResourceEventRelationType::MODEL_INCLUDES_FREE_SHELL)
    .value("MODEL_INCLUDES_FREE_AUX_GEOM", smtk::model::ResourceEventRelationType::MODEL_INCLUDES_FREE_AUX_GEOM)
    .value("MODEL_INCLUDES_MODEL", smtk::model::ResourceEventRelationType::MODEL_INCLUDES_MODEL)
    .value("MODEL_INCLUDES_GROUP", smtk::model::ResourceEventRelationType::MODEL_INCLUDES_GROUP)
    .value("CELL_INCLUDES_CELL", smtk::model::ResourceEventRelationType::CELL_INCLUDES_CELL)
    .value("CELL_HAS_USE", smtk::model::ResourceEventRelationType::CELL_HAS_USE)
    .value("SHELL_HAS_USE", smtk::model::ResourceEventRelationType::SHELL_HAS_USE)
    .value("SHELL_INCLUDES_SHELL", smtk::model::ResourceEventRelationType::SHELL_INCLUDES_SHELL)
    .value("GROUP_SUPERSET_OF_ENTITY", smtk::model::ResourceEventRelationType::GROUP_SUPERSET_OF_ENTITY)
    .value("MODEL_SUPERSET_OF_MODEL", smtk::model::ResourceEventRelationType::MODEL_SUPERSET_OF_MODEL)
    .value("MODEL_SUPERSET_OF_GROUP", smtk::model::ResourceEventRelationType::MODEL_SUPERSET_OF_GROUP)
    .value("SESSION_SUPERSET_OF_MODEL", smtk::model::ResourceEventRelationType::SESSION_SUPERSET_OF_MODEL)
    .value("AUX_GEOM_INCLUDES_AUX_GEOM", smtk::model::ResourceEventRelationType::AUX_GEOM_INCLUDES_AUX_GEOM)
    .value("INSTANCE_OF_ENTITY", smtk::model::ResourceEventRelationType::INSTANCE_OF_ENTITY)
    .value("INVALID_RELATIONSHIP", smtk::model::ResourceEventRelationType::INVALID_RELATIONSHIP)
    .export_values();
}

#endif
