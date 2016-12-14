//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_EntityTypeBits_h
#define pybind_smtk_model_EntityTypeBits_h

#include <pybind11/pybind11.h>

#include "smtk/model/EntityTypeBits.h"

namespace py = pybind11;

void pybind11_init_smtk_model_EntityTypeBits(py::module &m)
{
  py::enum_<smtk::model::EntityTypeBits>(m, "EntityTypeBits")
    .value("DIMENSION_0", smtk::model::EntityTypeBits::DIMENSION_0)
    .value("DIMENSION_1", smtk::model::EntityTypeBits::DIMENSION_1)
    .value("DIMENSION_2", smtk::model::EntityTypeBits::DIMENSION_2)
    .value("DIMENSION_3", smtk::model::EntityTypeBits::DIMENSION_3)
    .value("DIMENSION_4", smtk::model::EntityTypeBits::DIMENSION_4)
    .value("CELL_ENTITY", smtk::model::EntityTypeBits::CELL_ENTITY)
    .value("USE_ENTITY", smtk::model::EntityTypeBits::USE_ENTITY)
    .value("SHELL_ENTITY", smtk::model::EntityTypeBits::SHELL_ENTITY)
    .value("GROUP_ENTITY", smtk::model::EntityTypeBits::GROUP_ENTITY)
    .value("MODEL_ENTITY", smtk::model::EntityTypeBits::MODEL_ENTITY)
    .value("INSTANCE_ENTITY", smtk::model::EntityTypeBits::INSTANCE_ENTITY)
    .value("SESSION", smtk::model::EntityTypeBits::SESSION)
    .value("AUX_GEOM_ENTITY", smtk::model::EntityTypeBits::AUX_GEOM_ENTITY)
    .value("CONCEPT_ENTITY", smtk::model::EntityTypeBits::CONCEPT_ENTITY)
    .value("SURFACE_ENTITY", smtk::model::EntityTypeBits::SURFACE_ENTITY)
    .value("COVER", smtk::model::EntityTypeBits::COVER)
    .value("PARTITION", smtk::model::EntityTypeBits::PARTITION)
    .value("OPEN", smtk::model::EntityTypeBits::OPEN)
    .value("CLOSED", smtk::model::EntityTypeBits::CLOSED)
    .value("MODEL_BOUNDARY", smtk::model::EntityTypeBits::MODEL_BOUNDARY)
    .value("MODEL_DOMAIN", smtk::model::EntityTypeBits::MODEL_DOMAIN)
    .value("HOMOGENOUS_GROUP", smtk::model::EntityTypeBits::HOMOGENOUS_GROUP)
    .value("NO_SUBGROUPS", smtk::model::EntityTypeBits::NO_SUBGROUPS)
    .value("ANY_DIMENSION", smtk::model::EntityTypeBits::ANY_DIMENSION)
    .value("ENTITY_MASK", smtk::model::EntityTypeBits::ENTITY_MASK)
    .value("ANY_ENTITY", smtk::model::EntityTypeBits::ANY_ENTITY)
    .value("VERTEX", smtk::model::EntityTypeBits::VERTEX)
    .value("EDGE", smtk::model::EntityTypeBits::EDGE)
    .value("FACE", smtk::model::EntityTypeBits::FACE)
    .value("VOLUME", smtk::model::EntityTypeBits::VOLUME)
    .value("CELL_0D", smtk::model::EntityTypeBits::CELL_0D)
    .value("CELL_1D", smtk::model::EntityTypeBits::CELL_1D)
    .value("CELL_2D", smtk::model::EntityTypeBits::CELL_2D)
    .value("CELL_3D", smtk::model::EntityTypeBits::CELL_3D)
    .value("ANY_CELL", smtk::model::EntityTypeBits::ANY_CELL)
    .value("VERTEX_USE", smtk::model::EntityTypeBits::VERTEX_USE)
    .value("EDGE_USE", smtk::model::EntityTypeBits::EDGE_USE)
    .value("FACE_USE", smtk::model::EntityTypeBits::FACE_USE)
    .value("VOLUME_USE", smtk::model::EntityTypeBits::VOLUME_USE)
    .value("USE_0D", smtk::model::EntityTypeBits::USE_0D)
    .value("USE_1D", smtk::model::EntityTypeBits::USE_1D)
    .value("USE_2D", smtk::model::EntityTypeBits::USE_2D)
    .value("USE_3D", smtk::model::EntityTypeBits::USE_3D)
    .value("ANY_USE", smtk::model::EntityTypeBits::ANY_USE)
    .value("CHAIN", smtk::model::EntityTypeBits::CHAIN)
    .value("LOOP", smtk::model::EntityTypeBits::LOOP)
    .value("SHELL", smtk::model::EntityTypeBits::SHELL)
    .value("SHELL_0D", smtk::model::EntityTypeBits::SHELL_0D)
    .value("SHELL_1D", smtk::model::EntityTypeBits::SHELL_1D)
    .value("SHELL_2D", smtk::model::EntityTypeBits::SHELL_2D)
    .value("ANY_SHELL", smtk::model::EntityTypeBits::ANY_SHELL)
    .value("GROUP_0D", smtk::model::EntityTypeBits::GROUP_0D)
    .value("GROUP_1D", smtk::model::EntityTypeBits::GROUP_1D)
    .value("GROUP_2D", smtk::model::EntityTypeBits::GROUP_2D)
    .value("GROUP_3D", smtk::model::EntityTypeBits::GROUP_3D)
    .value("ANY_GROUP", smtk::model::EntityTypeBits::ANY_GROUP)
    .value("BOUNDARY_GROUP", smtk::model::EntityTypeBits::BOUNDARY_GROUP)
    .value("DOMAIN_GROUP", smtk::model::EntityTypeBits::DOMAIN_GROUP)
    .value("GROUP_CONSTRAINT_MASK", smtk::model::EntityTypeBits::GROUP_CONSTRAINT_MASK)
    .value("HALF_OPEN", smtk::model::EntityTypeBits::HALF_OPEN)
    .value("HALF_CLOSED", smtk::model::EntityTypeBits::HALF_CLOSED)
    .value("INVALID", smtk::model::EntityTypeBits::INVALID)
    .export_values()
    .def("__or__", [](const smtk::model::EntityTypeBits &a, const smtk::model::EntityTypeBits &b) { return smtk::model::EntityTypeBits(a | b); })
    .def("__and__", [](const smtk::model::EntityTypeBits &a, const smtk::model::EntityTypeBits &b) { return smtk::model::EntityTypeBits(a & b); })
    .def("__xor__", [](const smtk::model::EntityTypeBits &a, const smtk::model::EntityTypeBits &b) { return smtk::model::EntityTypeBits(a ^ b); })
    .def("__inv__", [](const smtk::model::EntityTypeBits &a, const smtk::model::EntityTypeBits &b) { return smtk::model::EntityTypeBits(~a); })
    .def("__rshift__", [](const smtk::model::EntityTypeBits &a, const smtk::model::EntityTypeBits &b) { return smtk::model::EntityTypeBits(a >> b); })
    .def("__lshift__", [](const smtk::model::EntityTypeBits &a, const smtk::model::EntityTypeBits &b) { return smtk::model::EntityTypeBits(a << b); })
    .def("__int__", [](const smtk::model::EntityTypeBits &a) { return static_cast<int>(a); })
    ;
}

