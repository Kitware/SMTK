//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Loop_h
#define pybind_smtk_model_Loop_h

#include <pybind11/pybind11.h>

#include "smtk/model/Loop.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ShellEntity.h"

namespace py = pybind11;

py::class_< smtk::model::Loop, smtk::model::ShellEntity > pybind11_init_smtk_model_Loop(py::module &m)
{
  py::class_< smtk::model::Loop, smtk::model::ShellEntity > instance(m, "Loop");
  instance
    .def(py::init<::smtk::model::Loop const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::Loop::*)(::smtk::model::EntityRef const &) const) &smtk::model::Loop::operator!=)
    .def("deepcopy", (smtk::model::Loop & (smtk::model::Loop::*)(::smtk::model::Loop const &)) &smtk::model::Loop::operator=)
    .def("__eq__", (bool (smtk::model::Loop::*)(::smtk::model::EntityRef const &) const) &smtk::model::Loop::operator==)
    .def("classname", &smtk::model::Loop::classname)
    .def("containedLoops", &smtk::model::Loop::containedLoops)
    .def("containingLoop", &smtk::model::Loop::containingLoop)
    .def("edgeUses", &smtk::model::Loop::edgeUses)
    .def("face", &smtk::model::Loop::face)
    .def("faceUse", &smtk::model::Loop::faceUse)
    .def("isValid", (bool (smtk::model::Loop::*)() const) &smtk::model::Loop::isValid)
    // .def("isValid", (bool (smtk::model::Loop::*)(::smtk::model::Entity * *) const) &smtk::model::Loop::isValid, py::arg("entRec"))
    .def("replaceEdgeUseWithUses", &smtk::model::Loop::replaceEdgeUseWithUses, py::arg("original"), py::arg("replacements"))
    ;
  return instance;
}

#endif
