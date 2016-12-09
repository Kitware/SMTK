//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Model_h
#define pybind_smtk_model_Model_h

#include <pybind11/pybind11.h>

#include "smtk/model/Model.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SessionRef.h"

namespace py = pybind11;

py::class_< smtk::model::Model, smtk::model::EntityRef > pybind11_init_smtk_model_Model(py::module &m)
{
  py::class_< smtk::model::Model, smtk::model::EntityRef > instance(m, "Model");
  instance
    .def(py::init<::smtk::model::Model const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::Model::*)(::smtk::model::EntityRef const &) const) &smtk::model::Model::operator!=)
    .def("deepcopy", (smtk::model::Model & (smtk::model::Model::*)(::smtk::model::Model const &)) &smtk::model::Model::operator=)
    .def("__eq__", (bool (smtk::model::Model::*)(::smtk::model::EntityRef const &) const) &smtk::model::Model::operator==)
    .def("addAuxiliaryGeometry", &smtk::model::Model::addAuxiliaryGeometry, py::arg("ag"))
    .def("addCell", &smtk::model::Model::addCell, py::arg("c"))
    .def("addGroup", &smtk::model::Model::addGroup, py::arg("g"))
    .def("addSubmodel", &smtk::model::Model::addSubmodel, py::arg("m"))
    .def("assignDefaultNames", &smtk::model::Model::assignDefaultNames)
    .def("auxiliaryGeometry", &smtk::model::Model::auxiliaryGeometry)
    .def("cells", &smtk::model::Model::cells)
    .def("classname", &smtk::model::Model::classname)
    .def("entitiesWithTessellation", &smtk::model::Model::entitiesWithTessellation)
    .def("geometryStyle", &smtk::model::Model::geometryStyle)
    .def("groups", &smtk::model::Model::groups)
    .def("isValid", (bool (smtk::model::Model::*)() const) &smtk::model::Model::isValid)
    // .def("isValid", (bool (smtk::model::Model::*)(::smtk::model::Entity * *) const) &smtk::model::Model::isValid, py::arg("entRec"))
    .def("op", &smtk::model::Model::op, py::arg("operatorName"))
    .def("operatorNames", &smtk::model::Model::operatorNames)
    .def("parent", &smtk::model::Model::parent)
    .def("removeAuxiliaryGeometry", &smtk::model::Model::removeAuxiliaryGeometry, py::arg("ag"))
    .def("removeCell", &smtk::model::Model::removeCell, py::arg("c"))
    .def("removeGroup", &smtk::model::Model::removeGroup, py::arg("g"))
    .def("removeSubmodel", &smtk::model::Model::removeSubmodel, py::arg("m"))
    .def("session", &smtk::model::Model::session)
    .def("setEmbeddingDimension", &smtk::model::Model::setEmbeddingDimension, py::arg("dim"))
    .def("setSession", &smtk::model::Model::setSession, py::arg("sess"))
    .def("submodels", &smtk::model::Model::submodels)
    ;
  return instance;
}

#endif
