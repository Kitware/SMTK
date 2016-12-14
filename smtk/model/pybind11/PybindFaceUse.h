//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_FaceUse_h
#define pybind_smtk_model_FaceUse_h

#include <pybind11/pybind11.h>

#include "smtk/model/FaceUse.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Shell.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Volume.h"

namespace py = pybind11;

py::class_< smtk::model::FaceUse, smtk::model::UseEntity > pybind11_init_smtk_model_FaceUse(py::module &m)
{
  py::class_< smtk::model::FaceUse, smtk::model::UseEntity > instance(m, "FaceUse");
  instance
    .def(py::init<::smtk::model::FaceUse const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::FaceUse::*)(::smtk::model::EntityRef const &) const) &smtk::model::FaceUse::operator!=)
    .def("deepcopy", (smtk::model::FaceUse & (smtk::model::FaceUse::*)(::smtk::model::FaceUse const &)) &smtk::model::FaceUse::operator=)
    .def("__eq__", (bool (smtk::model::FaceUse::*)(::smtk::model::EntityRef const &) const) &smtk::model::FaceUse::operator==)
    .def("boundingShell", &smtk::model::FaceUse::boundingShell)
    .def("classname", &smtk::model::FaceUse::classname)
    .def("face", &smtk::model::FaceUse::face)
    .def("isValid", (bool (smtk::model::FaceUse::*)() const) &smtk::model::FaceUse::isValid)
    // .def("isValid", (bool (smtk::model::FaceUse::*)(::smtk::model::Entity * *) const) &smtk::model::FaceUse::isValid, py::arg("entRec"))
    .def("loops", &smtk::model::FaceUse::loops)
    .def("volume", &smtk::model::FaceUse::volume)
    ;
  return instance;
}

#endif
