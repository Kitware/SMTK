//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Arrangement_h
#define pybind_smtk_model_Arrangement_h

#include <pybind11/pybind11.h>

#include "smtk/model/Arrangement.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/ArrangementKind.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityTypeBits.h"

namespace py = pybind11;

py::class_< smtk::model::Arrangement > pybind11_init_smtk_model_Arrangement(py::module &m)
{
  py::class_< smtk::model::Arrangement > instance(m, "Arrangement");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::Arrangement const &>())
    .def("deepcopy", (smtk::model::Arrangement & (smtk::model::Arrangement::*)(::smtk::model::Arrangement const &)) &smtk::model::Arrangement::operator=)
    .def_static("CellEmbeddedInEntityWithIndex", &smtk::model::Arrangement::CellEmbeddedInEntityWithIndex, py::arg("relationIdx"))
    .def_static("CellHasShellWithIndex", &smtk::model::Arrangement::CellHasShellWithIndex, py::arg("relationIdx"))
    .def_static("CellHasUseWithIndexSenseAndOrientation", &smtk::model::Arrangement::CellHasUseWithIndexSenseAndOrientation, py::arg("relationIdx"), py::arg("sense"), py::arg("o"))
    .def_static("CellIncludesEntityWithIndex", &smtk::model::Arrangement::CellIncludesEntityWithIndex, py::arg("relationIdx"))
    .def_static("Construct", &smtk::model::Arrangement::Construct, py::arg("t"), py::arg("k"), py::arg("relationIdx"), py::arg("sense"), py::arg("o"))
    .def_static("EntityInstancedByWithIndex", &smtk::model::Arrangement::EntityInstancedByWithIndex, py::arg("relationIdx"))
    .def_static("EntitySubsetOfWithIndex", &smtk::model::Arrangement::EntitySubsetOfWithIndex, py::arg("relationIdx"))
    .def_static("EntitySupersetOfWithIndex", &smtk::model::Arrangement::EntitySupersetOfWithIndex, py::arg("relationIdx"))
    .def("IndexAndSenseFromUseHasCell", &smtk::model::Arrangement::IndexAndSenseFromUseHasCell, py::arg("relationIdx"), py::arg("sense"))
    .def("IndexFromAuxiliaryGeometryEmbeddedInEntity", &smtk::model::Arrangement::IndexFromAuxiliaryGeometryEmbeddedInEntity, py::arg("relationIdx"))
    .def("IndexFromAuxiliaryGeometryIncludesEntity", &smtk::model::Arrangement::IndexFromAuxiliaryGeometryIncludesEntity, py::arg("relationIdx"))
    .def("IndexFromCellEmbeddedInEntity", &smtk::model::Arrangement::IndexFromCellEmbeddedInEntity, py::arg("relationIdx"))
    .def("IndexFromCellHasShell", &smtk::model::Arrangement::IndexFromCellHasShell, py::arg("relationIdx"))
    .def("IndexFromCellIncludesEntity", &smtk::model::Arrangement::IndexFromCellIncludesEntity, py::arg("relationIdx"))
    .def("IndexFromEntityInstancedBy", &smtk::model::Arrangement::IndexFromEntityInstancedBy, py::arg("relationIdx"))
    .def("IndexFromEntitySubsetOf", &smtk::model::Arrangement::IndexFromEntitySubsetOf, py::arg("relationIdx"))
    .def("IndexFromEntitySupersetOf", &smtk::model::Arrangement::IndexFromEntitySupersetOf, py::arg("relationIdx"))
    .def("IndexFromInstanceInstanceOf", &smtk::model::Arrangement::IndexFromInstanceInstanceOf, py::arg("relationIdx"))
    .def("IndexFromShellEmbeddedInUseOrShell", &smtk::model::Arrangement::IndexFromShellEmbeddedInUseOrShell, py::arg("relationIdx"))
    .def("IndexFromShellHasCell", &smtk::model::Arrangement::IndexFromShellHasCell, py::arg("relationIdx"))
    .def("IndexFromSimple", &smtk::model::Arrangement::IndexFromSimple, py::arg("relationIdx"))
    .def("IndexFromUseHasShell", &smtk::model::Arrangement::IndexFromUseHasShell, py::arg("relationIdx"))
    .def("IndexFromUseOrShellIncludesShell", &smtk::model::Arrangement::IndexFromUseOrShellIncludesShell, py::arg("relationIdx"))
    .def("IndexRangeFromShellHasUse", &smtk::model::Arrangement::IndexRangeFromShellHasUse, py::arg("relationBegin"), py::arg("relationEnd"))
    .def("IndexSenseAndOrientationFromCellHasUse", &smtk::model::Arrangement::IndexSenseAndOrientationFromCellHasUse, py::arg("relationIdx"), py::arg("sense"), py::arg("orient"))
    .def_static("InstanceInstanceOfWithIndex", &smtk::model::Arrangement::InstanceInstanceOfWithIndex, py::arg("relationIdx"))
    .def_static("ShellEmbeddedInUseOrShellWithIndex", &smtk::model::Arrangement::ShellEmbeddedInUseOrShellWithIndex, py::arg("relationIdx"))
    .def_static("ShellHasCellWithIndex", &smtk::model::Arrangement::ShellHasCellWithIndex, py::arg("relationIdx"))
    .def_static("ShellHasUseWithIndexRange", &smtk::model::Arrangement::ShellHasUseWithIndexRange, py::arg("relationBegin"), py::arg("relationEnd"))
    .def_static("SimpleIndex", &smtk::model::Arrangement::SimpleIndex, py::arg("relationIdx"))
    .def_static("UseHasCellWithIndexAndSense", &smtk::model::Arrangement::UseHasCellWithIndexAndSense, py::arg("relationIdx"), py::arg("sense"))
    .def_static("UseHasShellWithIndex", &smtk::model::Arrangement::UseHasShellWithIndex, py::arg("relationIdx"))
    .def_static("UseOrShellIncludesShellWithIndex", &smtk::model::Arrangement::UseOrShellIncludesShellWithIndex, py::arg("relationIdx"))
    .def("details", (std::vector<int, std::allocator<int> > & (smtk::model::Arrangement::*)()) &smtk::model::Arrangement::details)
    .def("details", (std::vector<int, std::allocator<int> > const & (smtk::model::Arrangement::*)() const) &smtk::model::Arrangement::details)
    .def("relationIndices", &smtk::model::Arrangement::relationIndices, py::arg("relsOut"), py::arg("ent"), py::arg("k"))
    .def("relations", &smtk::model::Arrangement::relations, py::arg("relsOut"), py::arg("ent"), py::arg("k"))
    ;
  return instance;
}

py::class_< smtk::model::ArrangementReference > pybind11_init_smtk_model_ArrangementReference(py::module &m)
{
  py::class_< smtk::model::ArrangementReference > instance(m, "ArrangementReference");
  instance
    .def(py::init<::smtk::model::ArrangementReference const &>())
    .def(py::init<::smtk::common::UUID const &, ::smtk::model::ArrangementKind, int>())
    .def(py::init<>())
    .def("deepcopy", (smtk::model::ArrangementReference & (smtk::model::ArrangementReference::*)(::smtk::model::ArrangementReference const &)) &smtk::model::ArrangementReference::operator=)
    .def("isValid", &smtk::model::ArrangementReference::isValid)
    .def_readwrite("entityId", &smtk::model::ArrangementReference::entityId)
    .def_readwrite("index", &smtk::model::ArrangementReference::index)
    .def_readwrite("kind", &smtk::model::ArrangementReference::kind)
    ;
  return instance;
}

#endif
