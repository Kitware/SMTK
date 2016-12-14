//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_UseEntity_h
#define pybind_smtk_model_UseEntity_h

#include <pybind11/pybind11.h>

#include "smtk/model/UseEntity.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ShellEntity.h"

namespace py = pybind11;

py::class_< smtk::model::UseEntity, smtk::model::EntityRef > pybind11_init_smtk_model_UseEntity(py::module &m)
{
  py::class_< smtk::model::UseEntity, smtk::model::EntityRef > instance(m, "UseEntity");
  instance
    .def(py::init<::smtk::model::UseEntity const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::UseEntity::*)(::smtk::model::EntityRef const &) const) &smtk::model::UseEntity::operator!=)
    .def("deepcopy", (smtk::model::UseEntity & (smtk::model::UseEntity::*)(::smtk::model::UseEntity const &)) &smtk::model::UseEntity::operator=)
    .def("__eq__", (bool (smtk::model::UseEntity::*)(::smtk::model::EntityRef const &) const) &smtk::model::UseEntity::operator==)
    .def("addShellEntity", &smtk::model::UseEntity::addShellEntity, py::arg("shell"))
    .def("boundingShellEntity", &smtk::model::UseEntity::boundingShellEntity)
    .def("cell", &smtk::model::UseEntity::cell)
    .def("classname", &smtk::model::UseEntity::classname)
    .def("isValid", (bool (smtk::model::UseEntity::*)() const) &smtk::model::UseEntity::isValid)
    // .def("isValid", (bool (smtk::model::UseEntity::*)(::smtk::model::Entity * *) const) &smtk::model::UseEntity::isValid, py::arg("entRec"))
    .def("orientation", &smtk::model::UseEntity::orientation)
    .def("sense", &smtk::model::UseEntity::sense)
    .def("setBoundingShellEntity", &smtk::model::UseEntity::setBoundingShellEntity, py::arg("shell"))
    ;
  return instance;
}

#endif
