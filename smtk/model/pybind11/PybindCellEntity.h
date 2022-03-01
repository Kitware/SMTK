//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_CellEntity_h
#define pybind_smtk_model_CellEntity_h

#include <pybind11/pybind11.h>

#include "smtk/model/CellEntity.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Model.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/UseEntity.h"

namespace py = pybind11;

inline py::class_< smtk::model::CellEntity, smtk::model::EntityRef > pybind11_init_smtk_model_CellEntity(py::module &m)
{
  py::class_< smtk::model::CellEntity, smtk::model::EntityRef > instance(m, "CellEntity");
  instance
    .def(py::init<::smtk::model::CellEntity const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityPtr>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ResourcePtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::CellEntity::*)(::smtk::model::EntityRef const &) const) &smtk::model::CellEntity::operator!=)
    .def("deepcopy", (smtk::model::CellEntity & (smtk::model::CellEntity::*)(::smtk::model::CellEntity const &)) &smtk::model::CellEntity::operator=)
    .def("__eq__", (bool (smtk::model::CellEntity::*)(::smtk::model::EntityRef const &) const) &smtk::model::CellEntity::operator==)
    .def("__hash__", &smtk::model::CellEntity::hash)
    .def("boundingCellUses", &smtk::model::CellEntity::boundingCellUses, py::arg("orientation"))
    .def("boundingCells", &smtk::model::CellEntity::boundingCells)
    .def("findShellEntitiesContainingCell", &smtk::model::CellEntity::findShellEntitiesContainingCell, py::arg("cell"))
    .def("findShellEntityContainingUse", &smtk::model::CellEntity::findShellEntityContainingUse, py::arg("bdyUse"))
    .def("isValid", (bool (smtk::model::CellEntity::*)() const) &smtk::model::CellEntity::isValid)
    // .def("isValid", (bool (smtk::model::CellEntity::*)(::smtk::model::Entity * *) const) &smtk::model::CellEntity::isValid, py::arg("entRec"))
    .def("model", &smtk::model::CellEntity::model)
    ;
  return instance;
}

#endif
