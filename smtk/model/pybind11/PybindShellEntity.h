//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_ShellEntity_h
#define pybind_smtk_model_ShellEntity_h

#include <pybind11/pybind11.h>

#include "smtk/model/ShellEntity.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/UseEntity.h"

namespace py = pybind11;

py::class_< smtk::model::ShellEntity, smtk::model::EntityRef > pybind11_init_smtk_model_ShellEntity(py::module &m)
{
  py::class_< smtk::model::ShellEntity, smtk::model::EntityRef > instance(m, "ShellEntity");
  instance
    .def(py::init<::smtk::model::ShellEntity const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::ShellEntity::*)(::smtk::model::EntityRef const &) const) &smtk::model::ShellEntity::operator!=)
    .def("deepcopy", (smtk::model::ShellEntity & (smtk::model::ShellEntity::*)(::smtk::model::ShellEntity const &)) &smtk::model::ShellEntity::operator=)
    .def("__eq__", (bool (smtk::model::ShellEntity::*)(::smtk::model::EntityRef const &) const) &smtk::model::ShellEntity::operator==)
    .def("addUse", &smtk::model::ShellEntity::addUse, py::arg("use"))
    .def("boundingCell", &smtk::model::ShellEntity::boundingCell)
    .def("boundingUseEntity", &smtk::model::ShellEntity::boundingUseEntity)
    .def("classname", &smtk::model::ShellEntity::classname)
    .def("containingShellEntity", &smtk::model::ShellEntity::containingShellEntity)
    .def("contains", &smtk::model::ShellEntity::contains, py::arg("bdyUse"))
    .def("isValid", (bool (smtk::model::ShellEntity::*)() const) &smtk::model::ShellEntity::isValid)
    // .def("isValid", (bool (smtk::model::ShellEntity::*)(::smtk::model::Entity * *) const) &smtk::model::ShellEntity::isValid, py::arg("entRec"))
    ;
  return instance;
}

#endif
