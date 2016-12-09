//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Shell_h
#define pybind_smtk_model_Shell_h

#include <pybind11/pybind11.h>

#include "smtk/model/Shell.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/Volume.h"

namespace py = pybind11;

py::class_< smtk::model::Shell, smtk::model::ShellEntity > pybind11_init_smtk_model_Shell(py::module &m)
{
  py::class_< smtk::model::Shell, smtk::model::ShellEntity > instance(m, "Shell");
  instance
    .def(py::init<::smtk::model::Shell const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::Shell::*)(::smtk::model::EntityRef const &) const) &smtk::model::Shell::operator!=)
    .def("deepcopy", (smtk::model::Shell & (smtk::model::Shell::*)(::smtk::model::Shell const &)) &smtk::model::Shell::operator=)
    .def("__eq__", (bool (smtk::model::Shell::*)(::smtk::model::EntityRef const &) const) &smtk::model::Shell::operator==)
    .def("classname", &smtk::model::Shell::classname)
    .def("containedShells", &smtk::model::Shell::containedShells)
    .def("containingShell", &smtk::model::Shell::containingShell)
    .def("faceUses", &smtk::model::Shell::faceUses)
    .def("isValid", (bool (smtk::model::Shell::*)() const) &smtk::model::Shell::isValid)
    // .def("isValid", (bool (smtk::model::Shell::*)(::smtk::model::Entity * *) const) &smtk::model::Shell::isValid, py::arg("entRec"))
    .def("volume", &smtk::model::Shell::volume)
    ;
  return instance;
}

#endif
