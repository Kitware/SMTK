//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Instance_h
#define pybind_smtk_model_Instance_h

#include <pybind11/pybind11.h>

#include "smtk/model/Instance.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"

namespace py = pybind11;

py::class_< smtk::model::Instance, smtk::model::EntityRef > pybind11_init_smtk_model_Instance(py::module &m)
{
  py::class_< smtk::model::Instance, smtk::model::EntityRef > instance(m, "Instance");
  instance
    .def(py::init<::smtk::model::Instance const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::Instance::*)(::smtk::model::EntityRef const &) const) &smtk::model::Instance::operator!=)
    .def("deepcopy", (smtk::model::Instance & (smtk::model::Instance::*)(::smtk::model::Instance const &)) &smtk::model::Instance::operator=)
    .def("__eq__", (bool (smtk::model::Instance::*)(::smtk::model::EntityRef const &) const) &smtk::model::Instance::operator==)
    .def("classname", &smtk::model::Instance::classname)
    .def("isValid", (bool (smtk::model::Instance::*)() const) &smtk::model::Instance::isValid)
    // .def("isValid", (bool (smtk::model::Instance::*)(::smtk::model::Entity * *) const) &smtk::model::Instance::isValid, py::arg("entRec"))
    .def("prototype", &smtk::model::Instance::prototype)
    ;
  return instance;
}

#endif
