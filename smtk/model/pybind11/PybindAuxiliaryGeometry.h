//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_AuxiliaryGeometry_h
#define pybind_smtk_model_AuxiliaryGeometry_h

#include <pybind11/pybind11.h>

#include "smtk/model/AuxiliaryGeometry.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"

namespace py = pybind11;

py::class_< smtk::model::AuxiliaryGeometry, smtk::model::EntityRef > pybind11_init_smtk_model_AuxiliaryGeometry(py::module &m)
{
  py::class_< smtk::model::AuxiliaryGeometry, smtk::model::EntityRef > instance(m, "AuxiliaryGeometry");
  instance
    .def(py::init<::smtk::model::AuxiliaryGeometry const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::AuxiliaryGeometry::*)(::smtk::model::EntityRef const &) const) &smtk::model::AuxiliaryGeometry::operator!=)
    .def("deepcopy", (smtk::model::AuxiliaryGeometry & (smtk::model::AuxiliaryGeometry::*)(::smtk::model::AuxiliaryGeometry const &)) &smtk::model::AuxiliaryGeometry::operator=)
    .def("__eq__", (bool (smtk::model::AuxiliaryGeometry::*)(::smtk::model::EntityRef const &) const) &smtk::model::AuxiliaryGeometry::operator==)
    .def("classname", &smtk::model::AuxiliaryGeometry::classname)
    .def("hasUrl", &smtk::model::AuxiliaryGeometry::hasUrl)
    .def("isValid", (bool (smtk::model::AuxiliaryGeometry::*)() const) &smtk::model::AuxiliaryGeometry::isValid)
    // .def("isValid", (bool (smtk::model::AuxiliaryGeometry::*)(::smtk::model::Entity * *) const) &smtk::model::AuxiliaryGeometry::isValid, py::arg("entRec"))
    .def("setUrl", &smtk::model::AuxiliaryGeometry::setUrl, py::arg("url"))
    .def("url", &smtk::model::AuxiliaryGeometry::url)
    ;
  return instance;
}

#endif
