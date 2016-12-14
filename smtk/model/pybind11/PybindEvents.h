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

void pybind11_init_smtk_model_ManagerEventChangeType(py::module &m)
{
  py::enum_<smtk::model::ManagerEventChangeType>(m, "ManagerEventChangeType")
    .value("ADD_EVENT", smtk::model::ManagerEventChangeType::ADD_EVENT)
    .value("MOD_EVENT", smtk::model::ManagerEventChangeType::MOD_EVENT)
    .value("DEL_EVENT", smtk::model::ManagerEventChangeType::DEL_EVENT)
    .value("ANY_EVENT", smtk::model::ManagerEventChangeType::ANY_EVENT)
    .export_values();
}

void pybind11_init_smtk_model_ManagerEventRelationType(py::module &m)
{
  py::enum_<smtk::model::ManagerEventRelationType>(m, "ManagerEventRelationType")
    .value("ENTITY_ENTRY", smtk::model::ManagerEventRelationType::ENTITY_ENTRY)
    .value("TESSELLATION_ENTRY", smtk::model::ManagerEventRelationType::TESSELLATION_ENTRY)
    .value("ENTITY_HAS_PROPERTY", smtk::model::ManagerEventRelationType::ENTITY_HAS_PROPERTY)
    .value("ENTITY_HAS_ATTRIBUTE", smtk::model::ManagerEventRelationType::ENTITY_HAS_ATTRIBUTE)
    .value("SESSION_INCLUDES_MODEL", smtk::model::ManagerEventRelationType::SESSION_INCLUDES_MODEL)
    .value("MODEL_INCLUDES_FREE_CELL", smtk::model::ManagerEventRelationType::MODEL_INCLUDES_FREE_CELL)
    .value("MODEL_INCLUDES_FREE_USE", smtk::model::ManagerEventRelationType::MODEL_INCLUDES_FREE_USE)
    .value("MODEL_INCLUDES_FREE_SHELL", smtk::model::ManagerEventRelationType::MODEL_INCLUDES_FREE_SHELL)
    .value("MODEL_INCLUDES_FREE_AUX_GEOM", smtk::model::ManagerEventRelationType::MODEL_INCLUDES_FREE_AUX_GEOM)
    .value("MODEL_INCLUDES_MODEL", smtk::model::ManagerEventRelationType::MODEL_INCLUDES_MODEL)
    .value("MODEL_INCLUDES_GROUP", smtk::model::ManagerEventRelationType::MODEL_INCLUDES_GROUP)
    .value("CELL_INCLUDES_CELL", smtk::model::ManagerEventRelationType::CELL_INCLUDES_CELL)
    .value("CELL_HAS_USE", smtk::model::ManagerEventRelationType::CELL_HAS_USE)
    .value("SHELL_HAS_USE", smtk::model::ManagerEventRelationType::SHELL_HAS_USE)
    .value("SHELL_INCLUDES_SHELL", smtk::model::ManagerEventRelationType::SHELL_INCLUDES_SHELL)
    .value("GROUP_SUPERSET_OF_ENTITY", smtk::model::ManagerEventRelationType::GROUP_SUPERSET_OF_ENTITY)
    .value("MODEL_SUPERSET_OF_MODEL", smtk::model::ManagerEventRelationType::MODEL_SUPERSET_OF_MODEL)
    .value("MODEL_SUPERSET_OF_GROUP", smtk::model::ManagerEventRelationType::MODEL_SUPERSET_OF_GROUP)
    .value("SESSION_SUPERSET_OF_MODEL", smtk::model::ManagerEventRelationType::SESSION_SUPERSET_OF_MODEL)
    .value("AUX_GEOM_INCLUDES_AUX_GEOM", smtk::model::ManagerEventRelationType::AUX_GEOM_INCLUDES_AUX_GEOM)
    .value("INSTANCE_OF_ENTITY", smtk::model::ManagerEventRelationType::INSTANCE_OF_ENTITY)
    .value("INVALID_RELATIONSHIP", smtk::model::ManagerEventRelationType::INVALID_RELATIONSHIP)
    .export_values();
}

void pybind11_init_smtk_model_OperatorEventType(py::module &m)
{
  py::enum_<smtk::model::OperatorEventType>(m, "OperatorEventType")
    .value("CREATED_OPERATOR", smtk::model::OperatorEventType::CREATED_OPERATOR)
    .value("WILL_OPERATE", smtk::model::OperatorEventType::WILL_OPERATE)
    .value("DID_OPERATE", smtk::model::OperatorEventType::DID_OPERATE)
    .export_values();
}

#endif