void pybind11_init_smtk_model_ModelGeometryStyle(py::module &m)
{
  py::enum_<smtk::model::ModelGeometryStyle>(m, "ModelGeometryStyle")
    .value("DISCRETE", smtk::model::ModelGeometryStyle::DISCRETE)
    .value("PARAMETRIC", smtk::model::ModelGeometryStyle::PARAMETRIC)
    .export_values();
}

void pybind11_init_smtk_model_isCellEntity(py::module &m)
{
  m.def("isCellEntity", &smtk::model::isCellEntity, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isVertex(py::module &m)
{
  m.def("isVertex", &smtk::model::isVertex, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isEdge(py::module &m)
{
  m.def("isEdge", &smtk::model::isEdge, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isFace(py::module &m)
{
  m.def("isFace", &smtk::model::isFace, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isVolume(py::module &m)
{
  m.def("isVolume", &smtk::model::isVolume, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isUseEntity(py::module &m)
{
  m.def("isUseEntity", &smtk::model::isUseEntity, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isVertexUse(py::module &m)
{
  m.def("isVertexUse", &smtk::model::isVertexUse, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isEdgeUse(py::module &m)
{
  m.def("isEdgeUse", &smtk::model::isEdgeUse, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isFaceUse(py::module &m)
{
  m.def("isFaceUse", &smtk::model::isFaceUse, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isVolumeUse(py::module &m)
{
  m.def("isVolumeUse", &smtk::model::isVolumeUse, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isShellEntity(py::module &m)
{
  m.def("isShellEntity", &smtk::model::isShellEntity, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isChain(py::module &m)
{
  m.def("isChain", &smtk::model::isChain, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isLoop(py::module &m)
{
  m.def("isLoop", &smtk::model::isLoop, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isShell(py::module &m)
{
  m.def("isShell", &smtk::model::isShell, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isGroup(py::module &m)
{
  m.def("isGroup", &smtk::model::isGroup, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isModel(py::module &m)
{
  m.def("isModel", &smtk::model::isModel, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isInstance(py::module &m)
{
  m.def("isInstance", &smtk::model::isInstance, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isAuxiliaryGeometry(py::module &m)
{
  m.def("isAuxiliaryGeometry", &smtk::model::isAuxiliaryGeometry, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isConcept(py::module &m)
{
  m.def("isConcept", &smtk::model::isConcept, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_isSessionRef(py::module &m)
{
  m.def("isSessionRef", &smtk::model::isSessionRef, "", py::arg("entityFlags"));
}

void pybind11_init_smtk_model_ModelGeometryStyleName(py::module &m)
{
  m.def("ModelGeometryStyleName", &smtk::model::ModelGeometryStyleName, "", py::arg("s"));
}

void pybind11_init_smtk_model_NamedModelGeometryStyle(py::module &m)
{
  m.def("NamedModelGeometryStyle", &smtk::model::NamedModelGeometryStyle, "", py::arg("s"));
}

#endif
