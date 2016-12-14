//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_ArrangementKind_h
#define pybind_smtk_model_ArrangementKind_h

#include <pybind11/pybind11.h>

#include "smtk/model/ArrangementKind.h"

#include "smtk/model/EntityTypeBits.h"

namespace py = pybind11;

void pybind11_init_smtk_model_Orientation(py::module &m)
{
  py::enum_<smtk::model::Orientation>(m, "Orientation")
    .value("NEGATIVE", smtk::model::Orientation::NEGATIVE)
    .value("POSITIVE", smtk::model::Orientation::POSITIVE)
    .value("UNDEFINED", smtk::model::Orientation::UNDEFINED)
    .export_values();
}

void pybind11_init_smtk_model_ArrangementKind(py::module &m)
{
  py::enum_<smtk::model::ArrangementKind>(m, "ArrangementKind")
    .value("INCLUDES", smtk::model::ArrangementKind::INCLUDES)
    .value("HAS_CELL", smtk::model::ArrangementKind::HAS_CELL)
    .value("HAS_SHELL", smtk::model::ArrangementKind::HAS_SHELL)
    .value("HAS_USE", smtk::model::ArrangementKind::HAS_USE)
    .value("EMBEDDED_IN", smtk::model::ArrangementKind::EMBEDDED_IN)
    .value("SUBSET_OF", smtk::model::ArrangementKind::SUBSET_OF)
    .value("SUPERSET_OF", smtk::model::ArrangementKind::SUPERSET_OF)
    .value("INSTANCE_OF", smtk::model::ArrangementKind::INSTANCE_OF)
    .value("INSTANCED_BY", smtk::model::ArrangementKind::INSTANCED_BY)
    .value("KINDS_OF_ARRANGEMENTS", smtk::model::ArrangementKind::KINDS_OF_ARRANGEMENTS)
    .export_values();
}

void pybind11_init_smtk_model_ArrangementKindFromName(py::module &m)
{
  m.def("ArrangementKindFromName", &smtk::model::ArrangementKindFromName, "", py::arg("name"));
}

void pybind11_init_smtk_model_NameForArrangementKind(py::module &m)
{
  m.def("NameForArrangementKind", &smtk::model::NameForArrangementKind, "", py::arg("k"));
}

void pybind11_init_smtk_model_ArrangementKindFromAbbreviation(py::module &m)
{
  m.def("ArrangementKindFromAbbreviation", &smtk::model::ArrangementKindFromAbbreviation, "", py::arg("abbr"));
}

void pybind11_init_smtk_model_AbbreviationForArrangementKind(py::module &m)
{
  m.def("AbbreviationForArrangementKind", &smtk::model::AbbreviationForArrangementKind, "", py::arg("k"));
}

void pybind11_init_smtk_model_Dual(py::module &m)
{
  m.def("Dual", &smtk::model::Dual, "", py::arg("entType"), py::arg("k"));
}

#endif
